/**
 * @file i2c.cpp
 * @brief Particle implementation of SF_HAL I2C functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

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

bool i2c_read(uint8_t device_addr, uint8_t reg, uint8_t* buf, std::size_t len)
{
    if (!Wire.isEnabled())
    {
        Wire.begin();
    }

    Wire.beginTransmission(device_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    Wire.requestFrom(static_cast<int>(device_addr), static_cast<int>(len));
    for (std::size_t i = 0; i < len; ++i)
    {
        if (!Wire.available())
        {
            return false;
        }
        buf[i] = static_cast<uint8_t>(Wire.read());
    }
    return true;
}

bool i2c_write(uint8_t device_addr, uint8_t reg, const uint8_t* buf,
               std::size_t len)
{
    if (!Wire.isEnabled())
    {
        Wire.begin();
    }

    Wire.beginTransmission(device_addr);
    Wire.write(reg);
    for (std::size_t i = 0; i < len; ++i)
    {
        Wire.write(buf[i]);
    }
    return Wire.endTransmission() == 0;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
