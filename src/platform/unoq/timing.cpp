/**
 * @file timing.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL timing functions.
 * @date 2026-07-23
 *
 * Timing is native to Linux (clock_gettime()/usleep()); none of it needs
 * the SPI link to the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

tick_t millis()
{
    // TODO(unoq): return milliseconds since boot via clock_gettime(CLOCK_MONOTONIC).
    return 0;
}

void delay_ms(uint32_t ms)
{
    // TODO(unoq): block for ms milliseconds (e.g. usleep()).
}

void delay_us(uint32_t us)
{
    // TODO(unoq): block for us microseconds (e.g. usleep()).
}

uint32_t micros()
{
    // TODO(unoq): return microseconds since boot via clock_gettime(CLOCK_MONOTONIC).
    return 0;
}

uint32_t time_now()
{
    // TODO(unoq): return the current UTC Unix timestamp (e.g. time()).
    return 0;
}

bool time_is_valid()
{
    // TODO(unoq): report whether the system clock has been synchronised
    // (e.g. via NTP/chrony status).
    return false;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
