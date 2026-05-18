/**
 * @file i2c.cpp
 * @brief Particle implementation of SF_HAL I2C functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"
#include <Wire.h>

namespace SF_HAL
{

void i2c_begin()
{
    Wire.begin();
}

bool i2c_is_enabled()
{
    return Wire.isEnabled();
}

int i2c_read(uint8_t address, char *data, int length, bool repeated)
{
    if (!SF_HAL::i2c_is_enabled())
    {
        SF_HAL::i2c_begin();
        SF_HAL::delay_ms(1000);
    }

    Wire.beginTransmission(address);
    uint8_t bytes = Wire.requestFrom((int)address, length);
    uint8_t idx;
    for (idx = 0; Wire.available() && idx < bytes; idx++)
    {
        data[idx] = (uint8_t)Wire.read();
    }
    Wire.endTransmission(repeated == false);

    return idx == length ? 0 : -1;
}

int i2c_write(uint8_t address, const char *data, int length, bool repeated)
{
    if (!SF_HAL::i2c_is_enabled())
    {
        SF_HAL::i2c_begin();
        SF_HAL::delay_ms(1000);
    }

    Wire.beginTransmission(address);
    for (int idx = 0; idx < length; idx++)
    {
        Wire.write(data[idx]);
    }
    return Wire.endTransmission(!repeated) == 0 ? length : -1;
}

TwoWire& i2c_get_wire()
{
    return Wire;
}

void i2c_lock()
{
    Wire.lock();
}

void i2c_unlock()
{
    Wire.unlock();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
