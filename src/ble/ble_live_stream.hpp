#ifndef __BLE_LIVE_STREAM_HPP__
#define __BLE_LIVE_STREAM_HPP__

/**
 * @file ble_live_stream.hpp
 * @brief BLE live-stream helper that batches ensembles into packets.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include <cstddef>
#include <cstdint>

#include "ble_config.hpp"
#include "ble_packet_builder.hpp"
#include "spsc_queue.hpp"

/**
 * @brief Singleton that manages the BLE transmit builder/queue.
 *
 * It lets the ride/recorder code enqueue ensemble blobs while the BLE stack
 * drains packets asynchronously.
 */
class BleLiveStream
{
public:
    static BleLiveStream& getInstance();

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
    bool enqueueEnsemble(const void* pData, size_t len);

    /**
     * @brief Flush the builder into the transmit queue immediately.
     */
    void flush();

    /**
     * @brief Drain queued packets and hand them to the BLE backend.
     */
    void processTx();

    /**
     * @brief Check if a BLE central is currently connected.
     * @return true when connected.
     */
    bool isConnected() const;

private:
    /** @brief Construct the singleton (hide public ctor). */
    BleLiveStream();

    sf::ble::transport::PacketBuilder packetBuilder_;
    sf::util::SpscQueue<sf::ble::transport::TxPacket,
                        SF_BLE_QUEUE_CAPACITY> txQueue_;

    bool initialized_;        //!< True when `init()` completed.
    uint32_t droppedPackets_; //!< Count of dropped/overflow packets.
};

#endif // __BLE_LIVE_STREAM_HPP__
