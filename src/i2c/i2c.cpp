#include "i2c.h"

#include "cli/conio.hpp"
#include "platform/hal.hpp"
#include "product.hpp"

#include <stdint.h>

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
    return SF_HAL::i2c_write(address, data, length, repeated);
}
