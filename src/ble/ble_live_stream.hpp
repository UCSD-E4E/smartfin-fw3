/**
 * @file ble_live_stream.hpp
 * @brief BLE live-stream helper that batches ensembles into packets.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */
#ifndef __BLE_LIVE_STREAM_HPP__
#define __BLE_LIVE_STREAM_HPP__

#include "ble_config.hpp"
#include "ble_packet_builder.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>

/**
 * @brief Singleton that manages the BLE transmit builder/queue.
 *
 * It lets the ride/recorder code enqueue ensemble blobs while the BLE stack
 * drains packets asynchronously. TransportWorker must be running to send.
 */
class BleLiveStream
{
public:
    static BleLiveStream &getInstance();

    /**
     * @brief Reset internal buffers and clear the queue.
     * @return true on success.
     */
    bool init();

    /**
     * @brief Append an ensemble to the current packet stream.
     * @param pData Pointer to ensemble bytes.
     * @param len Number of bytes.
     * @return true if enqueued successfully.
     */
    bool enqueueEnsemble(const void *pData, size_t len);

    /**
     * @brief Flush the builder into the transmit queue immediately.
     */
    void flush();

    /**
     * @brief Legacy no-op; transport is handled by TransportWorker.
     */
    void processTx();

    /**
     * @brief Estimate Unix time in seconds using last sync plus board millis.
     */
    uint32_t estimateUnixTime(uint32_t boardMillis) const;

    /**
     * @brief Check if a BLE central is currently connected.
     * @return true when connected.
     */
    bool isConnected() const;

    /**
     * @brief Estimate Unix time in seconds using last sync plus board millis.
     */
    uint32_t estimateUnixTime(uint32_t boardMillis) const;

private:
    /** @brief Construct the singleton (hide public ctor). */
    BleLiveStream();

    sf::ble::transport::PacketBuilder packetBuilder_;

    struct TimeSyncState
    {
        std::atomic<bool> valid; //!< True after at least one sync message.
        // Updated under timeSyncMutex_ to publish a consistent snapshot.
        uint32_t boardMillisAtSync;
        uint64_t watchUnixMsAtSync;
        uint32_t syncSeq;
        int64_t offsetMs;      //!< Raw offset (watchUnixMs - boardMillis).
        int64_t offsetEmaMs;   //!< Smoothed offset estimate.
        uint32_t lastUpdateMs; //!< Board millis when last sync applied.
        uint8_t quality;       //!< Simple quality score (0-255).
    };

    TimeSyncState timeSync_;
    mutable std::mutex timeSyncMutex_;

    std::atomic<bool> initialized_; //!< True when `init()` completed.
    std::atomic<uint32_t> droppedPackets_; //!< Count of dropped/overflow packets.

    /** @brief Handle control-channel RX and dispatch message types. */
    void handleControlRx(const uint8_t *data, size_t len);
    /** @brief Update local time sync state from a remote watch timestamp. */
    void handleTimeSync(uint64_t watchUnixMs, uint32_t seq);
    /** @brief Static thunk registered with SFBLE for control RX. */
    static void controlRxThunk(const uint8_t *data, size_t len, void *context);

};

#endif // __BLE_LIVE_STREAM_HPP__
