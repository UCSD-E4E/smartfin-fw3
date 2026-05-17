#include "i2c.h"

#include "cli/conio.hpp"
#include "platform/hal.hpp"
#include "product.hpp"

#include <stdint.h>
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include <Wire.h>
#endif

// Map MBED I2C class to the Particle IO Device OS I2C class.

int I2C::read(uint8_t address, char *data, int length, bool repeated)
{

    // convert from 8-bit to 7-bit address
    address >>= 1;
    return SF_HAL::i2c_read(address, data, length, repeated);
}

int I2C::write(uint8_t address, const char *data, int length, bool repeated)
{

    // convert from 8-bit to 7-bit address
    address >>= 1;
    int ret = SF_HAL::i2c_write(address, data, length, repeated);
    return ret >= 0 ? I2C_NO_ERROR : I2C_ERROR;
}
