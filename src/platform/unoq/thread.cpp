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

namespace SF_HAL
{

void* thread_create(const char* name,
                    ThreadFn function,
                    void* param,
                    ThreadPriority priority,
                    std::size_t stack_size)
{
    // TODO(unoq): create and start a pthread running function(param), with
    // stack_size and a scheduling priority derived from priority.
    return nullptr;
}

void thread_yield()
{
    // TODO(unoq): yield the calling thread (e.g. sched_yield()).
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
