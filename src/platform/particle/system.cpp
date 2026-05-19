/**
 * @file system.cpp
 * @brief Particle implementation of SF_HAL system identity and control functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"
#include "platform/particle/pins.hpp"
#include "Particle.h"

namespace
{

    // Cached null-terminated copies of strings returned by Particle avoids
    // repeated heap allocation on every call and satisfies the "pointer valid for
    // process lifetime" contract in hal.hpp.
    char device_id_buf[32] = {};
    char os_version_buf[32] = {};

} // namespace

namespace SF_HAL
{

const char* system_device_id()
{
    if (device_id_buf[0] == '\0')
    {

        strncpy(device_id_buf, System.deviceID().c_str(), sizeof(device_id_buf) - 1);
    }
    return device_id_buf;
}

const char* system_os_version()
{
    if (os_version_buf[0] == '\0')
    {
        strncpy(os_version_buf, System.version().c_str(), sizeof(os_version_buf) - 1);
    }
    return os_version_buf;
}

void system_reset()
{
    System.reset();
}

void system_sleep(SleepMode mode, uint32_t duration_sec)
{
    switch (mode)
    {
        case SleepMode::SOFT_POWER_OFF:
            if (duration_sec > 0)
            {
                System.sleep(SLEEP_MODE_SOFTPOWEROFF, duration_sec);
            }
            else
            {
                System.sleep(SLEEP_MODE_SOFTPOWEROFF);
            }
            break;

        case SleepMode::HIBERNATE:
            // HIBERNATE with no GPIO wake source callers that need a wake
            // pin should use system_sleep_gpio_wake() instead.
            break;
    }
}

void system_sleep_gpio_wake(PinId wake_pin)
{
    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::HIBERNATE).gpio(pinToInt(wake_pin), RISING);
    System.sleep(config);
}

float battery_voltage()
{
    static FuelGauge gauge;
    return gauge.getVCell();
}

BatteryState battery_state()
{
    return static_cast<BatteryState>(System.batteryState());
}

void reset_info_enable()
{
    System.enableFeature(FEATURE_RESET_INFO);
}

ResetReason reset_reason()
{
    // Particle reset reasons are sparse multiples of 10; our enum is sequential.
    // Explicit mapping avoids undefined behaviour from out-of-range static_cast.
    switch (System.resetReason())
    {
    case RESET_REASON_NONE:            return ResetReason::NONE;
    case RESET_REASON_PIN_RESET:       return ResetReason::PIN_RESET;
    case RESET_REASON_POWER_MANAGEMENT:return ResetReason::POWER_MANAGEMENT;
    case RESET_REASON_POWER_DOWN:      return ResetReason::POWER_DOWN;
    case RESET_REASON_POWER_BROWNOUT:  return ResetReason::POWER_BROWNOUT;
    case RESET_REASON_WATCHDOG:        return ResetReason::WATCHDOG;
    case RESET_REASON_UPDATE:          return ResetReason::UPDATE;
    case RESET_REASON_UPDATE_ERROR:    return ResetReason::UPDATE_TIMEOUT;
    case RESET_REASON_UPDATE_TIMEOUT:  return ResetReason::UPDATE_TIMEOUT;
    case RESET_REASON_FACTORY_RESET:   return ResetReason::FACTORY_RESET;
    case RESET_REASON_SAFE_MODE:       return ResetReason::SAFE_MODE;
    case RESET_REASON_DFU_MODE:        return ResetReason::DFU_MODE;
    case RESET_REASON_PANIC:           return ResetReason::PANIC;
    case RESET_REASON_USER:            return ResetReason::USER;
    default:                           return ResetReason::UNKNOWN;
    }
}

uint32_t reset_reason_data()
{
    return System.resetReasonData();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
