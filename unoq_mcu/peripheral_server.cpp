/**
 * @file peripheral_server.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Implementation of the msgpack-rpc dispatcher for the STM32U585
 *        peripheral server.
 * @date 2026-07-23
 *
 * Handler signatures take and return raw msgpack-encoded bytes rather than
 * a specific MessagePack library's object type, so this file does not
 * commit to a MessagePack implementation choice (e.g. CMP) at the
 * declaration level; that choice is made inside each handler body later.
 */
#include "peripheral_server.hpp"

#include "../ipc/hal_rpc_protocol.h"
#include "pins.hpp"
#include "stm32u5xx_hal.h"
#include "transport.hpp"

#include <cstddef>
#include <cstdint>

namespace sf_mcu
{

    namespace
    {

        /**
         * @brief One buffered sensor sample.
         *
         * @note The sensor fields are deliberately absent. Which readings belong
         *       here depends on whether the IMU hangs off the STM32 or the
         *       QRB2210, which is still an open question, so only the timestamp
         *       (which every layout needs) is fixed. The ring machinery below does
         *       not care about the payload and will not need revisiting when the
         *       fields are added.
         */
        struct Sample
        {
            uint32_t timestamp_ms; ///< HAL tick at the instant of capture.
        };

        /**
         * @brief Depth of the sample ring in samples.
         *
         * At the 55 Hz sample rate this is roughly 4.6 seconds of slack before
         * Linux must drain, which is the margin that keeps a scheduling hiccup on
         * the Linux side from costing samples. SRAM is not scarce on this part, so
         * the depth is chosen for margin rather than economy.
         */
        constexpr std::size_t kSampleRingCapacity = 256;

        Sample g_sample_ring[kSampleRingCapacity];

        /**
         * @brief Producer index, written only by the sampling ISR.
         */
        volatile std::size_t g_sample_head = 0;

        /**
         * @brief Consumer index, written only by the RPC dispatch loop.
         */
        volatile std::size_t g_sample_tail = 0;

        /**
         * @brief Set when the ISR overwrites an undrained sample.
         *
         * Cleared when reported, so each overrun is reported to Linux exactly
         * once via @c SF_RPC_STATUS_ERR_OVERRUN.
         */
        volatile bool g_sample_overrun = false;

        /**
         * @brief Append one sample, dropping the oldest if the ring is full.
         *
         * Called from the sampling ISR only. Single producer, single consumer: the
         * ISR touches only @c g_sample_head, the dispatch loop only
         * @c g_sample_tail, so no critical section is needed on either side.
         *
         * @param sample The sample to store.
         */
        void sample_ring_push(const Sample &sample)
        {
            const std::size_t head = g_sample_head;
            const std::size_t next = (head + 1) % kSampleRingCapacity;

            if (next == g_sample_tail)
            {
                // Full. Drop the oldest so the newest data always survives, and
                // flag the loss rather than failing silently.
                g_sample_tail = (g_sample_tail + 1) % kSampleRingCapacity;
                g_sample_overrun = true;
            }

            g_sample_ring[head] = sample;
            g_sample_head = next;
        }

        /**
         * @brief Number of samples currently buffered.
         *
         * @return Count of samples available to drain.
         */
        std::size_t sample_ring_count()
        {
            const std::size_t head = g_sample_head;
            const std::size_t tail = g_sample_tail;
            return (head + kSampleRingCapacity - tail) % kSampleRingCapacity;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_GPIO_SET_MODE call.
         *
         * @param params_bytes Msgpack-encoded [pin_id, mode] params array.
         * @param params_len   Length of @p params_bytes in bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_gpio_set_mode(const uint8_t *params_bytes, std::size_t params_len)
        {
            // TODO(unoq_mcu): decode [pin_id, mode], resolve the pin via pins.hpp,
            // and configure the real STM32U585 GPIO peripheral.
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_GPIO_WRITE call.
         *
         * @param params_bytes Msgpack-encoded [pin_id, state] params array.
         * @param params_len   Length of @p params_bytes in bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_gpio_write(const uint8_t *params_bytes, std::size_t params_len)
        {
            // TODO(unoq_mcu): decode [pin_id, state] and drive the resolved pin.
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_GPIO_READ call.
         *
         * @param params_bytes    Msgpack-encoded [pin_id] params array.
         * @param params_len      Length of @p params_bytes in bytes.
         * @param result_bytes    Destination buffer for the msgpack-encoded result.
         * @param result_capacity Capacity of @p result_bytes in bytes.
         * @param result_len      Set to the number of bytes written to @p result_bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_gpio_read(const uint8_t *params_bytes,
                                std::size_t params_len,
                                uint8_t *result_bytes,
                                std::size_t result_capacity,
                                std::size_t &result_len)
        {
            // TODO(unoq_mcu): decode [pin_id], sample the resolved pin, and encode
            // the state as the result.
            result_len = 0;
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_I2C_READ call.
         *
         * @param params_bytes    Msgpack-encoded [address, length, repeated] params array.
         * @param params_len      Length of @p params_bytes in bytes.
         * @param result_bytes    Destination buffer for the msgpack-encoded
         *                        [status, data] result.
         * @param result_capacity Capacity of @p result_bytes in bytes.
         * @param result_len      Set to the number of bytes written to @p result_bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_i2c_read(const uint8_t *params_bytes,
                               std::size_t params_len,
                               uint8_t *result_bytes,
                               std::size_t result_capacity,
                               std::size_t &result_len)
        {
            // TODO(unoq_mcu): decode [address, length, repeated], perform the I2C
            // read transaction, and encode [status, data] as the result.
            result_len = 0;
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_I2C_WRITE call.
         *
         * @param params_bytes Msgpack-encoded [address, repeated, data] params array.
         * @param params_len   Length of @p params_bytes in bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_i2c_write(const uint8_t *params_bytes, std::size_t params_len)
        {
            // TODO(unoq_mcu): decode [address, repeated, data] and perform the I2C
            // write transaction.
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_RESET notification.
         *
         * Sent as a notification, not a request: no response is produced. Resets
         * this MCU; the Linux side resets itself independently on the same call.
         */
        void handle_reset()
        {
            // TODO(unoq_mcu): perform an immediate MCU reset.
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_GET_RESET_REASON call.
         *
         * @param result_bytes    Destination buffer for the msgpack-encoded
         *                        [reason, reason_data] result.
         * @param result_capacity Capacity of @p result_bytes in bytes.
         * @param result_len      Set to the number of bytes written to @p result_bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_get_reset_reason(uint8_t *result_bytes,
                                       std::size_t result_capacity,
                                       std::size_t &result_len)
        {
            // TODO(unoq_mcu): encode [reason, reason_data] for this MCU's last reset.
            result_len = 0;
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_GET_FW_VERSION call.
         *
         * @param result_bytes    Destination buffer for the msgpack-encoded version
         *                        string result.
         * @param result_capacity Capacity of @p result_bytes in bytes.
         * @param result_len      Set to the number of bytes written to @p result_bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_get_fw_version(uint8_t *result_bytes,
                                     std::size_t result_capacity,
                                     std::size_t &result_len)
        {
            // TODO(unoq_mcu): encode this firmware's version string as the result.
            result_len = 0;
            return SF_RPC_STATUS_ERR_GENERIC;
        }

        /**
         * @brief Handle a @c SF_RPC_METHOD_READ_SAMPLES call.
         *
         * Drains up to @c max_samples buffered samples, oldest first. Draining is
         * destructive. Reports @c SF_RPC_STATUS_ERR_OVERRUN once per overrun.
         *
         * @param params_bytes    Msgpack-encoded [max_samples] params array.
         * @param params_len      Length of @p params_bytes in bytes.
         * @param result_bytes    Destination buffer for the msgpack-encoded
         *                        [status, count, data] result.
         * @param result_capacity Capacity of @p result_bytes in bytes.
         * @param result_len      Set to the number of bytes written to @p result_bytes.
         * @return Status per @c SF_RPC_Status.
         */
        int8_t handle_read_samples(const uint8_t *params_bytes,
                                   std::size_t params_len,
                                   uint8_t *result_bytes,
                                   std::size_t result_capacity,
                                   std::size_t &result_len)
        {
            // TODO(unoq_mcu): decode [max_samples], then drain that many entries
            // from the ring (advancing g_sample_tail) and encode [status, count,
            // data]. Blocked on the MCU-side MessagePack library choice, same as
            // every other handler here, and on the per-sample layout, which is
            // blocked on the IMU placement question.
            (void)sample_ring_count();
            result_len = 0;
            return SF_RPC_STATUS_ERR_GENERIC;
        }

    } // namespace

    void peripheral_server_init()
    {
        g_sample_head = 0;
        g_sample_tail = 0;
        g_sample_overrun = false;

        // TODO(unoq_mcu): put GPIO and I2C peripherals into a known idle state.
        // The sampling timer itself is armed in main() once CubeMX generates
        // TIM2, via HAL_TIM_Base_Start_IT(&htim2).
    }

    void peripheral_server_run()
    {
        // TODO(unoq_mcu): read one msgpack-rpc message via
        // sf_mcu::transport_recv(), decode its message type and method name,
        // dispatch to the matching handle_* function above, and, if the
        // message was a request (not a notification), encode and send the
        // response via sf_mcu::transport_send().
    }

} // namespace sf_mcu

#ifdef HAL_TIM_MODULE_ENABLED

/**
 * @brief Hardware-timer tick: capture one sample.
 *
 * Overrides the weak definition in the STM32 HAL, so it must keep C
 * linkage and sit at global scope. TIM2 owns sample-rate timing at 55 Hz
 * independently of RPC traffic, which is what keeps capture instants off
 * Linux's scheduler.
 *
 * Guarded because the HAL's timer module is only compiled in once TIM2
 * exists in the STM32CubeMX project. The guard disappears on its own when
 * TIM2 is added; nothing here needs revisiting.
 *
 * @param htim Handle of the timer that elapsed.
 */
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2)
    {
        return;
    }

    sf_mcu::Sample sample;
    sample.timestamp_ms = HAL_GetTick();

    // TODO(unoq_mcu): read temp and water sensors (and the IMU, if it ends
    // up on this processor) into the sample before storing it.

    sf_mcu::sample_ring_push(sample);
}

#endif // HAL_TIM_MODULE_ENABLED
