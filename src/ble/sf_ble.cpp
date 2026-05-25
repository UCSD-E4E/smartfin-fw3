/**
 * @file sf_ble.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Platform-agnostic BLE wrapper implementation.
 * @date 2026-05-17
 */

#include "sf_ble.hpp"

#include "platform/hal.hpp"
#include "sf_ble_defs.hpp"

#include <atomic>

namespace
{

/**
 * @brief Thunk from HAL connection callback into SFBLE::handleConnectionEvent.
 * @param isConnected true on connect, false on disconnect.
 * @param ctx Pointer to the SFBLE singleton.
 */
void onConnectionThunk(bool isConnected, void* ctx)
{
    auto* ble = static_cast<SFBLE*>(ctx);
    if (ble)
    {
        ble->handleConnectionEvent(isConnected);
    }
}

/**
 * @brief Thunk from HAL write callback into SFBLE::handleControlEvent.
 * @param characteristic Unused — only one control characteristic exists.
 * @param data Received payload.
 * @param len Payload length in bytes.
 * @param ctx Pointer to the SFBLE singleton.
 */
void onWriteThunk(SF_HAL::ble::CharHandle /*characteristic*/,
                  const uint8_t* data,
                  std::size_t len,
                  void* ctx)
{
    auto* ble = static_cast<SFBLE*>(ctx);
    if (ble)
    {
        ble->handleControlEvent(data, len);
    }
}

} // namespace

/**
 * @brief Get singleton SFBLE instance.
 * @return Reference to SFBLE.
 */
SFBLE& SFBLE::getInstance(void)
{
    static SFBLE instance;
    return instance;
}

/**
 * @brief Construct default wrapper state.
 */
SFBLE::SFBLE()
    : initialized(false), connected(false), advertising(false), send_sync_on_next_ensemble(false),
      connection_tick(0), connection_time(0), controlCallback(nullptr), controlContext(nullptr),
      connectionCallback(nullptr), connectionContext(nullptr)
{
}

/**
 * @brief Internal connection-event handler.
 * @param isConnected true if connected, false if disconnected.
 */
void SFBLE::handleConnectionEvent(bool isConnected)
{
    // Called from the BLE thread — no Serial/BLE API here.
    this->connected.store(isConnected, std::memory_order_release);
    if (isConnected)
    {
        this->advertising.store(false, std::memory_order_release);

        // Record connection time and tick for ensemble synchronization
#pragma pack(push, 1)
        struct
        {
            sf::deploy::EnsembleHeader_t header;
            sf::deploy::Ensemble08_data_t data;
        } ens;
#pragma pack(pop)

        ens.header.ensembleType = ENS_TEMP_TIME;
        ens.header.elapsedTime_ms = sf::deploy::Ens_getStartTime();
        ens.data.scaled_temp = 0;
        ens.data.water = 0;
        ens.data.timestamp = SF_HAL::time_now();
        ens.data.tick = SF_HAL::system_ticks();

        // Save connection snapshot for later BLE transmission
        this->connection_tick = ens.data.tick;
        this->connection_time = ens.data.timestamp;
        this->send_sync_on_next_ensemble.store(true, std::memory_order_release);

        // Still commit to SD card immediately
        sf::deploy::commitEnsemble(&ens, sizeof(ens));
    }

    auto cb  = this->connectionCallback.load(std::memory_order_acquire);
    auto ctx = this->connectionContext;
    if (cb)
    {
        cb(isConnected, ctx);
    }
}

uint32_t SFBLE::getConnectionTick() const
{
    return this->connection_tick;
}

uint32_t SFBLE::getConnectionTime() const
{
    return this->connection_time;
}

bool SFBLE::popSendSyncFlag()
{
    return this->send_sync_on_next_ensemble.exchange(false, std::memory_order_acquire);
}

/**
 * @brief Internal control-event handler.
 * @param data Pointer to received payload.
 * @param len Payload length.
 */
void SFBLE::handleControlEvent(const uint8_t* data, size_t len)
{
    // Called from the BLE thread — no Serial/BLE API here.
    if (!data || len == 0)
    {
        return;
    }

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

    SF_HAL::ble::Callbacks cbs;
    cbs.on_connection = onConnectionThunk;
    cbs.on_write      = onWriteThunk;
    cbs.context       = this;
    SF_HAL::ble_set_callbacks(cbs);

    bool ok = SF_HAL::ble_init(sf::bledefs::DEVICE_NAME,
                                sf::bledefs::SERVICE_UUID,
                                sf::bledefs::TELEMETRY_CHAR_UUID,
                                sf::bledefs::CONTROL_CHAR_UUID);
    if (ok)
    {
        this->initialized.store(true, std::memory_order_release);
    }
    return ok;
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

    bool ok = SF_HAL::ble_advertise(sf::bledefs::SERVICE_UUID, sf::bledefs::DEVICE_NAME);
    if (ok)
    {
        this->advertising.store(true, std::memory_order_release);
    }
    return ok;
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
    this->advertising.store(false, std::memory_order_release);
    return true;
}

/**
 * @brief Report initialization status.
 * @return true after a successful init(), else false.
 */
bool SFBLE::isInitialized(void) const
{
    return this->initialized.load(std::memory_order_acquire);
}

/**
 * @brief Report advertising status.
 * @return true when advertising is active, else false.
 */
bool SFBLE::isAdvertising(void) const
{
    return this->advertising.load(std::memory_order_acquire);
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
bool SFBLE::notifyTelemetry(const void* pData, size_t len)
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
void SFBLE::setControlCallback(control_rx_callback_t cb, void* context)
{
    this->controlContext = context;
    this->controlCallback.store(cb, std::memory_order_release);
}

/**
 * @brief Register callback for connection state changes.
 * @param cb Callback function pointer.
 * @param context User context passed to callback.
 */
void SFBLE::setConnectionCallback(connection_callback_t cb, void* context)
{
    this->connectionContext = context;
    this->connectionCallback.store(cb, std::memory_order_release);
}
