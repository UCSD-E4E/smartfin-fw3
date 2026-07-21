/**
 * @file mutex.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Particle implementation of SF_HAL::Mutex using the OS mutex API.
 * @date 2026-05-17
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "product.hpp"
#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

Mutex::Mutex()
{
    os_mutex_t *m = new os_mutex_t;
    os_mutex_create(m);
    _handle = m;
}

Mutex::~Mutex()
{
    os_mutex_t *m = static_cast<os_mutex_t *>(_handle);
    os_mutex_destroy(*m);
    delete m;
}

void Mutex::lock()
{
    os_mutex_lock(*static_cast<os_mutex_t *>(_handle));
}

void Mutex::unlock()
{
    os_mutex_unlock(*static_cast<os_mutex_t *>(_handle));
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE


