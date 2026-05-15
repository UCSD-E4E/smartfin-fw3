/**
 * @file serial.cpp
 * @brief Particle implementation of SF_HAL serial functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

void serial_begin(uint32_t baud)
{
    Serial.begin(baud);
}

int serial_available()
{
    return Serial.available();
}

int serial_read()
{
    return Serial.read();
}

void serial_print_char(char ch)
{
    Serial.print(ch);
}

void serial_write(const uint8_t* buf, std::size_t len)
{
    Serial.write(buf, len);
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
