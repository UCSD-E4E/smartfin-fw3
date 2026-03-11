#ifndef __HIGH_RATE_STREAM_HPP__
#define __HIGH_RATE_STREAM_HPP__

/**
 * @file high_rate_stream.hpp
 * @brief Producer/consumer pipeline for high-rate IMU records.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble/ble_packet_builder.hpp"
#include "ble/high_rate_record.hpp"
#include "ble/spsc_queue.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

/**
 * @brief Singleton that shuttles high-rate IMU records to BLE packets.
 *
 * Producers enqueue fixed-size IMU records; a consumer thread drains the queue,
 * batches them into BLE telemetry packets, and notifies the connected central.
 */
class HighRateStream
{
public:
    /** @brief Access the singleton instance. */
    static HighRateStream& getInstance();

    /** @brief Reset queues/counters; safe to call before start(). */
    bool init();
    /** @brief Begin streaming (spawns transport thread on Particle). */
    void start();
    /** @brief Stop streaming; transport loop will exit. */
    void stop();

    /**
     * @brief Enqueue a single IMU record from the producer thread.
     * @return false if the queue is full or stream not running.
     */
    bool enqueueImuRecord(const HighRateImuRecord& record);

    /**
     * @brief Transport-side service loop; called only by the transport thread.
     */
    void serviceOnce();

    /**
     * @brief Flush any pending packet to BLE immediately.
     */
    void flush();

    /**
     * @brief Enqueue a low-rate ensemble payload for recorder + BLE handling.
     * @param data Pointer to bytes to copy.
     * @param len Number of bytes to copy (must fit internal buffer).
     * @return true if accepted into the queue.
     */
    bool enqueueRecorderPayload(const void* data, std::size_t len);

    /**
     * @brief Enqueue an already-built BLE TxPacket for transmission.
     * @return true if queued successfully.
     */
    bool enqueueTxPacket(const sf::ble::transport::TxPacket& packet);

private:
    /** @brief Private ctor to enforce singleton. */
    HighRateStream();

    /** @brief Consumer loop that drains the queue and builds packets. */
    void transportLoop();
    /** @brief Thread entry thunk that calls transportLoop(). */
    static void transportLoopThunk(void* param);

    /** @brief True after init() succeeds. */
    std::atomic<bool> initialized_;
    /** @brief True while the stream is running. */
    std::atomic<bool> running_;
    /** @brief True when a stop has been requested (drain remaining work). */
    std::atomic<bool> stopRequested_;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    /** @brief Background thread handle for transportLoop(). */
    Thread* transportThread_;
#endif

    /** @brief Count of producer drops due to full queue. */
    uint32_t droppedProducerRecords_;
    /** @brief Count of transport-side packet drops. */
    uint32_t droppedTransportPackets_;
    /** @brief Count of BLE notify failures. */
    uint32_t notifyFailures_;

    /** @brief Lock-free queue holding pending IMU records. */
    sf::util::SpscQueue<HighRateImuRecord, 512> recordQueue_;
    /** @brief Builder used to batch IMU records into BLE packets. */
    sf::ble::transport::PacketBuilder packetBuilder_;

    /** @brief Maximum bytes per recorder payload chunk. */
    static constexpr std::size_t RECORDER_CHUNK_MAX = sf::ble::transport::MAX_PACKET_SIZE;

    /** @brief Recorder payload chunk. */
    struct RecorderChunk
    {
        std::size_t len;
        uint8_t bytes[RECORDER_CHUNK_MAX];
    };

    /** @brief Queue of recorder writes serialized to the transport thread (single producer). */
    sf::util::SpscQueue<RecorderChunk, 64> recorderQueue_;

    /** @brief Queue of TxPackets produced by other threads; drained here (single producer). */
    sf::util::SpscQueue<sf::ble::transport::TxPacket, 64> txQueue_;
};

#endif // __HIGH_RATE_STREAM_HPP__
