/**
 * @file ble.cpp
 * @brief Particle implementation of Smartfin BLE HAL functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 4-27-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "platform/hal.hpp"

#include "Particle.h"

#include <atomic>

namespace
{
    constexpr SF_HAL::ble::CharHandle TELEMETRY_HANDLE = 1;
    constexpr SF_HAL::ble::CharHandle CONTROL_HANDLE = 2;

    std::atomic<bool> connected_{false};
    std::atomic<bool> initialized_{false};
    SF_HAL::ble::Callbacks callbacks_{nullptr, nullptr, nullptr};

    class ParticleBleBackend
    {
    public:
        ParticleBleBackend(const char* service_uuid,
                           const char* telemetry_char_uuid,
                           const char* control_char_uuid)
            : serviceUuid_(service_uuid),
              telemetryUuid_(telemetry_char_uuid),
              controlUuid_(control_char_uuid),
              telemetryCharacteristic_("tele",
                                       BleCharacteristicProperty::NOTIFY,
                                       telemetryUuid_,
                                       serviceUuid_),
              controlCharacteristic_("ctrl",
                                     BleCharacteristicProperty::WRITE_WO_RSP,
                                     controlUuid_,
                                     serviceUuid_,
                                     ParticleBleBackend::onControlReceivedStatic,
                                     this)
        {
        }

        static ParticleBleBackend& getInstance(const char* service_uuid,
                                               const char* telemetry_char_uuid,
                                               const char* control_char_uuid)
        {
            static ParticleBleBackend instance(service_uuid,
                                               telemetry_char_uuid,
                                               control_char_uuid);
            return instance;
        }

        static void onConnectedStatic(const BlePeerDevice& peer, void* context)
        {
            (void)peer;
            ParticleBleBackend* self = static_cast<ParticleBleBackend*>(context);
            if (self != nullptr)
            {
                self->onConnected();
            }
        }

        static void onDisconnectedStatic(const BlePeerDevice& peer, void* context)
        {
            (void)peer;
            ParticleBleBackend* self = static_cast<ParticleBleBackend*>(context);
            if (self != nullptr)
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
            ParticleBleBackend* self = static_cast<ParticleBleBackend*>(context);
            if (self != nullptr)
            {
                self->onControlReceived(data, len);
            }
        }

        bool notifyTelemetry(const uint8_t* data, std::size_t len)
        {
            return telemetryCharacteristic_.setValue(data, len);
        }

        void ensureRegistered()
        {
            (void)telemetryCharacteristic_;
            (void)controlCharacteristic_;
        }

    private:
        void onConnected()
        {
            connected_.store(true, std::memory_order_release);

            SF_HAL::ble::Callbacks callbacks = callbacks_;
            if (callbacks.on_connection != nullptr)
            {
                callbacks.on_connection(true, callbacks.context);
            }
        }

        void onDisconnected()
        {
            connected_.store(false, std::memory_order_release);

            SF_HAL::ble::Callbacks callbacks = callbacks_;
            if (callbacks.on_connection != nullptr)
            {
                callbacks.on_connection(false, callbacks.context);
            }
        }

        void onControlReceived(const uint8_t* data, std::size_t len)
        {
            SF_HAL::ble::Callbacks callbacks = callbacks_;
            if (callbacks.on_write != nullptr)
            {
                callbacks.on_write(CONTROL_HANDLE, data, len, callbacks.context);
            }
        }

        BleUuid serviceUuid_;
        BleUuid telemetryUuid_;
        BleUuid controlUuid_;
        BleCharacteristic telemetryCharacteristic_;
        BleCharacteristic controlCharacteristic_;
    };

    ParticleBleBackend* backend_ = nullptr;
} // namespace

namespace SF_HAL
{

bool ble_init(const char* device_name,
              const char* service_uuid,
              const char* telemetry_char_uuid,
              const char* control_char_uuid)
{
    if (initialized_.load(std::memory_order_acquire))
    {
        return true;
    }

    if (device_name == nullptr || service_uuid == nullptr ||
        telemetry_char_uuid == nullptr || control_char_uuid == nullptr)
    {
        return false;
    }

    ParticleBleBackend& backend = ParticleBleBackend::getInstance(service_uuid,
                                                                  telemetry_char_uuid,
                                                                  control_char_uuid);
    backend_ = &backend;

    BLE.on();
    BLE.setDeviceName(device_name);
    BLE.onConnected(&ParticleBleBackend::onConnectedStatic, &backend);
    BLE.onDisconnected(&ParticleBleBackend::onDisconnectedStatic, &backend);

    backend.ensureRegistered();

    initialized_.store(true, std::memory_order_release);
    return true;
}

void ble_set_callbacks(const ble::Callbacks& callbacks)
{
    callbacks_ = callbacks;
}

bool ble_advertise(const char* service_uuid, const char* local_name)
{
    if (!initialized_.load(std::memory_order_acquire) || service_uuid == nullptr)
    {
        return false;
    }

    if (local_name != nullptr)
    {
        BLE.setDeviceName(local_name);
    }

    BleAdvertisingData advData;
    advData.appendServiceUUID(BleUuid(service_uuid));
    BLE.advertise(&advData);
    return true;
}

void ble_stop_advertising()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    BLE.stopAdvertising();
}

bool ble_is_connected()
{
    return connected_.load(std::memory_order_acquire);
}

bool ble_notify(const uint8_t* data, std::size_t len)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !connected_.load(std::memory_order_acquire) ||
        backend_ == nullptr || data == nullptr || len == 0)
    {
        return false;
    }

    return backend_->notifyTelemetry(data, len);
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE
