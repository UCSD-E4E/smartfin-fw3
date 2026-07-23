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
 * Blocks until all bytes have been handed to the peripheral.
 *
 * @param data Pointer to the bytes to send.
 * @param len  Number of bytes to send.
 */
void transport_send(const uint8_t *data, std::size_t len);

/**
 * @brief Read raw bytes from the SPI link.
 *
 * Blocks until @p len bytes have been received or an implementation-defined
 * timeout elapses.
 *
 * @param out Destination buffer; must be at least @p len bytes.
 * @param len Number of bytes to read.
 * @return Number of bytes actually read; less than @p len on timeout.
 */
std::size_t transport_recv(uint8_t *out, std::size_t len);

} // namespace sf_mcu

#endif // SF_MCU_TRANSPORT_HPP
