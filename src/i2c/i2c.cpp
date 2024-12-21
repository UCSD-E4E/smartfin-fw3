#include "i2c.h"

#include "Particle.h"
#include "cli/conio.hpp"
#include "product.hpp"

#include <stdint.h>
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include <Wire.h>
#endif

// Map MBED I2C class to the Particle IO Device OS I2C class.

int I2C::read(uint8_t address, char *data, int length, bool repeated)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    // convert from 8-bit to 7-bit address
    address >>= 1;

    if (!Wire.isEnabled())
    {
        Wire.begin();
        delay(1000);
    }

    Wire.beginTransmission(address);
    uint8_t bytes = Wire.requestFrom((int)address, length);
    uint8_t idx;
    for (idx = 0; Wire.available() && idx < bytes; idx++)
    {
        data[idx] = (uint8_t)Wire.read();
    }
    Wire.endTransmission(repeated == false);

    return idx == length ? I2C_NO_ERROR : I2C_ERROR;
#else
    return I2C_NO_ERROR;
#endif
}

int I2C::write(uint8_t address, const char *data, int length, bool repeated)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    // convert from 8-bit to 7-bit address
    address >>= 1;

    if (!Wire.isEnabled())
    {
        Wire.begin();
        delay(1000);
    }

    Wire.beginTransmission(address);
    uint8_t idx;
    for (idx = 0; idx < length; idx++)
    {
        Wire.write(data[idx]);
    }
    Wire.endTransmission(repeated == false);

    return idx == length ? I2C_NO_ERROR : I2C_ERROR;
#else
    return I2C_NO_ERROR;
#endif
}
