#include "ble_live_stream.hpp"

#include "ble_config.hpp"
#include "ble_transport.hpp"
#include "sf_ble.hpp"

/**
 * @file ble_live_stream.cpp
 * @brief Implements the BLE live-stream helper and queueing logic.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

BleLiveStream::BleLiveStream()
    : packetBuilder_(), txQueue_(), initialized_(false), droppedPackets_(0)
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
        if (packetBuilder_.finalize(packet, PacketType::PACKET_TYPE_TELEMETRY))
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
        packetBuilder_.reset();
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
