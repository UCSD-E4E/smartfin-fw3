/**
 * @file ble_live_stream.cpp
 * @brief Implements the BLE live-stream helper and queueing logic.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble_live_stream.hpp"

#include "ble_config.hpp"
#include "ble_transport.hpp"
#include "ble/high_rate_stream.hpp"
#include "sf_ble.hpp"

#include "Particle.h"

#include <cstring>

namespace
{
    /** @brief Control message type identifiers sent by the watch (out-of-band from transport). */
    constexpr uint8_t TIME_SYNC_MSG = 1;

#pragma pack(push, 1)
    /**
     * @brief Payload layout for a time-sync control message.
     */
    struct TimeSyncMsg
    {
        uint8_t type;
        uint32_t seq;
        uint64_t watchUnixMs;
    };
#pragma pack(pop)
}

BleLiveStream::BleLiveStream()
    : packetBuilder_(),
      txQueue_(),
      initialized_(false),
      droppedPackets_(0)
{
    timeSync_.valid.store(false, std::memory_order_relaxed);
    timeSync_.boardMillisAtSync = 0;
    timeSync_.watchUnixMsAtSync = 0;
    timeSync_.syncSeq = 0;
}

BleLiveStream &BleLiveStream::getInstance()
{
    static BleLiveStream instance;
    return instance;
}

bool BleLiveStream::init()
{
    packetBuilder_.reset();
    if (!SFBLE::getInstance().init())
    {
        return false;
    }
    sf::ble::transport::TxPacket packet;
    while (txQueue_.pop(packet))
    {
    }

    initialized_.store(true, std::memory_order_release);
    droppedPackets_.store(0, std::memory_order_relaxed);
    SFBLE::getInstance().setControlCallback(BleLiveStream::controlRxThunk, this);
    SFBLE::getInstance().startAdvertising();
    return true;
}

bool BleLiveStream::enqueueEnsemble(const void *pData, size_t len)
{
    // Not initialized, or something wrong with data
    if (!initialized_.load(std::memory_order_acquire) || pData == nullptr || len == 0)
    {
        return false;
    }
    if (len > sf::ble::transport::MAX_PAYLOAD_SIZE)
    {
        droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // If the current packet is full, finalize and queue it before appending.
    // handles current packet full
    if (!packetBuilder_.canAppend(len))
    {
        sf::ble::transport::TxPacket packet;
        if (packetBuilder_.finalize(packet, sf::ble::transport::PACKET_TYPE_TELEMETRY))
        {
            if (!txQueue_.push(packet))
            {
                droppedPackets_.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
        }
    }

    // Copy the ensemble data into the packet buffer.
    if (!packetBuilder_.appendEnsemble(pData, len))
    {
        droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // If we exactly filled the packet, move it to the queue right away.
    if (packetBuilder_.remainingPayload() == 0)
    {
        flush();
    }

    return true;
}

void BleLiveStream::flush()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    sf::ble::transport::TxPacket packet;
    if (packetBuilder_.finalize(packet))
    {
        if (!txQueue_.push(packet))
        {
            droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

void BleLiveStream::processTx()
{
    if (!initialized_.load(std::memory_order_acquire) || !isConnected())
    {
        return;
    }

    sf::ble::transport::TxPacket packet;
    while (txQueue_.pop(packet))
    {
        if (!HighRateStream::getInstance().enqueueTxPacket(packet))
        {
            droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

bool BleLiveStream::isConnected() const
{
    return SFBLE::getInstance().isConnected();
}

void BleLiveStream::controlRxThunk(const uint8_t* data, size_t len, void* context)
{
    BleLiveStream* self = static_cast<BleLiveStream*>(context);
    if (self != nullptr)
    {
        self->handleControlRx(data, len);
    }
}

/**
 * @brief Process an incoming control message from the central.
 *
 * Currently only handles time-sync messages; silently ignores malformed input.
 */
void BleLiveStream::handleControlRx(const uint8_t* data, size_t len)
{
    if (data == nullptr || len < sizeof(TimeSyncMsg))
    {
        return;
    }

    TimeSyncMsg msg;
    std::memcpy(&msg, data, sizeof(msg));
    if (msg.type == TIME_SYNC_MSG)
    {
        handleTimeSync(msg.watchUnixMs, msg.seq);
    }
}

/**
 * @brief Update local time estimate using a watch-provided Unix time snapshot.
 */
void BleLiveStream::handleTimeSync(uint64_t watchUnixMs, uint32_t seq)
{
    std::lock_guard<std::mutex> lock(timeSyncMutex_);
    timeSync_.boardMillisAtSync = millis();
    timeSync_.watchUnixMsAtSync = watchUnixMs;
    timeSync_.syncSeq = seq;
    timeSync_.valid.store(true, std::memory_order_release);
    timeSyncVersion_.fetch_add(1, std::memory_order_relaxed);
}

/**
 * @brief Estimate Unix time (s) based on the last sync and current board millis.
 */
uint32_t BleLiveStream::estimateUnixTime(uint32_t boardMillis) const
{
    // Double-checked snapshot to avoid lock on common fast path.
    if (!timeSync_.valid.load(std::memory_order_acquire))
    {
        return 0;
    }

    uint32_t startVer = timeSyncVersion_.load(std::memory_order_acquire);
    uint32_t boardSnap;
    uint64_t watchSnap;
    uint32_t seqSnap;

    {
        std::lock_guard<std::mutex> lock(timeSyncMutex_);
        boardSnap = timeSync_.boardMillisAtSync;
        watchSnap = timeSync_.watchUnixMsAtSync;
        seqSnap = timeSync_.syncSeq;
    }

    uint32_t endVer = timeSyncVersion_.load(std::memory_order_acquire);
    if (startVer != endVer || !timeSync_.valid.load(std::memory_order_acquire))
    {
        return 0;
    }

    (void)seqSnap; // currently unused, kept for future validation/logging

    uint64_t delta = boardMillis - boardSnap;
    uint64_t estimate = watchSnap + delta;
    return static_cast<uint32_t>(estimate / 1000);
}
