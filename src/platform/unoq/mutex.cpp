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

#include <pthread.h>

namespace SF_HAL
{

    Mutex::Mutex()
    {
        auto *mut = new pthread_mutex_t;
        pthread_mutex_init(mut, nullptr);
        _handle = mut;
    }

    Mutex::~Mutex()
    {
        auto *mut = static_cast<pthread_mutex_t *>(_handle);
        pthread_mutex_destroy(mut);
        delete mut;
    }

    void Mutex::lock()
    {
        pthread_mutex_lock(static_cast<pthread_mutex_t *>(_handle));
    }

    void Mutex::unlock()
    {
        pthread_mutex_unlock(static_cast<pthread_mutex_t *>(_handle));
    }

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
