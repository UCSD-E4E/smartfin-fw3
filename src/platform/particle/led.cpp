/**
 * @file led.cpp
 * @brief Particle implementation of SF_HAL LED classes.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 *
 * SF_HAL::LedStatus is held by value in task classes, but Particle's
 * LEDStatus also carries internal state. A fixed-size pool maps each
 * SF_HAL::LedStatus instance address to its corresponding Particle LEDStatus
 * so no heap allocation is required. The pool size exceeds the number of
 * LedStatus members in the codebase (currently 5).
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "Particle.h"

namespace
{

constexpr int POOL_SIZE = 8;

struct PoolEntry
{
    SF_HAL::LedStatus* owner = nullptr;
    LEDStatus          status;
};

PoolEntry pool[POOL_SIZE];

LEDStatus& resolve(SF_HAL::LedStatus* self)
{
    for (auto& e : pool)
    {
        if (e.owner == self)
        {
            return e.status;
        }
    }
    for (auto& e : pool)
    {
        if (e.owner == nullptr)
        {
            e.owner = self;
            return e.status;
        }
    }
    // Pool exhausted — fall back to slot 0 rather than crashing.
    return pool[0].status;
}

} // namespace

namespace SF_HAL
{

// ---------------------------------------------------------------------------
// LedStatus
// ---------------------------------------------------------------------------

void LedStatus::setColor(uint32_t color)
{
    resolve(this).setColor(color);
}

void LedStatus::setPattern(LedPattern pattern)
{
    resolve(this).setPattern(static_cast<LEDPattern>(pattern));
}

void LedStatus::setPeriod(uint16_t period_ms)
{
    resolve(this).setPeriod(period_ms);
}

void LedStatus::setPriority(LedPriority priority)
{
    resolve(this).setPriority(static_cast<LEDPriority>(priority));
}

void LedStatus::setActive(bool active)
{
    resolve(this).setActive(active);
}

// ---------------------------------------------------------------------------
// LedSystemTheme
// ---------------------------------------------------------------------------

LedSystemTheme::LedSystemTheme() {}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color)
{
    static LEDSystemTheme theme;
    theme.setSignal(static_cast<LEDSignal>(signal), color);
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color,
                                LedPattern pattern, LedSpeed speed)
{
    static LEDSystemTheme theme;
    theme.setSignal(static_cast<LEDSignal>(signal),
                    color,
                    static_cast<LEDPattern>(pattern),
                    static_cast<LEDSpeed>(speed));
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color,
                                LedPattern pattern, uint16_t period_ms)
{
    static LEDSystemTheme theme;
    theme.setSignal(static_cast<LEDSignal>(signal),
                    color,
                    static_cast<LEDPattern>(pattern),
                    period_ms);
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
