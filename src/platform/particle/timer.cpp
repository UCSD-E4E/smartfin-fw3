/**
 * @file timer.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Particle implementation of SF_HAL::Timer.
 * @date 2026-05-19
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

Timer::Timer(uint32_t period_ms, void (*callback)(void), bool one_shot)
{
    _handle = new ::Timer(static_cast<int>(period_ms), callback, one_shot);
}

Timer::~Timer()
{
    static_cast<::Timer *>(_handle)->stop();
    delete static_cast<::Timer *>(_handle);
}

void Timer::start()
{
    static_cast<::Timer *>(_handle)->start();
}

void Timer::stop()
{
    static_cast<::Timer *>(_handle)->stop();
}

bool Timer::isActive()
{
    return static_cast<::Timer *>(_handle)->isActive();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
