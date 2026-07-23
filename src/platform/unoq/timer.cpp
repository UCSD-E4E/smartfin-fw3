/**
 * @file timer.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL::Timer.
 * @date 2026-07-23
 *
 * Native Linux periodic timer (e.g. timer_create()/POSIX itimer or a
 * dedicated thread); no interaction with the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

Timer::Timer(uint32_t period_ms, void (*callback)(void), bool one_shot)
{
    // TODO(unoq): allocate a native timer handle in _handle; do not start it.
}

Timer::~Timer()
{
    // TODO(unoq): stop and release the native timer handle.
}

void Timer::start()
{
    // TODO(unoq): start (or restart) the native timer.
}

void Timer::stop()
{
    // TODO(unoq): stop the native timer; safe to call when already stopped.
}

bool Timer::isActive()
{
    // TODO(unoq): report whether the native timer is currently running.
    return false;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
