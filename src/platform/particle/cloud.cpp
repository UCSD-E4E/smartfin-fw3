/**
 * @file cloud.cpp
 * @brief Particle implementation of SF_HAL cloud connectivity functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

void cloud_start_connect()
{
    Particle.connect();
}

void cloud_start_disconnect()
{
    Particle.disconnect();
}

bool cloud_connected()
{
    return Particle.connected();
}

bool cloud_publish(const char* event, const char* data)
{
    return Particle.publish(event, data);
}

void cloud_sync_time()
{
    Particle.syncTime();
}

bool cloud_sync_time_done()
{
    return Particle.syncTimeDone();
}

void cloud_process()
{
    Particle.process();
}

bool cellular_is_on()
{
    return Cellular.isOn();
}

bool cellular_is_ready()
{
    return Cellular.ready();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
