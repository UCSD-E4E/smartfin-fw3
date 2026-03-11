/**
 * @file high_rate_stream.cpp
 * @brief Unified transport worker for all BLE telemetry and recorder IO.
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
 * @brief Construct the TransportWorker singleton (clears counters/queues).
 */
TransportWorker::TransportWorker() :
    initialized_(false),
    running_(false),
    stopRequested_(false),
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    transportThread_(nullptr),
#endif
    transportActive_(false),
    droppedProducerRecords_(0),
    droppedTransportPackets_(0),
    notifyFailures_(0),
    recordQueue_(),
    packetBuilder_(),
    recorderQueue_(),
    txQueue_()
{
}

TransportWorker& TransportWorker::getInstance()
{
    static TransportWorker instance;
    return instance;
}

/**
 * @brief Reset the builder/queue and counters.
 * @return true always (currently cannot fail).
 */
bool TransportWorker::init()
{
    // Lazily create persistent transport thread once.
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    if (transportThread_ == nullptr)
    {
        transportThread_ = new Thread("transport_worker",
                                      TransportWorker::transportLoopThunk,
                                      this,
                                      OS_THREAD_PRIORITY_DEFAULT);
    }
#endif

    packetBuilder_.reset();
    stopRequested_.store(false, std::memory_order_release);
    transportActive_.store(false, std::memory_order_release);
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

    droppedProducerRecords_.store(0, std::memory_order_relaxed);
    droppedTransportPackets_.store(0, std::memory_order_relaxed);
    notifyFailures_.store(0, std::memory_order_relaxed);
    initialized_.store(true, std::memory_order_release);
    return true;
}

/**
 * @brief Start draining the queue and transmitting over BLE.
 */
void TransportWorker::start()
{
    running_.store(true, std::memory_order_release);
    stopRequested_.store(false, std::memory_order_release);
}

/**
 * @brief Stop streaming; transport loop will exit on next iteration.
 */
void TransportWorker::stop()
{
    stopRequested_.store(true, std::memory_order_release);
    running_.store(false, std::memory_order_release);
    // Wait for transport thread to finish and queues to drain.
    while (transportActive_.load(std::memory_order_acquire) ||
           !recordQueue_.empty() || !recorderQueue_.empty() || !txQueue_.empty())
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        delay(1);
#else
        std::this_thread::yield();
#endif
    }
    stopRequested_.store(false, std::memory_order_release);
}

void TransportWorker::shutdown()
{
    // Flush any in-flight builder payload into TX queue.
    flush();
    stop();
}

bool TransportWorker::enqueueRecorderPayload(const void* data, std::size_t len)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire) ||
        data == nullptr || len == 0 || len > RECORDER_CHUNK_MAX)
    {
        return false;
    }
    RecorderChunk chunk{};
    chunk.len = len;
    std::memcpy(chunk.bytes, data, len);
    return recorderQueue_.push(chunk);
}

bool TransportWorker::enqueueTxPacket(const sf::ble::transport::TxPacket& packet)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire))
    {
        return false;
    }
    return txQueue_.push(packet);
}

/**
 * @brief Enqueue a single IMU record from producer context.
 * @return false if uninitialized, stopped, or queue full.
 */
bool TransportWorker::enqueueImuRecord(const HighRateImuRecord& record)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire))
    {
        return false;
    }

    if (!recordQueue_.push(record))
    {
        droppedProducerRecords_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

/**
 * @brief Thread entry thunk to call the instance transport loop.
 */
void TransportWorker::transportLoopThunk(void* param)
{
    TransportWorker* self = static_cast<TransportWorker*>(param);
    if (self)
    {
        self->transportLoop();
    }
}

/**
 * @brief Consumer loop that drains queued IMU records and sends via BLE.
 */
void TransportWorker::transportLoop()
{
    transportActive_.store(true, std::memory_order_release);
    while (true)
    {
        if (running_.load(std::memory_order_acquire) ||
            stopRequested_.load(std::memory_order_acquire) ||
            !recordQueue_.empty() || !recorderQueue_.empty() || !txQueue_.empty())
        {
            serviceOnce();
        }
        if (!running_.load(std::memory_order_acquire) &&
            stopRequested_.load(std::memory_order_acquire) &&
            recordQueue_.empty() && recorderQueue_.empty() && txQueue_.empty())
        {
            break;
        }
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        delay(1);
#else
        std::this_thread::yield();
#endif
    }
    transportActive_.store(false, std::memory_order_release);
}

/**
 * @brief Single transport iteration: pop records, build packets, notify BLE.
 */
void TransportWorker::serviceOnce()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    HighRateImuRecord record;
    while (recordQueue_.pop(record))
    {
#if ENABLE_RECORD_SINK
        if (pSystemDesc && pSystemDesc->pRecorder)
        {
            if (pSystemDesc->pRecorder->putBytes(&record, sizeof(record)) != 0)
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
        }
#endif
        if (!packetBuilder_.canAppend(sizeof(record)))
        {
            sf::ble::transport::TxPacket packet;
            if (packetBuilder_.finalize(packet))
            {
                if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                {
                    notifyFailures_.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }

        if (!packetBuilder_.appendEnsemble(&record, sizeof(record)))
        {
            droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
        }

        if (packetBuilder_.remainingPayload() == 0)
        {
            sf::ble::transport::TxPacket packet;
            if (packetBuilder_.finalize(packet))
            {
                if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                {
                    notifyFailures_.fetch_add(1, std::memory_order_relaxed);
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
            if (pSystemDesc->pRecorder->putBytes(chunk.bytes, chunk.len) != 0)
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    // Drain any TxPackets generated by high-rate or low-rate producers.
    sf::ble::transport::TxPacket packet;
    while (txQueue_.pop(packet))
    {
        if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
        {
            notifyFailures_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

/**
 * @brief Flush any buffered payload immediately to BLE.
 */
void TransportWorker::flush()
{
    sf::ble::transport::TxPacket packet;
    if (packetBuilder_.finalize(packet))
    {
        if (!txQueue_.push(packet))
        {
            droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}
