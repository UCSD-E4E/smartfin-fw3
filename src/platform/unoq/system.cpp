/**
 * @file system.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL system identity,
 *        reset, sleep, and battery functions.
 * @date 2026-07-23
 *
 * system_reset()/reset_reason() span both processors: a reset must take
 * down the MCU alongside Linux, and reset diagnostics should combine both
 * sides' causes where possible. system_sleep()/system_sleep_gpio_wake()
 * still need a concrete design for how Linux suspend and MCU-side wake
 * coordinate; battery_voltage()/battery_state() depend on which bus the
 * fuel gauge (if any) is wired to, not yet confirmed against the schematic.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

#include "ipc/hal_rpc_protocol.h"

namespace SF_HAL
{

const char* system_device_id()
{
    // TODO(unoq): return the QRB2210's unique identifier (eFuse serial or
    // similar), not a Debian machine-id.
    return "";
}

const char* system_os_version()
{
    // TODO(unoq): build and return a composite string of the Debian image
    // version, `uname -r` kernel version, and the MCU firmware version
    // (via an SF_RPC_METHOD_GET_FW_VERSION query).
    return "";
}

void system_reset()
{
    // TODO(unoq): send an SF_RPC_METHOD_RESET notification to the MCU,
    // then reboot this Linux process/device so both processors reset
    // together.
}

void system_sleep(SleepMode mode, uint32_t duration_sec)
{
    // TODO(unoq): design and implement Linux suspend, coordinated with the
    // MCU's own sleep state; not yet resolved which side initiates/owns
    // wake for SOFT_POWER_OFF vs HIBERNATE.
}

void system_sleep_gpio_wake(PinId wake_pin)
{
    // TODO(unoq): the wake GPIO is physically on the MCU; needs a
    // mechanism for the MCU to wake/signal Linux, not yet designed.
}

float battery_voltage()
{
    // TODO(unoq): read the fuel gauge, if present, once its bus placement
    // is confirmed (likely an SF_RPC_METHOD_I2C_READ call if it is on the
    // STM32U585's bus).
    return 0.0f;
}

BatteryState battery_state()
{
    // TODO(unoq): report charger/battery state once fuel gauge placement
    // is confirmed.
    return BatteryState::UNKNOWN;
}

void reset_info_enable()
{
    // TODO(unoq): record that reset-cause reporting should be gathered on
    // the next boot (Linux side has its own boot-reason source; nothing to
    // enable on the MCU beyond what it tracks itself).
}

ResetReason reset_reason()
{
    // TODO(unoq): combine Linux's own last-boot reason with an
    // SF_RPC_METHOD_GET_RESET_REASON query to the MCU, where possible.
    return ResetReason::UNKNOWN;
}

uint32_t reset_reason_data()
{
    // TODO(unoq): return auxiliary reset data, combined from both
    // processors where possible.
    return 0;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
