/**
 * @file ble_live_stream.cpp
 * @brief Implements the BLE live-stream helper and queueing logic.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble_live_stream.hpp"

#include "ble_config.hpp"
#include "ble_transport.hpp"
#include "sf_ble.hpp"

#include "Particle.h"

#include <cstring>

/**
 * @brief Control message types coming from the watch.
 */
namespace
{
    constexpr uint8_t TIME_SYNC_MSG = 1;

#pragma pack(push, 1)
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
      timeSync_{false, 0, 0, 0},
      initialized_(false),
      droppedPackets_(0)
{
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

    initialized_ = true;
    droppedPackets_ = 0;
    SFBLE::getInstance().startAdvertising();
    SFBLE::getInstance().setControlCallback(BleLiveStream::controlRxThunk, this);
    return true;
}

bool BleLiveStream::enqueueEnsemble(const void *pData, size_t len)
{
    // Not initialized, or something wrong with data
    if (!initialized_ || pData == nullptr || len == 0)
    {
        return false;
    }
    if (len > sf::ble::transport::MAX_PAYLOAD_SIZE)
    {
        ++droppedPackets_;
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
                ++droppedPackets_;
                return false;
            }
        }
    }

    // Copy the ensemble data into the packet buffer.
    if (!packetBuilder_.appendEnsemble(pData, len))
    {
        ++droppedPackets_;
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
    if (!initialized_)
    {
        return;
    }

    sf::ble::transport::TxPacket packet;
    if (packetBuilder_.finalize(packet))
    {
        if (!txQueue_.push(packet))
        {
            ++droppedPackets_;
        }
    }
}

void BleLiveStream::processTx()
{
    if (!initialized_ || !isConnected())
    {
        return;
    }

    sf::ble::transport::TxPacket packet;
    while (txQueue_.pop(packet))
    {
        if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
        {
            ++droppedPackets_;
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

void BleLiveStream::handleTimeSync(uint64_t watchUnixMs, uint32_t seq)
{
    timeSync_.valid = true;
    timeSync_.boardMillisAtSync = millis();
    timeSync_.watchUnixMsAtSync = watchUnixMs;
    timeSync_.syncSeq = seq;
}

uint32_t BleLiveStream::estimateUnixTime(uint32_t boardMillis) const
{
    if (!timeSync_.valid)
    {
        return 0;
    }
    uint64_t delta = boardMillis - timeSync_.boardMillisAtSync;
    uint64_t estimate = timeSync_.watchUnixMsAtSync + delta;
    return static_cast<uint32_t>(estimate / 1000);
}
