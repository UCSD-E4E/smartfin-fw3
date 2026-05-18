/**
 * @file ble.cpp
 * @brief Particle implementation of Smartfin BLE HAL functions.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 3-9-2026
 */

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

#include "Particle.h"
#include "platform/hal.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace
{
    constexpr SF_HAL::ble::CharHandle CONTROL_HANDLE = 2;

    std::atomic<bool> connected_{false};
    std::atomic<bool> initialized_{false};
    SF_HAL::ble::Callbacks callbacks_{nullptr, nullptr, nullptr};

    class ParticleBleBackend
    {
    public:
        ParticleBleBackend()
            : registered_(false), serviceUuid_(nullptr), telemetryUuid_(nullptr),
              controlUuid_(nullptr), telemetryCharacteristic_(nullptr),
              controlCharacteristic_(nullptr)
        {
        }

        /**
         * @brief Access the singleton backend instance.
         *
         * A single instance is required because each @c BleCharacteristic
         * object represents one physical GATT entry. Constructing a second
         * instance would attempt to register duplicate characteristics and
         * corrupt the GATT table. The static-local pattern also controls
         * construction order: @c getInstance() creates only the backend shell;
         * @c init() creates and registers the Particle GATT objects after
         * @c BLE.on() has started the stack.
         */
        static ParticleBleBackend &getInstance()
        {
            static ParticleBleBackend instance;
            return instance;
        }

        bool init(const char *service_uuid,
                  const char *telemetry_char_uuid,
                  const char *control_char_uuid)
        {
            if (registered_)
            {
                return true;
            }

            if (service_uuid == nullptr || telemetry_char_uuid == nullptr ||
                control_char_uuid == nullptr)
            {
                return false;
            }

            serviceUuid_ = new BleUuid(service_uuid);
            telemetryUuid_ = new BleUuid(telemetry_char_uuid);
            controlUuid_ = new BleUuid(control_char_uuid);

            if (serviceUuid_ == nullptr || telemetryUuid_ == nullptr || controlUuid_ == nullptr)
            {
                return false;
            }

            telemetryCharacteristic_ = new BleCharacteristic(
                "tele", BleCharacteristicProperty::NOTIFY, *telemetryUuid_, *serviceUuid_);
            controlCharacteristic_ =
                new BleCharacteristic("ctrl",
                                      BleCharacteristicProperty::WRITE_WO_RSP,
                                      *controlUuid_,
                                      *serviceUuid_,
                                      ParticleBleBackend::onControlReceivedStatic,
                                      this);

            if (telemetryCharacteristic_ == nullptr || controlCharacteristic_ == nullptr)
            {
                return false;
            }

            // BleCharacteristic construction does not auto-register with the
            // GATT server on Particle Device OS; explicit registration is
            // required before advertising.
            BLE.addCharacteristic(*telemetryCharacteristic_);
            BLE.addCharacteristic(*controlCharacteristic_);

            registered_ = true;
            return true;
        }

        /**
         * @brief Particle control-write callback shim.
         * @param data Received payload.
         * @param len Number of bytes in payload.
         * @param peer Peer device (unused).
         * @param context Pointer to backend instance.
         */
        // BLE callbacks run on the BLE thread (small stack, Serial can deadlock).
        // No Serial calls inside any of these — state updates only.
        static void onControlReceivedStatic(const uint8_t *data,
                                            size_t len,
                                            const BlePeerDevice &peer,
                                            void *context)
        {
            (void)peer;
            if (!context || !data)
                return;
            static_cast<ParticleBleBackend *>(context)->onControlReceived(data, len);
        }

        static void onConnectedStatic(const BlePeerDevice &peer, void *context)
        {
            (void)peer;
            ParticleBleBackend *self = static_cast<ParticleBleBackend *>(context);
            if (self != nullptr)
            {
                self->onConnected();
            }
        }

        static void onDisconnectedStatic(const BlePeerDevice &peer, void *context)
        {
            (void)peer;
            ParticleBleBackend *self = static_cast<ParticleBleBackend *>(context);
            if (self != nullptr)
            {
                self->onDisconnected();
            }
        }

        bool notifyTelemetry(const uint8_t *data, std::size_t len)
        {
            if (!registered_ || telemetryCharacteristic_ == nullptr)
            {
                return false;
            }

            return telemetryCharacteristic_->setValue(data, len);
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

        void onControlReceived(const uint8_t *data, std::size_t len)
        {
            if (data == nullptr || len == 0)
            {
                return;
            }

            SF_HAL::ble::Callbacks callbacks = callbacks_;
            if (callbacks.on_write != nullptr)
            {
                callbacks.on_write(CONTROL_HANDLE, data, len, callbacks.context);
            }
        }

        bool registered_;
        BleUuid *serviceUuid_;
        BleUuid *telemetryUuid_;
        BleUuid *controlUuid_;
        BleCharacteristic *telemetryCharacteristic_;
        BleCharacteristic *controlCharacteristic_;
    };

    ParticleBleBackend *backend_ = nullptr;
} // namespace

namespace SF_HAL
{

    bool ble_init(const char *device_name,
                  const char *service_uuid,
                  const char *telemetry_char_uuid,
                  const char *control_char_uuid)
    {
        if (device_name == nullptr || service_uuid == nullptr || telemetry_char_uuid == nullptr ||
            control_char_uuid == nullptr)
        {
            return false;
        }

        if (initialized_.load(std::memory_order_acquire))
        {
            return true;
        }

        BLE.on();
        BLE.setDeviceName(device_name);

        ParticleBleBackend &backend = ParticleBleBackend::getInstance();
        if (!backend.init(service_uuid, telemetry_char_uuid, control_char_uuid))
        {
            return false;
        }

        backend_ = &backend;
        BLE.onConnected(&ParticleBleBackend::onConnectedStatic, &backend);
        BLE.onDisconnected(&ParticleBleBackend::onDisconnectedStatic, &backend);

        initialized_.store(true, std::memory_order_release);
        return true;
    }

void ble_set_callbacks(const ble::Callbacks &callbacks)
{
    callbacks_ = callbacks;
}

bool ble_advertise(const char *service_uuid, const char *local_name)
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

    BleAdvertisingData scanResp;
    if (local_name != nullptr)
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

bool ble_notify(const uint8_t *data, std::size_t len)
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
