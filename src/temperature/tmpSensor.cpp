/**
 * @file tmpSensor.cpp
 * @author @emilybthorpe
 * @brief Manages Temperature Sensor
 * @version 0.1
 * @date 2023-08-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "tmpSensor.h"

#include "max31725.h"
#include "max31725_cpp.h"

#include "cli/conio.hpp"
#include "consts.hpp"

#include <stdint.h>

tmpSensor::tmpSensor(MAX31725 &sensor):
m_sensor(sensor)
{
}

bool tmpSensor::init()
{
    SF_OSAL_printf("Telling max to start" __NL__);
    int sucsess = m_sensor.write_cfg_reg(MAX31725_CFG_CONTINUOUS);
    SF_OSAL_printf("Max start sucsess: %d" __NL__, sucsess);
    return sucsess;
}

bool tmpSensor::stop()
{
    m_sensor.write_cfg_reg(MAX31725_CFG_SHUTDOWN);
    return true;
}

float tmpSensor::getTemp()
{
    return m_sensor.read_reg_as_temperature(MAX31725_REG_TEMPERATURE);
}
