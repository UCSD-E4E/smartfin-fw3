#include "tmp117Sensor.h"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "tmp117.h"
#include "tmp117_cpp.h"
#include "product.hpp"

#include <math.h>
#include <stdint.h>

tmpSensor::tmpSensor(TMP117 &sensor):
m_sensor(sensor)
{
}

bool tmpSensor::init()
{
    int success = m_sensor.write_cfg_reg(TMP117_MODE_CONTINUOUS);
    if(!success) 
    {
        FLOG_AddError(FLOG_TEMP_FAIL, 0);
    }
    return success;
}

bool tmpSensor::stop()
{
    m_sensor.write_cfg_reg(TMP117_MODE_SHUTDOWN);
    return true;
}

float tmpSensor::getTemp()
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