/**
 * @file nvm.cpp
 * @brief Particle implementation of SF_HAL non-volatile memory functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace SF_HAL
{

void nvm_read(uint32_t addr, void* out, std::size_t len)
{
    uint8_t* dst = static_cast<uint8_t*>(out);
    for (std::size_t i = 0; i < len; ++i)
    {
        EEPROM.get(static_cast<int>(addr + i), dst[i]);
    }
}

void nvm_write(uint32_t addr, const void* in, std::size_t len)
{
    const uint8_t* src = static_cast<const uint8_t*>(in);
    for (std::size_t i = 0; i < len; ++i)
    {
        EEPROM.put(static_cast<int>(addr + i), src[i]);
    }
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
