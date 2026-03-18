/**
 * @file high_rate_stream.cpp
 * @brief Unified transport worker for all BLE telemetry and recorder IO.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "high_rate_stream.hpp"

#include "ble/sf_ble.hpp"
#include "cli/flog.hpp"
#include "product.hpp"
#include "system.hpp"
#include <cstring>
#include <thread>

/**
 * @brief Construct the TransportService singleton (clears counters/queues).
 */
TransportService::TransportService() :
    initialized_(false),
    running_(false),
    stopRequested_(false),
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    transportThread_(nullptr),
#endif
    transportActive_(false),
    lowRateFlusher_(nullptr),
    idle_(true),
    droppedProducerRecords_(0),
    droppedTransportPackets_(0),
    notifyFailures_(0),
    recordQueue_(),
    packetBuilder_(),
    recorderQueue_(),
    txQueue_(),
    lowRateQueue_(),
    lastFlushMs_(0)
{
}

TransportService& TransportService::getInstance()
{
    static TransportService instance;
    return instance;
}

/**
 * @brief Reset the builder/queue and counters.
 * @return true always (currently cannot fail).
 */
bool TransportService::init()
{
    // Lazily create persistent transport thread once.
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    if (transportThread_ == nullptr)
    {
        transportThread_ = new Thread("transport_worker",
                                      TransportService::transportLoopThunk,
                                      this,
                                      OS_THREAD_PRIORITY_DEFAULT);
    }
#endif

    packetBuilder_.reset();
    lastFlushMs_ = millis();
    stopRequested_.store(false, std::memory_order_release);
    transportActive_.store(false, std::memory_order_release);
    idle_.store(true, std::memory_order_release);
    accepting_.store(false, std::memory_order_release);
    sf::ble::transport::TxPacket dummyPacket;
    while (txQueue_.pop(dummyPacket))
    {
    }
    RecorderChunk dummyChunk;
    while (recorderQueue_.pop(dummyChunk))
    {
    }
    LowRateChunk dummyLow{};
    while (lowRateQueue_.pop(dummyLow))
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
void TransportService::start()
{
    running_.store(true, std::memory_order_release);
    stopRequested_.store(false, std::memory_order_release);
    accepting_.store(true, std::memory_order_release);
    idle_.store(false, std::memory_order_release);
}

/**
 * @brief Stop streaming; transport loop will exit on next iteration.
 */
void TransportService::stop()
{
    stopRequested_.store(true, std::memory_order_release);
    running_.store(false, std::memory_order_release);
    accepting_.store(false, std::memory_order_release);
    // Wait for queues to drain; thread remains alive.
    while (!idle_.load(std::memory_order_acquire))
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        delay(1);
#else
        std::this_thread::yield();
#endif
    }
    stopRequested_.store(false, std::memory_order_release);
    accepting_.store(false, std::memory_order_release);
}

void TransportService::shutdown()
{
    // Close the gate before flushing so no new enqueues can sneak in after the flush.
    accepting_.store(false, std::memory_order_release);
    if (lowRateFlusher_)
    {
        lowRateFlusher_();
    }
    stop();
}

void TransportService::setLowRateFlusher(void (*flusher)())
{
    lowRateFlusher_ = flusher;
}

bool TransportService::enqueueRecorderPayload(const void* data, std::size_t len)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire) ||
        !accepting_.load(std::memory_order_acquire) ||
        data == nullptr || len == 0 || len > RECORDER_CHUNK_MAX)
    {
        return false;
    }
    RecorderChunk chunk{};
    chunk.len = len;
    std::memcpy(chunk.bytes, data, len);
    return recorderQueue_.push(chunk);
}

bool TransportService::enqueueTxPacket(const sf::ble::transport::TxPacket& packet)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire) ||
        !accepting_.load(std::memory_order_acquire))
    {
        return false;
    }
    return txQueue_.push(packet);
}

bool TransportService::enqueueLowRateEnsemble(const void* data, std::size_t len)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire) ||
        !accepting_.load(std::memory_order_acquire) ||
        data == nullptr || len == 0 || len > LOW_RATE_MAX)
    {
        return false;
    }
    LowRateChunk chunk{};
    chunk.len = len;
    std::memcpy(chunk.bytes, data, len);
    return lowRateQueue_.push(chunk);
}

/**
 * @brief Enqueue a single IMU record from producer context.
 * @return false if uninitialized, stopped, or queue full.
 */
bool TransportService::enqueueImuRecord(const HighRateImuRecord& record)
{
    if (!initialized_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire) ||
        !accepting_.load(std::memory_order_acquire))
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
void TransportService::transportLoopThunk(void* param)
{
    TransportService* self = static_cast<TransportService*>(param);
    if (self)
    {
        self->transportLoop();
    }
}

/**
 * @brief Consumer loop that drains queued IMU records and sends via BLE.
 */
void TransportService::transportLoop()
{
    transportActive_.store(true, std::memory_order_release);
    while (true)
    {
        if (running_.load(std::memory_order_acquire) ||
            stopRequested_.load(std::memory_order_acquire) ||
            !recordQueue_.empty() || !recorderQueue_.empty() || !txQueue_.empty() ||
            !lowRateQueue_.empty())
        {
            serviceOnce();
        }
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        delay(1);
#else
        std::this_thread::yield();
#endif
    }
    // Persistent thread; never exits under normal conditions.
}

/**
 * @brief Single transport iteration: pop records, build packets, notify BLE.
 */
void TransportService::serviceOnce()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    idle_.store(false, std::memory_order_release);

    bool progress = false;
    do
    {
        progress = false;

        // Drain a bounded batch of high-rate records to avoid starving other queues.
        HighRateImuRecord record;
        for (std::size_t i = 0; i < MAX_RECORD_BATCH && recordQueue_.pop(record); ++i)
        {
            progress = true;
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
                    const bool connected = SFBLE::getInstance().isConnected();
                    if (!connected)
                    {
                        droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
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
                    const bool connected = SFBLE::getInstance().isConnected();
                    if (!connected)
                    {
                        droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                    {
                        notifyFailures_.fetch_add(1, std::memory_order_relaxed);
                    }
                }
                lastFlushMs_ = millis();
            }
        }

        // Packetize at most one low-rate ensemble per round to keep fairness.
        LowRateChunk low;
        if (lowRateQueue_.pop(low))
        {
            progress = true;
            if (!packetBuilder_.canAppend(low.len))
            {
                sf::ble::transport::TxPacket packet;
                if (packetBuilder_.finalize(packet))
                {
                    const bool connected = SFBLE::getInstance().isConnected();
                    if (!connected)
                    {
                        droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                    {
                        notifyFailures_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        lastFlushMs_ = millis();
                    }
                }
            }

            if (!packetBuilder_.appendEnsemble(low.bytes, low.len))
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
            else if (packetBuilder_.remainingPayload() == 0)
            {
                sf::ble::transport::TxPacket packet;
                if (packetBuilder_.finalize(packet))
                {
                    const bool connected = SFBLE::getInstance().isConnected();
                    if (!connected)
                    {
                        droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
                    {
                        notifyFailures_.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        lastFlushMs_ = millis();
                    }
                }
            }
        }

        // Drain one recorder chunk per round.
        if (pSystemDesc && pSystemDesc->pRecorder)
        {
            RecorderChunk chunk;
            if (recorderQueue_.pop(chunk))
            {
                progress = true;
                if (pSystemDesc->pRecorder->putBytes(chunk.bytes, chunk.len) != 0)
                {
                    droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }

        // Drain one tx packet per round.
        sf::ble::transport::TxPacket packet;
        const bool connected = SFBLE::getInstance().isConnected();
        if (txQueue_.pop(packet))
        {
            progress = true;
            if (!connected)
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
            else if (!SFBLE::getInstance().notifyTelemetry(packet.bytes, packet.len))
            {
                notifyFailures_.fetch_add(1, std::memory_order_relaxed);
            }
        }
    } while (progress && (stopRequested_.load(std::memory_order_acquire) ||
                          !recordQueue_.empty() || !lowRateQueue_.empty() ||
                          !recorderQueue_.empty() || !txQueue_.empty()));

    // Periodic flush for partial payloads to keep low-rate data live.
    if (packetBuilder_.hasData() && (millis() - lastFlushMs_ >= LOW_RATE_FLUSH_INTERVAL_MS))
    {
        sf::ble::transport::TxPacket flushPacket;
        if (packetBuilder_.finalize(flushPacket))
        {
            const bool connected = SFBLE::getInstance().isConnected();
            if (!connected)
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
            else if (!SFBLE::getInstance().notifyTelemetry(flushPacket.bytes, flushPacket.len))
            {
                notifyFailures_.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                lastFlushMs_ = millis();
            }
        }
    }

    // After draining all queues, if we were asked to stop and nothing remains, flush builder.
    if (stopRequested_.load(std::memory_order_acquire) &&
        recordQueue_.empty() && recorderQueue_.empty() && txQueue_.empty() && lowRateQueue_.empty() &&
        packetBuilder_.hasData())
    {
        sf::ble::transport::TxPacket finalPacket;
        if (packetBuilder_.finalize(finalPacket))
        {
            if (!SFBLE::getInstance().isConnected() ||
                !SFBLE::getInstance().notifyTelemetry(finalPacket.bytes, finalPacket.len))
            {
                droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    if (recordQueue_.empty() && recorderQueue_.empty() && txQueue_.empty() && lowRateQueue_.empty() &&
        !packetBuilder_.hasData() && stopRequested_.load(std::memory_order_acquire))
    {
        idle_.store(true, std::memory_order_release);
    }
}

/**
 * @brief Flush any buffered payload immediately to BLE.
 */
void TransportService::flush()
{
    sf::ble::transport::TxPacket packet;
    if (packetBuilder_.finalize(packet))
    {
        if (!txQueue_.push(packet))
        {
            droppedTransportPackets_.fetch_add(1, std::memory_order_relaxed);
        }
        else
        {
            lastFlushMs_ = millis();
        }
    }
}
