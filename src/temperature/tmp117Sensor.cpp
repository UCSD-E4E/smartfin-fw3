#include "tmp117Sensor.h"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "tmp117.h"
#include "tmp117_cpp.h"
#include "product.hpp"

#include <math.h>
#include <stdint.h>

tmp117Sensor::tmp117Sensor(TMP117 &sensor):
m_sensor(sensor)
{
}

bool tmp117Sensor::init()
{
    int success = m_sensor.write_cfg_reg(TMP117_MODE_CONTINUOUS);
    if (!success)
    {
        FLOG_AddError(FLOG_TEMP_FAIL, 0);
        return success;
    }

    // Verify we are talking to a TMP117 by reading the device ID register.
    uint16_t device_id = 0;
    if (m_sensor.read_device_id(&device_id) != TMP117_NO_ERROR)
    {
        SF_OSAL_printf("TMP117: failed to read device ID" __NL__);
        FLOG_AddError(FLOG_TEMP_FAIL, 0);
        return false;
    }

    SF_OSAL_printf("TMP117: device ID = 0x%04x (expected 0x%04x)" __NL__,
                   device_id, (unsigned)TMP117_DEVICE_ID);

    if (device_id != TMP117_DEVICE_ID)
    {
        SF_OSAL_printf("TMP117: unexpected device ID, temperature readings may be invalid" __NL__);
        FLOG_AddError(FLOG_TEMP_FAIL, device_id);
        return false;
    }

    return true;
}

bool tmp117Sensor::stop()
{
    m_sensor.write_cfg_reg(TMP117_MODE_SHUTDOWN);
    return true;
}

float tmp117Sensor::getTemp()
{
    float value = NAN;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    WITH_LOCK(Wire)
    {
        value = m_sensor.read_reg_as_temperature(TMP117_TEMP_DATA);
    }
#endif
    return value;
}