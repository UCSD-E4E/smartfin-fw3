/**
 * @file transport.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Implementation of the STM32U585 SPI transport link.
 * @date 2026-07-23
 *
 * SPI3 runs as a slave with hardware NSS. Linux (the master) frames every
 * message with chip select: it asserts NSS, clocks the bytes, and releases
 * NSS. The link carries no length prefix, so the end of a frame is the
 * moment NSS goes high.
 *
 * Each call to Linux's rpc_call() is two frames:
 *   1. a request frame: Linux sends the request, MISO is ignored;
 *   2. ~1.5 ms later, a response frame: Linux clocks out a fixed-size read
 *      and expects the response to start at the first byte.
 *
 * Because the slave can only send while the master is clocking, the
 * response must already be in the TX FIFO when the response frame starts,
 * and any bytes beyond it go out as the underrun pattern (0x00).
 *
 * The data path polls the SPI registers directly instead of using
 * HAL_SPI_Receive()/HAL_SPI_Transmit(): those need the transfer length up
 * front, and the request length is only known once NSS rises.
 */
#include "transport.hpp"

#include "spi.h"
#include "stm32u5xx_hal.h"

namespace sf_mcu
{

namespace
{

/**
 * @brief How long transport_recv() waits for Linux to start a request frame.
 *
 * Short, so the dispatch loop gets control back regularly when idle.
 */
constexpr uint32_t kRequestStartTimeoutMs = 10;

/**
 * @brief How long transport_send() waits for Linux to start the response frame.
 *
 * Linux starts it ~1.5 ms after the request; this leaves generous margin.
 */
constexpr uint32_t kResponseStartTimeoutMs = 20;

/**
 * @brief Upper bound on one frame, so a stuck-low NSS cannot hang the MCU.
 *
 * 128 bytes at 1 MHz take ~1 ms.
 */
constexpr uint32_t kFrameTimeoutMs = 50;

SPI_TypeDef *spi()
{
    return hspi3.Instance;
}

/**
 * @brief Whether Linux is currently asserting chip select (NSS, active low).
 *
 * Reads the pin level directly; IDR reflects the pin even in alternate
 * function mode. NSS is PG12, the pin the Uno Q board wires to the QRB2210
 * (Zephyr arduino_uno_q-common.dtsi: spi3_nss_pg12); it must match the NSS
 * pin configured in Core/Src/spi.c.
 */
bool nss_asserted()
{
    return (GPIOG->IDR & GPIO_PIN_12) == 0U;
}

/**
 * @brief Wait for NSS to reach the given state.
 *
 * @param asserted   true to wait for NSS low, false to wait for NSS high.
 * @param timeout_ms Maximum time to wait.
 * @return true if NSS reached the state in time.
 */
bool wait_for_nss(bool asserted, uint32_t timeout_ms)
{
    const uint32_t start = HAL_GetTick();
    while (nss_asserted() != asserted)
    {
        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Read one byte from the RX FIFO.
 *
 * Must be an 8-bit access: a 32-bit read of RXDR would pop four bytes.
 */
uint8_t rx_byte()
{
    return *reinterpret_cast<volatile uint8_t *>(&spi()->RXDR);
}

/**
 * @brief Write one byte to the TX FIFO.
 *
 * Must be an 8-bit access: a 32-bit write of TXDR would push four bytes.
 */
void tx_byte(uint8_t b)
{
    *reinterpret_cast<volatile uint8_t *>(&spi()->TXDR) = b;
}

/**
 * @brief Holds off the IMU sampling interrupt for the duration of a frame.
 *
 * TIM2's handler (HAL_TIM_PeriodElapsedCallback) runs blocking SPI2
 * transfers to the IMU, ~100-200 us, at a higher priority than this polling
 * loop. The SPI3 FIFO holds 8 bytes, 64 us at 1 MHz, so a sample taken
 * mid-frame would overrun it and drop request bytes or starve the response.
 * Masking TIM2 only while NSS is asserted delays a sample by at most one
 * frame (~1 ms); the pending interrupt fires as soon as the guard ends.
 */
class FrameIrqGuard
{
public:
    FrameIrqGuard() : m_was_enabled(NVIC_GetEnableIRQ(TIM2_IRQn) != 0U)
    {
        if (m_was_enabled)
        {
            NVIC_DisableIRQ(TIM2_IRQn);
        }
    }

    ~FrameIrqGuard()
    {
        if (m_was_enabled)
        {
            NVIC_EnableIRQ(TIM2_IRQn);
        }
    }

private:
    bool m_was_enabled;
};

/**
 * @brief Return the peripheral to a clean idle state between frames.
 *
 * Disabling SPI flushes both FIFOs, which drops unsent response bytes and
 * clears the underrun state left by a frame in which the MCU had nothing
 * to send.
 */
void restart()
{
    CLEAR_BIT(spi()->CR1, SPI_CR1_SPE);
    WRITE_REG(spi()->IFCR, SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_UDRC |
                               SPI_IFCR_OVRC | SPI_IFCR_MODFC);
    SET_BIT(spi()->CR1, SPI_CR1_SPE);
}

} // namespace

void transport_init()
{
    // Mode, pins and NSS come from CubeMX (Core/Src/spi.c): slave, mode 0
    // (CPOL low, CPHA 1st edge), 8-bit, MSB first, on the pins the board
    // wires to the QRB2210 (SCK PG9, MISO PG10, MOSI PB5, hardware NSS PG12),
    // matching Linux's SPI_MODE_0 / 8 bits per word.
    MX_SPI3_Init();

    // Configuration registers can only change while SPI is disabled.
    CLEAR_BIT(spi()->CR1, SPI_CR1_SPE);

    // When the MCU has nothing queued, send 0x00. This pads the response
    // frame after the response bytes, and fills MISO during request frames.
    CLEAR_BIT(spi()->CFG1, SPI_CFG1_UDRCFG);
    WRITE_REG(spi()->UDRDR, 0U);

    // TSIZE = 0: no fixed transfer length; NSS frames each message.
    MODIFY_REG(spi()->CR2, SPI_CR2_TSIZE, 0U);

    restart();
}

void transport_send(const uint8_t *data, std::size_t len)
{
    std::size_t sent = 0;

    // Preload as much as the TX FIFO holds (8 bytes on SPI3) so the first
    // byte is ready on the first clock edge of the response frame.
    while (sent < len && (spi()->SR & SPI_SR_TXP) != 0U)
    {
        tx_byte(data[sent++]);
    }

    if (wait_for_nss(true, kResponseStartTimeoutMs))
    {
        FrameIrqGuard guard;
        const uint32_t start = HAL_GetTick();
        while (nss_asserted() && (HAL_GetTick() - start) < kFrameTimeoutMs)
        {
            if (sent < len && (spi()->SR & SPI_SR_TXP) != 0U)
            {
                tx_byte(data[sent++]);
            }
            // Full duplex: Linux clocks in zeros while reading. Drain them so
            // the RX FIFO does not overrun.
            if ((spi()->SR & SPI_SR_RXP) != 0U)
            {
                (void)rx_byte();
            }
        }
    }

    restart();
}

std::size_t transport_recv(uint8_t *out, std::size_t len)
{
    if (!wait_for_nss(true, kRequestStartTimeoutMs))
    {
        return 0;
    }

    std::size_t received = 0;
    {
        FrameIrqGuard guard;
        const uint32_t start = HAL_GetTick();

        // Read until Linux releases NSS, then drain whatever is left in the
        // FIFO: the final byte can land just after NSS rises. Bytes beyond len
        // are read and dropped to keep the FIFO from overrunning.
        while ((nss_asserted() || (spi()->SR & SPI_SR_RXP) != 0U) &&
               (HAL_GetTick() - start) < kFrameTimeoutMs)
        {
            if ((spi()->SR & SPI_SR_RXP) != 0U)
            {
                const uint8_t b = rx_byte();
                if (received < len)
                {
                    out[received++] = b;
                }
            }
        }
    }

    restart();
    return received;
}

} // namespace sf_mcu
