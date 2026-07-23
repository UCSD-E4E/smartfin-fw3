/**
 * @file led.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL::LedStatus and
 *        SF_HAL::LedSystemTheme.
 * @date 2026-07-23
 *
 * LEDs are physically driven via SF_RPC gpio_write calls to the STM32U585,
 * composed with a software blink/fade engine on the Linux side (there is
 * no hardware LED controller to delegate to, unlike Particle's LEDStatus).
 * LedStatus must keep sizeof(LedStatus) == 1 and carry no data members,
 * per the address-pool contract documented on the class in hal_types.hpp;
 * this file owns the pool mapping instance addresses to platform LED
 * state.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

LedStatus::~LedStatus()
{
    // TODO(unoq): release this instance's slot in the address-pool.
}

void LedStatus::setColor(uint32_t color)
{
    // TODO(unoq): store color for this instance's pool slot.
}

void LedStatus::setPattern(LedPattern pattern)
{
    // TODO(unoq): store pattern for this instance's pool slot.
}

void LedStatus::setPeriod(uint16_t period_ms)
{
    // TODO(unoq): store period_ms for this instance's pool slot.
}

void LedStatus::setPriority(LedPriority priority)
{
    // TODO(unoq): store priority for this instance's pool slot.
}

void LedStatus::setActive(bool active)
{
    // TODO(unoq): mark this instance's pool slot active/inactive.
}

LedSystemTheme::LedSystemTheme()
{
    // TODO(unoq): initialise default signal-to-colour mappings.
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color)
{
    // TODO(unoq): map signal to a solid color.
}

void LedSystemTheme::setSignal(LedSignal signal,
                               uint32_t color,
                               LedPattern pattern,
                               LedSpeed speed)
{
    // TODO(unoq): map signal to color/pattern with a preset speed.
}

void LedSystemTheme::setSignal(LedSignal signal,
                               uint32_t color,
                               LedPattern pattern,
                               uint16_t period_ms)
{
    // TODO(unoq): map signal to color/pattern with an explicit period.
}

void LedSystemTheme::apply()
{
    // TODO(unoq): activate this theme as the system-wide LED theme.
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
