/**
 * @file gpio.cpp
 * @brief Particle implementation of SF_HAL GPIO functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "platform/particle/pins.hpp"
#include "Particle.h"

namespace SF_HAL
{

void gpio_set_mode(PinId pin, GpioMode mode)
{
    switch (mode)
    {
        case GpioMode::INPUT:          ::pinMode(pinToInt(pin), INPUT);          break;
        case GpioMode::INPUT_PULLDOWN: ::pinMode(pinToInt(pin), INPUT_PULLDOWN); break;
        case GpioMode::OUTPUT:         ::pinMode(pinToInt(pin), OUTPUT);         break;
    }
}

void gpio_write(PinId pin, GpioState state)
{
    ::digitalWrite(pinToInt(pin), state == GpioState::HIGH ? HIGH : LOW);
}

bool gpio_read(PinId pin)
{
    return ::digitalRead(pinToInt(pin)) == HIGH;
}

void gpio_write_fast(PinId pin, GpioState state)
{
    ::digitalWriteFast(pinToInt(pin), state == GpioState::HIGH ? HIGH : LOW);
}

bool gpio_read_fast(PinId pin)
{
    return ::pinReadFast(pinToInt(pin)) == HIGH;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
