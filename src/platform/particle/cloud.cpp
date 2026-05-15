/**
 * @file cloud.cpp
 * @brief Particle implementation of SF_HAL cloud connectivity functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

int cloud_connect(uint32_t timeout_ms, bool bypass_attempts)
{
    // bypass_attempts is an application-layer concern handled by sf_cloud.cpp;
    // the HAL only performs the raw platform connect with timeout.
    (void)bypass_attempts;

    if (Particle.connected())
    {
        return 0;
    }

    Particle.connect();
    tick_t end = millis() + timeout_ms;
    while (!Particle.connected())
    {
        Particle.process();
        if (millis() > end)
        {
            return -1;
        }
    }
    return 0;
}

int cloud_disconnect(uint32_t timeout_ms)
{
    Particle.disconnect();
    tick_t end = millis() + timeout_ms;
    while (!Particle.disconnected())
    {
        Particle.process();
        if (millis() > end)
        {
            return -1;
        }
    }
    return 0;
}

bool cloud_connected()
{
    return Particle.connected();
}

bool cloud_publish(const char* event, const char* data)
{
    return Particle.publish(event, data);
}

bool cloud_sync_time()
{
    return Particle.syncTime();
}

bool cloud_sync_time_done()
{
    return Particle.syncTimeDone();
}

void cloud_process()
{
    Particle.process();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
