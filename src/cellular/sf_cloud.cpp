#include "sf_cloud.hpp"

#include "cli/flog.hpp"
#include "platform/hal.hpp"
#include "product.hpp"
#include "sys/NVRAM.hpp"
#include "system.hpp"

#include <stdint.h>
namespace sf::cloud
{
    SF_HAL::tick_t last_publish_time = 0;
    int wait_connect(int timeout_ms, bool bypass_attempts)
    {
        uint16_t n_attempts;

        NVRAM& nvram = *pSystemDesc->pNvram;
        if (SF_HAL::cloud_connected())
        {
            return SUCCESS;
        }
        nvram.get(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        if (!bypass_attempts && n_attempts > SF_CLOUD_CONNECT_MAX_ATTEMPTS)
        {
            FLOG_AddError(FLOG_CELL_CONNECT_FAIL_ATTEMPT_EXCEEDED, n_attempts);
            return ATTEMPTS_EXCEEDED;
        }
        n_attempts++;
        nvram.put(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);

        SF_HAL::cloud_start_connect();
        SF_HAL::tick_t start = SF_HAL::millis();
        while (!SF_HAL::cloud_connected())
        {
            SF_HAL::cloud_process();
            if ((SF_HAL::millis() - start) >= (SF_HAL::tick_t)timeout_ms)
            {
                FLOG_AddError(FLOG_CELL_CONNECT_FAIL_TIMEOUT, timeout_ms);
                return TIMEOUT;
            }
        }
        n_attempts = 0;
        nvram.put(NVRAM::CLOUD_CONNECT_COUNTER, n_attempts);
        return SUCCESS;
    }

    int wait_disconnect(int timeout_ms)
    {
        SF_HAL::cloud_start_disconnect();
        SF_HAL::tick_t start = SF_HAL::millis();
        while (SF_HAL::cloud_connected())
        {
            SF_HAL::cloud_process();
            if ((SF_HAL::millis() - start) >= (SF_HAL::tick_t)timeout_ms)
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
        SF_HAL::tick_t time_since_last = SF_HAL::millis() - last_publish_time;
        // SF_OSAL_printf("%u ms since last publish" __NL__, time_since_last);
        if (time_since_last < SF_UPLOAD_MS_PER_TRANSMIT)
        {
            SF_HAL::delay_ms(SF_UPLOAD_MS_PER_TRANSMIT - time_since_last);
        }
        if (strlen(blob) > SF_HAL::cloud::MAX_EVENT_DATA_LEN)
        {
            return OVERSIZE_DATA;
        }
        if (strlen(title) > SF_HAL::cloud::MAX_EVENT_NAME_LEN)
        {
            return OVERSIZE_NAME;
        }

        if (!SF_HAL::cloud_connected())
        {
            return NOT_CONNECTED;
        }
        if (!SF_HAL::cloud_publish(title, blob))
        {
            return PUBLISH_FAIL;
        }
        last_publish_time = SF_HAL::millis();
        return SUCCESS;
    }

    bool is_connected()
    {
        return SF_HAL::cloud_connected();
    }

}