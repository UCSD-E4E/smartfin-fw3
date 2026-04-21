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
    : initialized_(false),
      droppedPackets_(0)
{
    timeSync_.valid.store(false, std::memory_order_relaxed);
    timeSync_.boardMillisAtSync = 0;
    timeSync_.watchUnixMsAtSync = 0;
    timeSync_.syncSeq = 0;
    timeSync_.offsetMs = 0;
    timeSync_.offsetEmaMs = 0;
    timeSync_.lastUpdateMs = 0;
    timeSync_.quality = 0;
}

BleLiveStream &BleLiveStream::getInstance()
{
    static BleLiveStream instance;
    return instance;
}

bool BleLiveStream::init()
{
    if (!SFBLE::getInstance().init())
    {
        return false;
    }
    initialized_.store(true, std::memory_order_release);
    droppedPackets_.store(0, std::memory_order_relaxed);
    SFBLE::getInstance().setControlCallback(BleLiveStream::controlRxThunk, this);
    SFBLE::getInstance().startAdvertising();
    return true;
}

bool BleLiveStream::enqueueEnsemble(const void *pData, size_t len)
{
    // Not initialized, shutting down, or bad data
    if (!initialized_.load(std::memory_order_acquire) || pData == nullptr || len == 0 ||
        !TransportService::getInstance().isAccepting())
    {
        return false;
    }
    if (len > sf::ble::transport::MAX_PAYLOAD_SIZE)
    {
        droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // Enqueue raw ensemble into transport-owned builder.
    if (!TransportService::getInstance().enqueueLowRateEnsemble(pData, len))
    {
        droppedPackets_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

void BleLiveStream::finalizePacket()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    // No-op: packetBuilder_ moved to HighRateStream; shutdown() handles flushing.
}

void BleLiveStream::processTx()
{
    // No-op: TransportService handles transmission; left for legacy callers.
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
    const uint32_t nowMs = millis();

    std::lock_guard<std::mutex> lock(timeSyncMutex_);

    if (timeSync_.valid.load(std::memory_order_relaxed) && seq <= timeSync_.syncSeq)
    {
        return;
    }

    // Basic seq gap handling: decay quality on gaps.
    if (timeSync_.valid.load(std::memory_order_relaxed) && seq != (timeSync_.syncSeq + 1))
    {
        if (timeSync_.quality > 10)
        {
            timeSync_.quality = static_cast<uint8_t>(timeSync_.quality - 10);
        }
    }

    // Offset estimate (no RTT field available; assume negligible).
    const int64_t offsetMs = static_cast<int64_t>(watchUnixMs) - static_cast<int64_t>(nowMs);
    if (!timeSync_.valid.load(std::memory_order_relaxed))
    {
        timeSync_.offsetEmaMs = offsetMs;
        timeSync_.quality = 128;
    }
    else
    {
        // EMA with alpha=1/4 for smoothing.
        timeSync_.offsetEmaMs = (timeSync_.offsetEmaMs * 3 + offsetMs) / 4;
        if (timeSync_.quality < 250)
        {
            timeSync_.quality = static_cast<uint8_t>(timeSync_.quality + 5);
        }
    }

    timeSync_.offsetMs = offsetMs;
    timeSync_.boardMillisAtSync = nowMs;
    timeSync_.watchUnixMsAtSync = watchUnixMs;
    timeSync_.lastUpdateMs = nowMs;
    timeSync_.syncSeq = seq;
    timeSync_.valid.store(true, std::memory_order_release);
}

/**
 * @brief Estimate Unix time (s) based on the last sync and current board millis.
 */
uint32_t BleLiveStream::estimateUnixTime(uint32_t boardMillis) const
{
    constexpr uint32_t MAX_SYNC_AGE_MS = 60000; // 60s validity window

    int64_t offsetSnap;
    uint32_t lastUpdate;

    {
        std::lock_guard<std::mutex> lock(timeSyncMutex_);
        if (!timeSync_.valid.load(std::memory_order_relaxed))
        {
            return 0;
        }
        offsetSnap = timeSync_.offsetEmaMs;
        lastUpdate = timeSync_.lastUpdateMs;
    }

    const uint32_t ageMs = boardMillis - lastUpdate;
    if (ageMs > MAX_SYNC_AGE_MS)
    {
        return 0;
    }

    int64_t estimateMs = static_cast<int64_t>(boardMillis) + offsetSnap;
    if (estimateMs <= 0)
    {
        return 0;
    }
    return static_cast<uint32_t>(estimateMs / 1000);
}

bool BleLiveStream::isTimeSynced(uint32_t maxAgeMs) const
{
    uint32_t lastUpdate;
    {
        std::lock_guard<std::mutex> lock(timeSyncMutex_);
        if (!timeSync_.valid.load(std::memory_order_relaxed))
        {
            return false;
        }
        lastUpdate = timeSync_.lastUpdateMs;
    }

    const uint32_t nowMs = millis();
    return (nowMs - lastUpdate) <= maxAgeMs;
}
