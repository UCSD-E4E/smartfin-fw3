/**
 * @file sf_ble.cpp
 * @brief Smartfin BLE wrapper implementation using the platform HAL.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 3-9-2026
 */

#include "sf_ble.hpp"

#include <atomic>

#include "platform/hal.hpp"
#include "product.hpp"
#include "sf_ble_defs.hpp"

namespace
{
    void bleConnectionThunk(bool isConnected, void* context)
    {
        SFBLE* ble = static_cast<SFBLE*>(context);
        if (ble != nullptr)
        {
            ble->handleConnectionEvent(isConnected);
        }
    }

    void bleControlThunk(SF_HAL::ble::CharHandle characteristic,
                         const uint8_t* data,
                         std::size_t len,
                         void* context)
    {
        (void)characteristic;

        SFBLE* ble = static_cast<SFBLE*>(context);
        if (ble != nullptr)
        {
            ble->handleControlEvent(data, len);
        }
    }
} // namespace

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

    SF_HAL::ble::Callbacks callbacks{
        bleConnectionThunk,
        bleControlThunk,
        this,
    };

    SF_HAL::ble_set_callbacks(callbacks);
    if (!SF_HAL::ble_init(sf::bledefs::DEVICE_NAME,
                          sf::bledefs::SERVICE_UUID,
                          sf::bledefs::TELEMETRY_CHAR_UUID,
                          sf::bledefs::CONTROL_CHAR_UUID))
    {
        return false;
    }

    this->initialized.store(true, std::memory_order_release);
    return true;
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

    return SF_HAL::ble_advertise(sf::bledefs::SERVICE_UUID, sf::bledefs::DEVICE_NAME);
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

    SF_HAL::ble_stop_advertising();
    return true;
}

/**
 * @brief Report connection status.
 * @return true when a central is connected, else false.
 */
bool SFBLE::isConnected(void) const
{
    if (!this->initialized.load(std::memory_order_acquire))
    {
        return false;
    }

    return SF_HAL::ble_is_connected();
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

    return SF_HAL::ble_notify(static_cast<const uint8_t*>(pData), len);
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
