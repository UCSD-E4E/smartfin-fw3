/**
 * @file gpio.cpp
 * @brief Particle implementation of SF_HAL GPIO functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

void gpio_set_mode(int pin, GpioMode mode)
{
    switch (mode)
    {
        case GpioMode::INPUT:          ::pinMode(pin, INPUT);          break;
        case GpioMode::INPUT_PULLDOWN: ::pinMode(pin, INPUT_PULLDOWN); break;
        case GpioMode::OUTPUT:         ::pinMode(pin, OUTPUT);         break;
    }
}

void gpio_write(int pin, GpioState state)
{
    ::digitalWrite(pin, state == GpioState::HIGH ? HIGH : LOW);
}

bool gpio_read(int pin)
{
    return ::digitalRead(pin) == HIGH;
}

void gpio_write_fast(int pin, GpioState state)
{
    ::digitalWriteFast(pin, state == GpioState::HIGH ? HIGH : LOW);
}

bool gpio_read_fast(int pin)
{
    return ::pinReadFast(pin) == HIGH;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
