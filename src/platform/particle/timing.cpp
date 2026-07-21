/**
 * @file timing.cpp
 * @brief Particle implementation of SF_HAL timing functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

tick_t millis()
{
    return ::millis();
}

void delay_ms(uint32_t ms)
{
    ::delay(ms);
}

void delay_us(uint32_t us)
{
    ::delayMicroseconds(us);
}

uint32_t micros()
{
    return ::micros();
}

uint32_t time_now()
{
    return (uint32_t)Time.now();
}

bool time_is_valid()
{
    return Time.isValid();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
