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
 *
 * Thread safety: the pool is not synchronised. This is safe in the current
 * codebase because all LedStatus construction, destruction, and method calls
 * occur on the main task thread. If LED state is ever manipulated from a
 * second thread, a mutex must be added around every pool access.
 */

#include "platform/platform.hpp"

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

// Single shared Particle theme all setSignal overloads write here.
LEDSystemTheme particleTheme;

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
    // Pool exhausted this is a programming error; halt rather than
    // silently aliasing two logical LED statuses to the same object.
    SPARK_ASSERT(false);
    return pool[0].status; // unreachable; satisfies compiler
}

void release(SF_HAL::LedStatus* self)
{
    for (auto& e : pool)
    {
        if (e.owner == self)
        {
            e.status.setActive(false);
            e.owner = nullptr;
            return;
        }
    }
}

} // namespace

namespace SF_HAL
{

// LedStatus

LedStatus::~LedStatus()
{
    release(this);
}

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

// LedSystemTheme

LedSystemTheme::LedSystemTheme() {}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color)
{
    particleTheme.setSignal(static_cast<LEDSignal>(signal), color);
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color,
                                LedPattern pattern, LedSpeed speed)
{
    particleTheme.setSignal(static_cast<LEDSignal>(signal),
                            color,
                            static_cast<LEDPattern>(pattern),
                            static_cast<LEDSpeed>(speed));
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color,
                                LedPattern pattern, uint16_t period_ms)
{
    particleTheme.setSignal(static_cast<LEDSignal>(signal),
                            color,
                            static_cast<LEDPattern>(pattern),
                            period_ms);
}

void LedSystemTheme::apply()
{
    particleTheme.apply();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
