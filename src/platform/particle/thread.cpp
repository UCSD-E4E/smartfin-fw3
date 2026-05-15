/**
 * @file thread.cpp
 * @brief Particle implementation of SF_HAL threading functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

void* thread_create(const char*    name,
                    ThreadFn       function,
                    void*          param,
                    ThreadPriority priority,
                    std::size_t    stack_size)
{
    // Particle's Thread class is heap-allocated so the handle outlives this
    // call. Threads are expected to run indefinitely; the handle is never freed.
    Thread* t = new Thread(
        name,
        function,
        param,
        static_cast<os_thread_prio_t>(priority),
        stack_size
    );
    return static_cast<void*>(t);
}

void thread_yield()
{
    os_thread_yield();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
