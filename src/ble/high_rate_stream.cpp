/**
 * @file high_rate_stream.cpp
 * @brief Transport loop for high-rate IMU BLE streaming.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "high_rate_stream.hpp"

#include "ble/sf_ble.hpp"
#include "ble/ble_live_stream.hpp"
#include "cli/flog.hpp"
#include "product.hpp"
#include "system.hpp"
#include <cstring>

/**
 * @brief Construct the HighRateStream singleton (clears counters/queues).
 */
HighRateStream::HighRateStream() :
    initialized_(false),
    running_(false),
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    transportThread_(nullptr),
#endif
    droppedProducerRecords_(0),
    droppedTransportPackets_(0),
    notifyFailures_(0),
    recordQueue_(),
    packetBuilder_(),
    recorderQueue_(),
    txQueue_()
{
}

HighRateStream& HighRateStream::getInstance()
{
    static HighRateStream instance;
    return instance;
}

/**
 * @brief Reset the builder/queue and counters.
 * @return true always (currently cannot fail).
 */
bool HighRateStream::init()
{
    packetBuilder_.reset();
    sf::ble::transport::TxPacket dummyPacket;
    while (txQueue_.pop(dummyPacket))
    {
    }
    RecorderChunk dummyChunk;
    while (recorderQueue_.pop(dummyChunk))
    {
    }

    HighRateImuRecord dummy;
    while (recordQueue_.pop(dummy))
    {
    }

    droppedProducerRecords_ = 0;
    droppedTransportPackets_ = 0;
    notifyFailures_ = 0;
    initialized_ = true;
    return true;
}

/**
 * @brief Start draining the queue and transmitting over BLE.
 */
void HighRateStream::start()
{
    running_ = true;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    if (transportThread_ == nullptr)
    {
        transportThread_ = new Thread("hr_stream",
                                      HighRateStream::transportLoopThunk,
                                      this,
                                      OS_THREAD_PRIORITY_DEFAULT);
    }
#endif
}

/**
 * @brief Stop streaming; transport loop will exit on next iteration.
 */
void HighRateStream::stop()
{
    // Stop the loop and perform one final drain.
    running_ = false;
    serviceOnce(true);
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    if (transportThread_ != nullptr)
    {
        // Give the thread a moment to exit its loop.
        delay(5);
        delete transportThread_;
        transportThread_ = nullptr;
    }
#endif
}

bool HighRateStream::enqueueRecorderPayload(const void* data, std::size_t len)
{
    if (!initialized_ || !running_ || data == nullptr || len == 0 || len > RECORDER_CHUNK_MAX)
    {
        return false;
    }
    RecorderChunk chunk{};
    chunk.len = len;
    std::memcpy(chunk.bytes, data, len);
    return recorderQueue_.push(chunk);
}

bool HighRateStream::enqueueTxPacket(const sf::ble::transport::TxPacket& packet)
{
    if (!initialized_ || !running_)
    {
        return false;
    }
    return txQueue_.push(packet);
}

/**
 * @brief Enqueue a single IMU record from producer context.
 * @return false if uninitialized, stopped, or queue full.
 */
bool HighRateStream::enqueueImuRecord(const HighRateImuRecord& record)
{
    if (!initialized_ || !running_)
    {
        return false;
    }

    if (!recordQueue_.push(record))
    {
        ++droppedProducerRecords_;
        return false;
    }

    return true;
}

/**
 * @brief Thread entry thunk to call the instance transport loop.
 */
void HighRateStream::transportLoopThunk(void* param)
{
    HighRateStream* self = static_cast<HighRateStream*>(param);
    if (self)
    {
        self->transportLoop();
    }
}

/**
 * @brief Consumer loop that drains queued IMU records and sends via BLE.
 */
void HighRateStream::transportLoop()
{
    while (running_)
    {
        serviceOnce();
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        delay(1);
#endif
    }
}

/**
 * @brief Single transport iteration: pop records, build packets, notify BLE.
 */
void HighRateStream::serviceOnce(bool force)
{
    if (!initialized_)
    {
        return;
    }

    if (!running_ && !force)
    {
        return;
    }

    HighRateImuRecord record;
    while (recordQueue_.pop(record))
    {
#if ENABLE_RECORD_SINK
        if (pSystemDesc && pSystemDesc->pRecorder)
        {
            pSystemDesc->pRecorder->putBytes(&record, sizeof(record));
        }
#endif
        if (!packetBuilder_.canAppend(sizeof(record)))
        {
            sf::ble::transport::TxPacket packet;
            if (packetBuilder_.finalize(packet))
            {
                if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                {
                    ++notifyFailures_;
                }
            }
        }

        if (!packetBuilder_.appendEnsemble(&record, sizeof(record)))
        {
            ++droppedTransportPackets_;
        }

        if (packetBuilder_.remainingPayload() == 0)
        {
            sf::ble::transport::TxPacket packet;
            if (packetBuilder_.finalize(packet))
            {
                if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                {
                    ++notifyFailures_;
                }
            }
        }
    }

    // Drain recorder writes queued by other threads.
    if (pSystemDesc && pSystemDesc->pRecorder)
    {
        RecorderChunk chunk;
        while (recorderQueue_.pop(chunk))
        {
            pSystemDesc->pRecorder->putBytes(chunk.bytes, chunk.len);
        }
    }

    // Drain any TxPackets generated by high-rate or low-rate producers.
    sf::ble::transport::TxPacket packet;
    while (txQueue_.pop(packet))
    {
        if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
        {
            ++notifyFailures_;
        }
    }
}

/**
 * @brief Flush any buffered payload immediately to BLE.
 */
void HighRateStream::flush()
{
    sf::ble::transport::TxPacket packet;
    if (packetBuilder_.finalize(packet))
    {
        txQueue_.push(packet);
    }
}
