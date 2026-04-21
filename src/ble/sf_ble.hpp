/**
 * @file sf_ble.hpp
 * @brief Platform Smartfin BLE wrapper interface.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#ifndef __SF_BLE_HPP__
#define __SF_BLE_HPP__

#include <atomic>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Smartfin BLE wrapper.
 *
 * Hides Particle BLE types from the rest of the codebase so porting to a
 * different chip requires changes mostly in the implementation file.
 *
 * @note Current intent: peripheral role with one telemetry notify
 * characteristic and an optional control write characteristic
 * inside a single custom service.
 */
class SFBLE
{
public:
    /**
     * @brief Callback invoked when the watch writes to the control characteristic.
     *
     * @param data  Pointer to the raw bytes written by the peer.
     * @param len   Number of bytes in @p data.
     * @param context Caller-supplied pointer passed back verbatim; typically
     *                @c this so the callback can reach instance state without
     *                a global.
     */
    typedef void (*control_rx_callback_t)(const uint8_t *data, size_t len, void *context);

    /**
     * @brief Callback invoked when a central connects or disconnects.
     *
     * @param connected  @c true on connect, @c false on disconnect.
     * @param context Caller-supplied pointer passed back verbatim; typically
     *                @c this so the callback can reach instance state without
     *                a global.
     */
    typedef void (*connection_callback_t)(bool connected, void* context);

    /** @brief Get singleton instance. */
    static SFBLE& getInstance(void);

    /**
     * @brief Initialize BLE stack and register service/characteristics.
     * @return true on success, false on failure.
     */
    bool init(void);

    /**
     * @brief Start BLE advertising.
     * @return true on success, false on failure.
     */
    bool startAdvertising(void);

    /**
     * @brief Stop BLE advertising.
     * @return true on success, false on failure.
     */
    bool stopAdvertising(void);

    /**
     * @brief Returns true if a central is connected.
     * @return true when a peer is connected, otherwise false.
     */
    bool isConnected(void) const;

    /**
     * @brief Send telemetry bytes to connected central using NOTIFY.
     *
     * @param pData Pointer to telemetry payload.
     * @param len Number of bytes to send.
     * @return true on success, false on failure.
     */
    bool notifyTelemetry(const void* pData, size_t len);

    /**
     * @brief Register callback for incoming control data.
     *
     * @param cb Callback function invoked on received control bytes.
     * @param context User context pointer passed to the callback.
     */
    void setControlCallback(control_rx_callback_t cb, void* context);

    /**
     * @brief Register callback for connect/disconnect events.
     *
     * @param cb Callback function invoked on connection state changes.
     * @param context User context pointer passed to the callback.
     */
    void setConnectionCallback(connection_callback_t cb, void* context);
    /**
     * @brief Internal helper invoked by platform backend on connect/disconnect.
     *
     * Updates internal state and forwards to the user callback if registered.
     *
     * @param isConnected true if connected, false if disconnected
     */
    void handleConnectionEvent(bool isConnected);

    /**
     * @brief Internal helper invoked by platform backend on control RX.
     *
     * Forwards received control data to the registered callback if present.
     *
     * @param data Pointer to received data
     * @param len Number of bytes received
     */
    void handleControlEvent(const uint8_t *data, size_t len);

private:
    /** @brief Private default constructor to enforce singleton. */
    SFBLE();

    /** @brief Deleted copy constructor (singleton). */
    SFBLE(const SFBLE&);

    /** @brief Deleted assignment operator (singleton). */
    SFBLE& operator=(const SFBLE&);

    /** @brief True after BLE stack/characteristics are initialized. */
    std::atomic<bool> initialized;

    /** @brief True when a central is currently connected. */
    std::atomic<bool> connected;

    /** @brief Registered control data callback. */
    std::atomic<control_rx_callback_t> controlCallback;

    /** @brief User context for control callback. */
    void* controlContext;

    /** @brief Registered connection state callback. */
    std::atomic<connection_callback_t> connectionCallback;

    /** @brief User context for connection callback. */
    void* connectionContext;
};

#endif // __SF_BLE_HPP__
