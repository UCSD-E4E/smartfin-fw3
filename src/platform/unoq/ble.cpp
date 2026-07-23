/**
 * @file ble.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL BLE functions.
 * @date 2026-07-23
 *
 * The BLE radio (WCBN3536A) is handled entirely by Linux via BlueZ; the
 * STM32U585 is not involved. Concrete BlueZ types (D-Bus proxies, GATT
 * application objects) are confined to this file, matching how
 * platform/particle/ble.cpp confines Particle's BLE types.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

void ble_set_callbacks(const ble::Callbacks& callbacks)
{
    // TODO(unoq): store callbacks for later dispatch from BlueZ D-Bus signals.
}

bool ble_init(const char* device_name,
             const char* service_uuid,
             const char* telemetry_char_uuid,
             const char* control_char_uuid)
{
    // TODO(unoq): register a BlueZ GATT application with the given service
    // and characteristics.
    return false;
}

bool ble_advertise(const char* service_uuid, const char* local_name)
{
    // TODO(unoq): start BlueZ advertising for service_uuid, using
    // local_name in the scan response.
    return false;
}

void ble_stop_advertising()
{
    // TODO(unoq): stop BlueZ advertising.
}

bool ble_notify(const uint8_t* data, std::size_t len)
{
    // TODO(unoq): send a NOTIFY payload on the telemetry characteristic.
    return false;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
