/**
 * @file cloud.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL cloud/cellular
 *        functions.
 * @date 2026-07-23
 *
 * cellular_is_on()/cellular_is_ready() are permanent dummies, not deferred
 * work: neither Uno Q nor its peripheral board carries a cellular modem.
 * The remaining cloud_* functions are native Linux networking; what
 * backend they connect to (this no longer targets Particle's cloud) is
 * still an open decision.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

void cloud_start_connect()
{
    // TODO(unoq): initiate a non-blocking connection to the cloud backend.
}

void cloud_start_disconnect()
{
    // TODO(unoq): initiate a non-blocking disconnect from the cloud backend.
}

bool cloud_connected()
{
    // TODO(unoq): report whether a cloud session is currently active.
    return false;
}

bool cloud_publish(const char* event, const char* data)
{
    // TODO(unoq): publish event/data to the cloud backend.
    return false;
}

void cloud_sync_time()
{
    // TODO(unoq): request a time sync from the cloud backend.
}

bool cloud_sync_time_done()
{
    // TODO(unoq): report whether the pending time sync has completed.
    return false;
}

void cloud_process()
{
    // TODO(unoq): service the cloud backend's connection; likely a no-op
    // if networking runs on its own thread rather than needing a pump.
}

bool cellular_is_on()
{
    return false;
}

bool cellular_is_ready()
{
    return false;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
