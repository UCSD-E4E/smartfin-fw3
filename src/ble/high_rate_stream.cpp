/**
 * @file high_rate_stream.cpp
 * @brief Transport loop for high-rate IMU BLE streaming.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "high_rate_stream.hpp"

#include "ble/sf_ble.hpp"
#include "cli/flog.hpp"
#include "product.hpp"
#include "system.hpp"

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
    packetBuilder_()
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
    running_ = false;
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
void HighRateStream::serviceOnce()
{
    if (!initialized_ || !running_)
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
}

/**
 * @brief Flush any buffered payload immediately to BLE.
 */
void HighRateStream::flush()
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
