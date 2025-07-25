/**
 * @file tmpSensor.cpp
 * @author Emily Thorpe <ethorpe@macalester.edu>
 * @brief Manages Temperature Sensor
 * @version 0.1
 * @date 2023-08-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "tmpSensor.h"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "max31725.h"
#include "max31725_cpp.h"
#include "product.hpp"

#include <math.h>
#include <stdint.h>

tmpSensor::tmpSensor(MAX31725 &sensor):
m_sensor(sensor)
{
}

bool tmpSensor::init()
{
    int sucsess = m_sensor.write_cfg_reg(MAX31725_CFG_CONTINUOUS);
    if(!sucsess) 
    {
        FLOG_AddError(FLOG_TEMP_FAIL, 0);
    }
    return sucsess;
}

bool tmpSensor::stop()
{
    m_sensor.write_cfg_reg(MAX31725_CFG_SHUTDOWN);
    return true;
}

float tmpSensor::getTemp()
{
    float value = NAN;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    WITH_LOCK(Wire)
    {
        value = m_sensor.read_reg_as_temperature(MAX31725_REG_TEMPERATURE);
    }
#endif
    return value;
}
