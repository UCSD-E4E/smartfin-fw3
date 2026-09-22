/**
 * @file timing.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Brent Brewster (brentbrewster11@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL timing functions.
 * @date 2026-09-22
 *
 * Timing is native to Linux (clock_gettime()/usleep()); none of it needs
 * the SPI link to the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include <time.h>
#include <unistd.h>

namespace SF_HAL
{

tick_t millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

void delay_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

void delay_us(uint32_t us)
{
    usleep(us);
}

uint32_t micros()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

uint32_t time_now()
{
    return (uint32_t)time(NULL);
}

bool time_is_valid()
{
    // Simple heuristic: if the system clock is past Jan 1, 2024, 
    // we assume it has been synced via NTP.
    return time_now() > 1704067200;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ