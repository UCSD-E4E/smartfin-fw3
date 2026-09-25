/**
 * @file imu.cpp
 * @brief Implementation of lightweight ICM-20948 SPI driver for STM32U585.
 */
#include "imu.hpp"

#include "stm32u5xx_hal.h"
#include <cstring>

namespace sf_mcu
{

namespace
{

// ICM-20948 Bank 0 Registers
constexpr uint8_t kRegWhoAmI        = 0x00;
constexpr uint8_t kRegPwrMgmt1      = 0x06;
constexpr uint8_t kRegPwrMgmt2      = 0x07;
constexpr uint8_t kRegAccelXoutH    = 0x2D;
constexpr uint8_t kRegBankSel       = 0x7F;

// Expected WHO_AM_I response
constexpr uint8_t kWhoAmIExpected   = 0xEA;

// SPI timeout in milliseconds
constexpr uint32_t kSpiTimeoutMs    = 50;

// Default CS pin on Uno Q (Arduino D10 / PB12 or dedicated JSPI CS)
#ifndef IMU_DEFAULT_CS_PORT
#define IMU_DEFAULT_CS_PORT GPIOB
#endif
#ifndef IMU_DEFAULT_CS_PIN
#define IMU_DEFAULT_CS_PIN  GPIO_PIN_12
#endif

// Hardware handles
static SPI_HandleTypeDef s_hspi2;
static GPIO_TypeDef *s_cs_port   = IMU_DEFAULT_CS_PORT;
static uint16_t s_cs_pin         = IMU_DEFAULT_CS_PIN;

/**
 * @brief Low-level SPI register write.
 *
 * Pulls CS low, sets Bit 7 = 0 to signify a write, sends the value,
 * and releases CS high.
 */
bool spi_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2] = {
        static_cast<uint8_t>(reg & 0x7F), // Bit 7 cleared (0) for WRITE
        value
    };

    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_hspi2, tx_data, 2, kSpiTimeoutMs);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    return status == HAL_OK;
}

/**
 * @brief Low-level SPI burst register read.
 *
 * Pulls CS low, sets Bit 7 = 1 to signify a read, reads len bytes,
 * and releases CS high.
 */
bool spi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (buf == nullptr)
    {
        return false;
    }

    uint8_t reg_addr = static_cast<uint8_t>((reg & 0x7F) | 0x80); // Bit 7 set (1) for READ

    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_hspi2, &reg_addr, 1, kSpiTimeoutMs);
    if (status == HAL_OK)
    {
        status = HAL_SPI_Receive(&s_hspi2, buf, len, kSpiTimeoutMs);
    }
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    return status == HAL_OK;
}

/**
 * @brief Select the active ICM-20948 register bank (0-3).
 */
bool select_bank(uint8_t bank)
{
    uint8_t val = (bank << 4);
    return spi_write_reg(kRegBankSel, val);
}

/**
 * @brief Initialize SPI2 Master hardware and CS GPIO pin.
 */
bool init_spi_master()
{
    // Enable Clocks
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // 1. Configure CS GPIO pin as Output Push-Pull, initial state HIGH (deselected)
    GPIO_InitTypeDef gpio_cs = {0};
    gpio_cs.Pin = s_cs_pin;
    gpio_cs.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_cs.Pull = GPIO_NOPULL;
    gpio_cs.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(s_cs_port, &gpio_cs);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    // 2. Configure SPI2 Master pins (JSPI header: MISO PC2, MOSI PC3, SCK PD1)
    GPIO_InitTypeDef gpio_spi = {0};

    // SCK (PD1 - AF5_SPI2)
    gpio_spi.Pin = GPIO_PIN_1;
    gpio_spi.Mode = GPIO_MODE_AF_PP;
    gpio_spi.Pull = GPIO_NOPULL;
    gpio_spi.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_spi.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOD, &gpio_spi);

    // MISO (PC2 - AF5_SPI2) & MOSI (PC3 - AF5_SPI2)
    gpio_spi.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_spi.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOC, &gpio_spi);

    // 3. Configure SPI2 Master instance (Mode 0: CPOL Low, CPHA 1Edge, up to ~7 MHz)
    s_hspi2.Instance               = SPI2;
    s_hspi2.Init.Mode              = SPI_MODE_MASTER;
    s_hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    s_hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    s_hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    s_hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    s_hspi2.Init.NSS               = SPI_NSS_SOFT; // Software CS management
    s_hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // ~5 MHz from 160 MHz clock
    s_hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    s_hspi2.Init.TIMode            = SPI_TIMODE_DISABLE;
    s_hspi2.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;

    return HAL_SPI_Init(&s_hspi2) == HAL_OK;
}

} // namespace

bool imu_init()
{
    // Initialize SPI2 Master and CS pin
    if (!init_spi_master())
    {
        return false;
    }

    // Ensure we are in Bank 0
    if (!select_bank(0))
    {
        return false;
    }

    // 1. Verify WHO_AM_I (0x00) -> expected 0xEA
    uint8_t who_am_i = 0;
    if (!spi_read_regs(kRegWhoAmI, &who_am_i, 1))
    {
        return false;
    }

    if (who_am_i != kWhoAmIExpected)
    {
        return false; // Wrong chip or SPI connection issue
    }

    // 2. Wake up chip from sleep: write 0x01 to PWR_MGMT_1 (Auto-select clock source)
    if (!spi_write_reg(kRegPwrMgmt1, 0x01))
    {
        return false;
    }

    HAL_Delay(10); // Wait for internal clocks to stabilize

    // 3. Enable accelerometer and gyroscope: write 0x00 to PWR_MGMT_2 (all axes enabled)
    if (!spi_write_reg(kRegPwrMgmt2, 0x00))
    {
        return false;
    }

    return true;
}

bool imu_read_sample(SF_RPC_IMUSampleRecord &sample)
{
    // Buffer for 12 contiguous registers:
    // [0..1]: Accel X (H, L)
    // [2..3]: Accel Y (H, L)
    // [4..5]: Accel Z (H, L)
    // [6..7]: Gyro X (H, L)
    // [8..9]: Gyro Y (H, L)
    // [10..11]: Gyro Z (H, L)
    uint8_t raw_buf[12];

    // Burst read 12 contiguous registers starting at ACCEL_XOUT_H (0x2D) over SPI
    if (!spi_read_regs(kRegAccelXoutH, raw_buf, sizeof(raw_buf)))
    {
        return false;
    }

    sample.timestamp_ms = HAL_GetTick();

    // Reconstruct 16-bit big-endian signed integers
    sample.accel_x = static_cast<int16_t>((raw_buf[0] << 8) | raw_buf[1]);
    sample.accel_y = static_cast<int16_t>((raw_buf[2] << 8) | raw_buf[3]);
    sample.accel_z = static_cast<int16_t>((raw_buf[4] << 8) | raw_buf[5]);

    sample.gyro_x  = static_cast<int16_t>((raw_buf[6] << 8) | raw_buf[7]);
    sample.gyro_y  = static_cast<int16_t>((raw_buf[8] << 8) | raw_buf[9]);
    sample.gyro_z  = static_cast<int16_t>((raw_buf[10] << 8) | raw_buf[11]);

    // Magnetometer & Quaternion orientation (will be populated once DMP is configured)
    sample.mag_x = 0;
    sample.mag_y = 0;
    sample.mag_z = 0;
    sample.quat9_1 = 0;
    sample.quat9_2 = 0;
    sample.quat9_3 = 0;
    sample.quat9_accuracy = 0;

    return true;
}

} // namespace sf_mcu
