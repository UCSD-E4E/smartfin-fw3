/**
 * @file ble.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Particle implementation of SF_HAL BLE functions.
 * @date 2026-05-17
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "Particle.h"
#include "platform/hal.hpp"

#include <cstddef>
#include <cstdint>

namespace
{

// callbacks_ is written once by ble_set_callbacks() before ble_init() and
// read-only from the BLE thread after that. The single-writer / read-after-
// init pattern makes the struct copy safe without a lock; ble_set_callbacks()
// asserts this invariant so misuse is caught at runtime.
SF_HAL::ble::Callbacks callbacks_{nullptr, nullptr, nullptr};

/**
 * @brief Particle BLE backend: owns the GATT characteristics and routes
 *        radio events to the registered SF_HAL callbacks.
 *
 * Confined to this translation unit so no Particle BLE types leak outward.
 * Constructed lazily via getInstance() after BLE.on() is live.
 *
 * All Particle objects (BleUuid, BleCharacteristic) are stored by value so
 * no heap allocation occurs after singleton construction.
 */
class ParticleBleBackend
{
public:
    /**
     * @brief Construct the backend shell with no characteristics registered yet.
     */
    ParticleBleBackend() : registered_(false) {}

    /**
     * @brief Return the singleton backend instance.
     */
    static ParticleBleBackend& getInstance()
    {
        static ParticleBleBackend instance;
        return instance;
    }

    /**
     * @brief Register GATT characteristics and wire connect/disconnect callbacks.
     *
     * Idempotent: subsequent calls return true immediately without
     * re-registering.
     *
     * @param service_uuid        128-bit service UUID string.
     * @param telemetry_char_uuid 128-bit telemetry characteristic UUID string.
     * @param control_char_uuid   128-bit control characteristic UUID string.
     * @return true on success, false if any argument is null.
     */
    bool init(const char* service_uuid,
              const char* telemetry_char_uuid,
              const char* control_char_uuid)
    {
        if (registered_)
        {
            return true;
        }

        serviceUuid_   = BleUuid(service_uuid);
        telemetryUuid_ = BleUuid(telemetry_char_uuid);
        controlUuid_   = BleUuid(control_char_uuid);

        telemetryChar_ = BleCharacteristic(
            "tele", BleCharacteristicProperty::NOTIFY, telemetryUuid_, serviceUuid_);
        controlChar_ = BleCharacteristic("ctrl",
                                         BleCharacteristicProperty::WRITE_WO_RSP,
                                         controlUuid_,
                                         serviceUuid_,
                                         ParticleBleBackend::onControlReceivedStatic,
                                         this);

        BLE.addCharacteristic(telemetryChar_);
        BLE.addCharacteristic(controlChar_);
        BLE.onConnected(&ParticleBleBackend::onConnectedStatic, this);
        BLE.onDisconnected(&ParticleBleBackend::onDisconnectedStatic, this);

        registered_ = true;
        return true;
    }

    /**
     * @brief Send a NOTIFY payload on the telemetry characteristic.
     * @param data Payload bytes.
     * @param len  Number of bytes to send.
     * @return true if the stack accepted the notification.
     */
    bool notifyTelemetry(const uint8_t* data, std::size_t len)
    {
        if (!registered_)
        {
            return false;
        }
        return telemetryChar_.setValue(data, len);
    }

private:
    // BLE callbacks run on the BLE thread (small stack; Serial can deadlock).
    // State updates only no Serial or BLE API calls inside these.

    static void onConnectedStatic(const BlePeerDevice& peer, void* context)
    {
        (void)peer;
        auto* self = static_cast<ParticleBleBackend*>(context);
        if (self)
        {
            self->onConnected();
        }
    }

    static void onDisconnectedStatic(const BlePeerDevice& peer, void* context)
    {
        (void)peer;
        auto* self = static_cast<ParticleBleBackend*>(context);
        if (self)
        {
            self->onDisconnected();
        }
    }

    static void onControlReceivedStatic(const uint8_t* data,
                                        size_t len,
                                        const BlePeerDevice& peer,
                                        void* context)
    {
        (void)peer;
        if (!context || !data)
        {
            return;
        }
        static_cast<ParticleBleBackend*>(context)->onControlReceived(data, len);
    }

    void onConnected()
    {
        SF_HAL::ble::Callbacks cbs = callbacks_;
        if (cbs.on_connection)
        {
            cbs.on_connection(true, cbs.context);
        }
    }

    void onDisconnected()
    {
        SF_HAL::ble::Callbacks cbs = callbacks_;
        if (cbs.on_connection)
        {
            cbs.on_connection(false, cbs.context);
        }
    }

    void onControlReceived(const uint8_t* data, std::size_t len)
    {
        SF_HAL::ble::Callbacks cbs = callbacks_;
        if (cbs.on_write)
        {
            cbs.on_write(0, data, len, cbs.context);
        }
    }

    bool registered_;
    BleUuid serviceUuid_;
    BleUuid telemetryUuid_;
    BleUuid controlUuid_;
    BleCharacteristic telemetryChar_;
    BleCharacteristic controlChar_;
};

ParticleBleBackend* backend_ = nullptr;

} // namespace

namespace SF_HAL
{

/**
 * @brief Register BLE event callbacks.
 *
 * Must be called before @c ble_init(). BLE events cannot fire before the
 * radio is on, so the write is safe without a lock. Calling after
 * @c ble_init() is a programming error and triggers SPARK_ASSERT.
 */
void ble_set_callbacks(const ble::Callbacks& callbacks)
{
    SPARK_ASSERT(backend_ == nullptr);
    callbacks_ = callbacks;
}

bool ble_init(const char* device_name,
              const char* service_uuid,
              const char* telemetry_char_uuid,
              const char* control_char_uuid)
{
    if (!device_name || !service_uuid || !telemetry_char_uuid || !control_char_uuid)
    {
        return false;
    }

    BLE.on();
    BLE.setDeviceName(device_name);

    ParticleBleBackend& backend = ParticleBleBackend::getInstance();
    backend_ = &backend;
    return backend.init(service_uuid, telemetry_char_uuid, control_char_uuid);
}

bool ble_advertise(const char* service_uuid, const char* local_name)
{
    if (!service_uuid)
    {
        return false;
    }

    BleAdvertisingData advData;
    advData.appendServiceUUID(BleUuid(service_uuid));

    BleAdvertisingData scanResp;
    if (local_name)
    {
        scanResp.appendLocalName(local_name);
        BLE.advertise(&advData, &scanResp);
    }
    else
    {
        BLE.advertise(&advData);
    }

    return true;
}

void ble_stop_advertising()
{
    BLE.stopAdvertising();
}

bool ble_notify(const uint8_t* data, std::size_t len)
{
    if (!backend_)
    {
        return false;
    }
    return backend_->notifyTelemetry(data, len);
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
