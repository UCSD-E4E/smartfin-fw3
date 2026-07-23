/**
 * @file mutex.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL::Mutex.
 * @date 2026-07-23
 *
 * Native pthread_mutex_t; no interaction with the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

Mutex::Mutex()
{
    // TODO(unoq): allocate and initialise a pthread_mutex_t into _handle.
}

Mutex::~Mutex()
{
    // TODO(unoq): destroy and release the pthread_mutex_t.
}

void Mutex::lock()
{
    // TODO(unoq): pthread_mutex_lock() on the underlying handle.
}

void Mutex::unlock()
{
    // TODO(unoq): pthread_mutex_unlock() on the underlying handle.
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
