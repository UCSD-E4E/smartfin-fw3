/**
 * @file ble_live_stream.hpp
 * @brief BLE live-stream helper that batches ensembles into packets.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */
#ifndef __BLE_LIVE_STREAM_HPP__
#define __BLE_LIVE_STREAM_HPP__

#include "ble_config.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>

/**
 * @brief Singleton that manages the BLE transmit builder/queue.
 *
 * It lets the ride/recorder code enqueue ensemble blobs while the BLE stack
 * drains packets asynchronously. TransportService must be running to send.
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
     * @brief Finalize any partial packet and push it into the transmit queue.
     * Does not send over the air — call processTx() to drain the queue to BLE.
     */
    void finalizePacket();

    /**
     * @brief Legacy no-op; transport is handled by TransportService.
     */
    void processTx();

    /**
     * @brief Estimate Unix time in seconds using last sync plus board millis.
     */
    uint32_t estimateUnixTime(uint32_t boardMillis) const;

    /**
     * @brief Return true if time sync is valid and younger than maxAgeMs.
     */
    bool isTimeSynced(uint32_t maxAgeMs = 60000) const;

    /**
     * @brief Check if a BLE central is currently connected.
     * @return true when connected.
     */
    bool isConnected() const;

private:
    /** @brief Construct the singleton (hide public ctor). */
    BleLiveStream();

    sf::ble::transport::PacketBuilder packetBuilder_;
    sf::util::SpscQueue<sf::ble::transport::TxPacket, SF_BLE_QUEUE_CAPACITY> txQueue_;

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
        bool valid;                 //!< True after at least one sync message.
        uint32_t boardMillisAtSync; //!< Board millis snapshot at sync receipt.
        uint64_t watchUnixMsAtSync; //!< Peer-provided Unix time in ms at sync.
        uint32_t syncSeq;           //!< Sequence counter echoed by peer.
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

    /** @brief Estimate Unix time in seconds using last sync plus board millis. */
    uint32_t estimateUnixTime(uint32_t boardMillis) const;
};

#endif // __BLE_LIVE_STREAM_HPP__
