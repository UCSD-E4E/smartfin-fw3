/**
 * @file transport.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Implementation of the STM32U585 SPI transport link.
 * @date 2026-07-23
 */
#include "transport.hpp"

namespace sf_mcu
{

void transport_init()
{
    // TODO(unoq_mcu): configure the SPI peripheral as a slave, matching the
    // QRB2210 SPI master's clock mode and rate.
}

void transport_send(const uint8_t *data, std::size_t len)
{
    // TODO(unoq_mcu): transmit len bytes from data over SPI.
}

std::size_t transport_recv(uint8_t *out, std::size_t len)
{
    // TODO(unoq_mcu): read up to len bytes into out; return the number of
    // bytes actually read.
    return 0;
}

} // namespace sf_mcu
