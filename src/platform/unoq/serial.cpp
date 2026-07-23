/**
 * @file serial.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL debug/CLI serial
 *        functions.
 * @date 2026-07-23
 *
 * The debug/CLI console is native to Linux (stdio or a tty), not the SPI
 * link to the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

void serial_begin(uint32_t baud)
{
    // TODO(unoq): initialise the debug/CLI console (stdio or a tty path).
}

int serial_available()
{
    // TODO(unoq): return the number of bytes waiting to be read.
    return 0;
}

int serial_read()
{
    // TODO(unoq): read one byte, or -1 if none is available.
    return -1;
}

void serial_print_char(char ch)
{
    // TODO(unoq): write one character to the console.
}

void serial_write(const uint8_t* buf, std::size_t len)
{
    // TODO(unoq): write len bytes from buf to the console.
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
