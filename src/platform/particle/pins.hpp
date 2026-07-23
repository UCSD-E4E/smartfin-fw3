/**
 * @file pins.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Internal Particle pin mapping translates SF_HAL::PinId to Particle pin integers.
 * @date 2026-05-17
 */
#pragma once

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "Particle.h"
#include "platform/hal.hpp"

namespace SF_HAL
{

/**
 * @brief Translate a logical @c PinId to the Particle platform pin integer.
 *
 * Only valid inside platform/particle translation units. Never call from
 * board-agnostic code.
 *
 * @param id Logical pin identifier.
 * @return Particle pin integer corresponding to @p id.
 */
inline int pinToInt(PinId id)
{
    switch (id)
    {
        case PinId::UsbPwrDetect:   return A4;
        case PinId::StatLed:        return A5;
        case PinId::WaterDetectEn:  return A2;
        case PinId::WaterDetect:    return A6;
        case PinId::WaterStatusLed: return D9;
        case PinId::WaterMfgTestEn: return A3;
        case PinId::Wkp:            return A7;
        default:                    return -1;
    }
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
