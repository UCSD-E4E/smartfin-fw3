/**
 * @file thread.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL threading functions.
 * @date 2026-07-23
 *
 * Native pthreads; no interaction with the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

#include <pthread.h>

namespace
{

struct ThreadStart
{
    SF_HAL::ThreadFn function;
    void* param;
};

// Adapts SF_HAL::ThreadFn (void(void*)) to the pthread entry signature.
void* threadTrampoline(void* arg)
{
    ThreadStart start = *static_cast<ThreadStart*>(arg);
    delete static_cast<ThreadStart*>(arg);
    start.function(start.param);
    return nullptr;
}

} // namespace

namespace SF_HAL
{

void* thread_create(const char* name,
                    ThreadFn function,
                    void* param,
                    ThreadPriority priority,
                    std::size_t stack_size)
{
    // TODO(unoq): apply priority and stack_size; Linux defaults are used for now.
    (void)priority;
    (void)stack_size;

    if (!function)
    {
        return nullptr;
    }

    auto* start = new ThreadStart{function, param};
    auto* thread = new pthread_t;
    if (pthread_create(thread, nullptr, threadTrampoline, start) != 0)
    {
        delete start;
        delete thread;
        return nullptr;
    }

    // Threads run indefinitely and are never joined (see hal.hpp).
    pthread_detach(*thread);
    if (name)
    {
        pthread_setname_np(*thread, name);
    }
    return thread;
}

void thread_yield()
{
    // TODO(unoq): yield the calling thread (e.g. sched_yield()).
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
