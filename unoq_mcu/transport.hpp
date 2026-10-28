/**
 * @file transport.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Raw byte transport link between the STM32U585 peripheral server and
 *        the QRB2210 Linux HAL client.
 * @date 2026-07-23
 *
 * Owns the physical link: SPI, matching Uno Q's own inter-processor wiring
 * (the QRB2210 is the SPI master; this MCU is the SPI slave, pending
 * confirmation against the board schematic). Provides only raw byte
 * send/receive; msgpack-rpc framing and dispatch live in peripheral_server.
 */
#ifndef SF_MCU_TRANSPORT_HPP
#define SF_MCU_TRANSPORT_HPP

#include <cstddef>
#include <cstdint>

namespace sf_mcu
{

/**
 * @brief Initialise the SPI peripheral as an SPI slave.
 *
 * Must be called once before any @c transport_send() or @c transport_recv()
 * call. Configures the SPI peripheral and its pins to match the QRB2210
 * SPI master's clock mode and rate.
 */
void transport_init();

/**
 * @brief Transmit raw bytes over the SPI link.
 *
 * As SPI slave the MCU can only send while Linux clocks a frame, so this
 * queues the bytes, then blocks until Linux has clocked out its response
 * frame (NSS asserted, then released) or a timeout elapses. Bytes Linux
 * clocks beyond @p len read as 0x00.
 *
 * @param data Pointer to the bytes to send.
 * @param len  Number of bytes to send.
 */
void transport_send(const uint8_t *data, std::size_t len);

/**
 * @brief Read raw bytes from the SPI link.
 *
 * Waits briefly for Linux to start a frame (assert NSS), then reads until
 * Linux releases NSS. One call returns one whole message.
 *
 * @param out Destination buffer; must be at least @p len bytes.
 * @param len Capacity of @p out. Bytes beyond it are read and dropped.
 * @return Number of bytes received; 0 if no frame started before the timeout.
 */
std::size_t transport_recv(uint8_t *out, std::size_t len);

} // namespace sf_mcu

#endif // SF_MCU_TRANSPORT_HPP
