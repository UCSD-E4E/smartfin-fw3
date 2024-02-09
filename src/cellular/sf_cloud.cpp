#include "sf_cloud.hpp"

#include "cli/flog.hpp"
#include "product.hpp"
#include "sys/NVRAM.hpp"
#include "system.hpp"

#include <stdint.h>

#include "Particle.h"

namespace sf::cloud
{
    int wait_connect(int timeout_ms)
    {
        uint16_t n_attempts;
        system_tick_t end_time = millis() + timeout_ms;
        NVRAM& nvram = *pSystemDesc->pNvram;
        nvram.get(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        if (n_attempts > SF_CLOUD_CONNECT_MAX_ATTEMPTS)
        {
            return ATTEMPTS_EXCEEDED;
        }
        n_attempts++;
        nvram.put(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        Particle.connect();
        while (!Particle.connected())
        {
            Particle.process();
            if (millis() > end_time)
            {
                return TIMEOUT;
            }
        }
        n_attempts--;
        nvram.put(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        return SUCCESS;
    }

    int wait_disconnect(int timeout_ms)
    {
        system_tick_t end_time = millis() + timeout_ms;
        Particle.disconnect();
        while (!Particle.disconnected())
        {
            Particle.process();
            if (millis() > end_time)
            {
                return TIMEOUT;
            }
        }
        return SUCCESS;
    }

    bool initialize_counter(void)
    {
        uint16_t n_attempts;
        NVRAM& nvram = *pSystemDesc->pNvram;
        bool retval = true;
        nvram.get(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        if (n_attempts > SF_CLOUD_CONNECT_MAX_ATTEMPTS)
        {
            FLOG_AddError(FLOG_UPL_CONNECT_FAIL, ATTEMPTS_EXCEEDED);
            retval = false;
        }
        n_attempts = 0;
        nvram.put(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        return retval;
    }

    int publish_blob(const char* title, const char* blob)
    {
        if (strlen(blob) > particle::protocol::MAX_EVENT_DATA_LENGTH)
        {
            return OVERSIZE_DATA;
        }
        if (strlen(title) > particle::protocol::MAX_EVENT_NAME_LENGTH)
        {
            return OVERSIZE_NAME;
        }
        if (!Particle.connected())
        {
            return NOT_CONNECTED;
        }
        if (!Particle.publish(title, blob))
        {
            return PUBLISH_FAIL;
        }
        return SUCCESS;
    }
}