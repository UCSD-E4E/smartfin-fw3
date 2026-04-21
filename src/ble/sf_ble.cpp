/**
 * @file sf_ble.cpp
 * @brief Particle-backed implementation of the platform-adaptable BLE wrapper.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 3-9-2026
 */

#include "sf_ble.hpp"

#include <atomic>

#include "cli/conio.hpp"
#include "product.hpp"
#include "sf_ble_defs.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

#if SF_PLATFORM == SF_PLATFORM_PARTICLE

namespace
{
    /** @brief Particle-specific UUID objects. */
    BleUuid g_serviceUuid(sf::bledefs::SERVICE_UUID);
    BleUuid g_telemetryUuid(sf::bledefs::TELEMETRY_CHAR_UUID);
    BleUuid g_controlUuid(sf::bledefs::CONTROL_CHAR_UUID);

    /**
     * @brief Particle BLE backend that wires characteristics and event hooks.
     *
     * This class is intentionally confined to this translation unit so the rest
     * of the codebase does not depend on Particle BLE types.
     */
    class ParticleBleBackend
    {
    public:
        /**
         * @brief Construct and register both GATT characteristics.
         *
         * Particle registers a @c BleCharacteristic into the GATT server at
         * construction time, so both characteristics must be fully described
         * here rather than in @c init().
         *
         * @c telemetryCharacteristic — short name @c "tele", property NOTIFY,
         * carries fin→watch data. No write callback; the central subscribes and
         * receives notifications pushed by @c notifyTelemetry().
         *
         * @c controlCharacteristic — short name @c "ctrl", property
         * WRITE_WO_RSP, carries watch→fin commands. Registers
         * @c onControlReceivedStatic as the write callback with @c this as
         * context so Particle can invoke it as a plain C function pointer.
         */
        ParticleBleBackend()
            : telemetryCharacteristic("tele",
                                      BleCharacteristicProperty::NOTIFY,
                                      g_telemetryUuid,
                                      g_serviceUuid),
              controlCharacteristic("ctrl",
                                    BleCharacteristicProperty::WRITE_WO_RSP,
                                    g_controlUuid,
                                    g_serviceUuid,
                                    ParticleBleBackend::onControlReceivedStatic,
                                    this)
        {
        }

        /** @brief Telemetry characteristic for fin -> watch data. */
        BleCharacteristic telemetryCharacteristic;

        /** @brief Control characteristic for watch -> fin commands. */
        BleCharacteristic controlCharacteristic;

        /**
         * @brief Access the singleton backend instance.
         *
         * A single instance is required because each @c BleCharacteristic
         * object represents one physical GATT entry. Constructing a second
         * instance would attempt to register duplicate characteristics and
         * corrupt the GATT table. The static-local pattern also controls
         * construction order: the instance is created on the first call to
         * @c getInstance(), which happens inside @c SFBLE::init() after
         * @c BLE.on(), guaranteeing the GATT server is live before any
         * characteristic tries to register.
         */
        static ParticleBleBackend &getInstance()
        {
            static ParticleBleBackend instance;
            return instance;
        }

        /**
         * @brief Particle control-write callback shim.
         * @param data Received payload.
         * @param len Number of bytes in payload.
         * @param peer Peer device (unused).
         * @param context Pointer to backend instance.
         */
        static void onControlReceivedStatic(const uint8_t *data,
                                            size_t len,
                                            const BlePeerDevice &peer,
                                            void *context)
        {
            (void)peer;
            ParticleBleBackend *self = static_cast<ParticleBleBackend *>(context);
            if (self)
            {
                self->onControlReceived(data, len);
            }
        }

        /**
         * @brief Handle Particle connect event.
         * @param peer Connected peer (unused).
         */
        void onConnected(const BlePeerDevice & /*peer*/)
        {
            SFBLE &ble = SFBLE::getInstance();
            bleConnectionThunk(true, &ble);
        }

        /**
         * @brief Handle Particle disconnect event.
         * @param peer Disconnected peer (unused).
         */
        void onDisconnected(const BlePeerDevice & /*peer*/)
        {
            SFBLE &ble = SFBLE::getInstance();
            bleConnectionThunk(false, &ble);
        }

        /**
         * @brief Forward control writes to wrapper.
         * @param data Control payload bytes.
         * @param len Payload length.
         */
        void onControlReceived(const uint8_t *data, size_t len)
        {
            SFBLE &ble = SFBLE::getInstance();
            bleControlThunk(data, len, &ble);
        }

        /**
         * @brief Bridge connection events to SFBLE instance.
         * @param isConnected True if connected, false otherwise.
         * @param context Pointer to SFBLE instance.
         */
        static void bleConnectionThunk(bool isConnected, void *context)
        {
            SFBLE *ble = static_cast<SFBLE *>(context);
            if (!ble)
            {
                return;
            }

            ble->handleConnectionEvent(isConnected);
        }

        /**
         * @brief Bridge control writes to SFBLE instance.
         * @param data Control payload.
         * @param len Payload length.
         * @param context Pointer to SFBLE instance.
         */
        static void bleControlThunk(const uint8_t *data, size_t len, void *context)
        {
            SFBLE *ble = static_cast<SFBLE *>(context);
            if (!ble)
            {
                return;
            }

            ble->handleControlEvent(data, len);
        }
    };
} // namespace

#endif // SF_PLATFORM == SF_PLATFORM_PARTICLE

/**
 * @brief Get singleton SFBLE instance.
 * @return Reference to SFBLE.
 */
SFBLE &SFBLE::getInstance(void)
{
    static SFBLE instance;
    return instance;
}

/**
 * @brief Construct default wrapper state.
 */
SFBLE::SFBLE()
    : initialized(false), connected(false), controlCallback(nullptr), controlContext(nullptr),
      connectionCallback(nullptr), connectionContext(nullptr)
{
}

/**
 * @brief Internal connection-event handler.
 * @param isConnected true if connected, false otherwise.
 */
void SFBLE::handleConnectionEvent(bool isConnected)
{
    this->connected.store(isConnected, std::memory_order_release);

    auto cb  = this->connectionCallback.load(std::memory_order_acquire);
    auto ctx = this->connectionContext;
    if (cb)
    {
        cb(isConnected, ctx);
    }
}

/**
 * @brief Internal control-event handler.
 * @param data Pointer to received payload.
 * @param len Payload length.
 */
void SFBLE::handleControlEvent(const uint8_t *data, size_t len)
{
    auto cb  = this->controlCallback.load(std::memory_order_acquire);
    auto ctx = this->controlContext;
    if (cb)
    {
        cb(data, len, ctx);
    }
}

/**
 * @brief Initialize BLE stack and register characteristics.
 * @return true on success, false otherwise.
 */
bool SFBLE::init(void)
{
    if (this->initialized.load(std::memory_order_acquire))
    {
        return true;
    }

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ParticleBleBackend &backend = ParticleBleBackend::getInstance();

    BLE.on();
    BLE.setDeviceName(sf::bledefs::DEVICE_NAME);
    BLE.onConnected(&ParticleBleBackend::onConnected, &backend);
    BLE.onDisconnected(&ParticleBleBackend::onDisconnected, &backend);

    // Ensure characteristic objects are instantiated before advertising.
    (void)backend.telemetryCharacteristic;
    (void)backend.controlCharacteristic;

    this->initialized.store(true, std::memory_order_release);
    return true;
#else
    return false;
#endif
}

/**
 * @brief Begin advertising the Smartfin BLE service.
 * @return true on success, false otherwise.
 */
bool SFBLE::startAdvertising(void)
{
    if (!this->initialized.load(std::memory_order_acquire))
    {
        return false;
    }

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    BleAdvertisingData advData;
    advData.appendServiceUUID(BleUuid(sf::bledefs::SERVICE_UUID));
    BLE.advertise(&advData);
    return true;
#else
    return false;
#endif
}

/**
 * @brief Stop BLE advertising.
 * @return true on success, false otherwise.
 */
bool SFBLE::stopAdvertising(void)
{
    if (!this->initialized.load(std::memory_order_acquire))
    {
        return false;
    }

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    BLE.stopAdvertising();
    return true;
#else
    return false;
#endif
}

/**
 * @brief Report connection status.
 * @return true when a central is connected, else false.
 */
bool SFBLE::isConnected(void) const
{
    return this->connected.load(std::memory_order_acquire);
}

/**
 * @brief Notify telemetry payload to connected central.
 * @param pData Pointer to payload.
 * @param len Payload length in bytes.
 * @return true on success, false on failure.
 */
bool SFBLE::notifyTelemetry(const void *pData, size_t len)
{
    if (!this->initialized.load(std::memory_order_acquire) ||
        !this->connected.load(std::memory_order_acquire) || !pData)
    {
        return false;
    }

    if (len == 0 || len > sf::bledefs::MAX_NOTIFY_LEN)
    {
        return false;
    }

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ParticleBleBackend &backend = ParticleBleBackend::getInstance();
    return backend.telemetryCharacteristic.setValue(static_cast<const uint8_t *>(pData), len);
#else
    return false;
#endif
}

/**
 * @brief Register callback for incoming control data.
 * @param cb Callback function pointer.
 * @param context User context passed to callback.
 */
void SFBLE::setControlCallback(control_rx_callback_t cb, void *context)
{
    this->controlContext = context;
    this->controlCallback.store(cb, std::memory_order_release);
}

/**
 * @brief Register callback for connection state changes.
 * @param cb Callback function pointer.
 * @param context User context passed to callback.
 */
void SFBLE::setConnectionCallback(connection_callback_t cb, void *context)
{
    this->connectionContext = context;
    this->connectionCallback.store(cb, std::memory_order_release);
}
