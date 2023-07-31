#include "tmpSensor.h"
#include "max31725.h"
#include "max31725_cpp.h"
#include <stdint.h>

tmpSensor::tmpSensor(MAX31725 &sensor):
m_sensor(sensor)
{
}

bool tmpSensor::init()
{
    m_sensor.write_cfg_reg(MAX31725_CFG_CONTINUOUS);
    return true;
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
