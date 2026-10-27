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
#include "i2c.h"
#include "pins.hpp"
#include "stm32u5xx_hal.h"
#include "transport.hpp"
#include "imu.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace sf_mcu
{

    namespace
    {

        /**
         * @brief One buffered sensor sample.
         *
         * Aliased to the shared SF_RPC_IMUSampleRecord wire format defined in
         * ipc/hal_rpc_protocol.h.
         */
        using Sample = SF_RPC_IMUSampleRecord;

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
         * @brief Timeout for one I2C transfer, in ms.
         *
         * Linux reads the response ~1.5 ms after sending the request, so a
         * transfer that runs this long misses that window. A missing device is
         * still reported in time: it NACKs its address within microseconds.
         */
        constexpr uint32_t kI2cTimeoutMs = 10;

        /**
         * @brief Map a HAL I2C result to an @c SF_RPC_Status.
         *
         * This HAL reports a timeout as HAL_ERROR with HAL_I2C_ERROR_TIMEOUT
         * set, not as HAL_TIMEOUT, so the error code decides.
         */
        int8_t i2c_status(HAL_StatusTypeDef st)
        {
            if (st == HAL_OK)
            {
                return SF_RPC_STATUS_OK;
            }
            if ((HAL_I2C_GetError(&hi2c3) & HAL_I2C_ERROR_TIMEOUT) != 0U)
            {
                return SF_RPC_STATUS_ERR_TIMEOUT;
            }
            return SF_RPC_STATUS_ERR_GENERIC;   // NACK (no device), bus error, ...
        }

        /**
         * @brief Decode a msgpack unsigned int that fits in 8 bits.
         *
         * @param p     Bytes to decode.
         * @param n     Bytes available at @p p.
         * @param value Set to the decoded value.
         * @return Bytes consumed, or 0 if @p p is not a positive fixint or uint8.
         */
        std::size_t decode_uint8(const uint8_t *p, std::size_t n, uint8_t &value)
        {
            if (n >= 1 && p[0] <= 0x7F)
            {
                value = p[0];                       // positive fixint
                return 1;
            }
            if (n >= 2 && p[0] == 0xCC)
            {
                value = p[1];                       // uint8
                return 2;
            }
            return 0;
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
            // Result layout: 92 <status> C4 <len> <data...>
            constexpr std::size_t kHeaderLen = 4;

            // Always leave a well-formed [status, data] result, even on failure.
            auto fail = [&](int8_t status) -> int8_t
            {
                result_len = 0;
                if (result_capacity >= kHeaderLen)
                {
                    result_bytes[0] = 0x92;                 // array of 2
                    result_bytes[1] = (uint8_t)status;      // negative fixint
                    result_bytes[2] = 0xC4;                 // bin8
                    result_bytes[3] = 0;                    // no data
                    result_len = kHeaderLen;
                }
                return status;
            };

            // Params: 93 <address> <length> <repeated>, e.g. 93 48 02 C2
            std::size_t i = 0;
            if (params_len < 1 || params_bytes[i++] != 0x93)
            {
                return fail(SF_RPC_STATUS_ERR_GENERIC);
            }

            uint8_t addr = 0;
            std::size_t used = decode_uint8(&params_bytes[i], params_len - i, addr);
            if (used == 0)
            {
                return fail(SF_RPC_STATUS_ERR_GENERIC);
            }
            i += used;

            uint8_t len = 0;
            used = decode_uint8(&params_bytes[i], params_len - i, len);
            if (used == 0)
            {
                return fail(SF_RPC_STATUS_ERR_GENERIC);
            }
            i += used;

            // repeated: STOP is always sent; the TMP117 keeps its register
            // pointer across a STOP, so a repeated START is not needed.
            if (i >= params_len || (params_bytes[i] != 0xC2 && params_bytes[i] != 0xC3))
            {
                return fail(SF_RPC_STATUS_ERR_GENERIC);
            }

            if (len == 0 || kHeaderLen + len > result_capacity)
            {
                return fail(SF_RPC_STATUS_ERR_GENERIC);
            }

            // Receive straight into the data slot of the result.
            const int8_t status = i2c_status(HAL_I2C_Master_Receive(
                &hi2c3, (uint16_t)(addr << 1), &result_bytes[kHeaderLen], len, kI2cTimeoutMs));
            if (status != SF_RPC_STATUS_OK)
            {
                return fail(status);
            }

            result_bytes[0] = 0x92;                         // array of 2
            result_bytes[1] = (uint8_t)SF_RPC_STATUS_OK;    // 00
            result_bytes[2] = 0xC4;                         // bin8
            result_bytes[3] = len;                          // e.g. 02
            result_len = kHeaderLen + len;                  // 92 00 C4 02 MSB LSB
            return SF_RPC_STATUS_OK;
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
            // Params: 93 <address> <repeated> C4 <len> <data...>, e.g. 93 48 C3 C4 01 00
            std::size_t i = 0;
            if (params_len < 1 || params_bytes[i++] != 0x93) return SF_RPC_STATUS_ERR_GENERIC;

            uint8_t addr = 0;
            const std::size_t used = decode_uint8(&params_bytes[i], params_len - i, addr);
            if (used == 0) return SF_RPC_STATUS_ERR_GENERIC;
            i += used;

            // repeated (C3) asks to keep the bus for a following read instead of
            // sending STOP. Each RPC is a separate SPI exchange, so honouring it
            // would mean holding SCL low between them for milliseconds. The
            // TMP117 keeps its register pointer across a STOP, so a STOP is
            // always sent and the next read still returns the selected register.
            if (i >= params_len || (params_bytes[i] != 0xC2 && params_bytes[i] != 0xC3))
                return SF_RPC_STATUS_ERR_GENERIC;
            i++;

            if (i + 2 > params_len || params_bytes[i] != 0xC4) return SF_RPC_STATUS_ERR_GENERIC;
            const uint8_t len = params_bytes[i + 1];
            const uint8_t *data = &params_bytes[i + 2];
            if (i + 2 + len > params_len) return SF_RPC_STATUS_ERR_GENERIC;

            return i2c_status(HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(addr << 1),
                                                      const_cast<uint8_t *>(data), len,
                                                      kI2cTimeoutMs));
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
            // Drains buffered SF_RPC_IMUSampleRecord entries from g_sample_ring
            // up to max_samples, encodes the [status, count, bin_data] MessagePack
            // response, and advances g_sample_tail.
            int8_t status = SF_RPC_STATUS_OK;
            if (g_sample_overrun)
            {
                status = SF_RPC_STATUS_ERR_OVERRUN;
                g_sample_overrun = false; // Reported once
            }

            // Decode [max_samples] parameter
            uint32_t max_samples = 1;
            if (params_bytes != nullptr && params_len > 0)
            {
                if (params_len >= 2 && (params_bytes[0] & 0xF0) == 0x90)
                {
                    max_samples = params_bytes[1] & 0x7F;
                }
                else if ((params_bytes[0] & 0x80) == 0x00)
                {
                    max_samples = params_bytes[0];
                }
            }
            if (max_samples == 0)
            {
                max_samples = 1;
            }

            const std::size_t available = sample_ring_count();
            const std::size_t count = (available < max_samples) ? available : max_samples;
            const std::size_t data_bytes = count * sizeof(Sample);

            // Ensure destination has enough space for MsgPack array header + data
            if (data_bytes + 10 > result_capacity)
            {
                result_len = 0;
                return SF_RPC_STATUS_ERR_GENERIC;
            }

            uint8_t *dest = result_bytes;

            // MsgPack fixarray of 3 elements: [status, count, bin_data]
            *dest++ = 0x93;

            // 1. Status (int8)
            if (status >= 0)
            {
                *dest++ = static_cast<uint8_t>(status);
            }
            else
            {
                *dest++ = 0xD0; // int8 type tag
                *dest++ = static_cast<uint8_t>(status);
            }

            // 2. Count (positive fixint or uint8)
            *dest++ = static_cast<uint8_t>(count);

            // 3. Binary blob of samples: bin8 format (0xC4, length, bytes...)
            *dest++ = 0xC4;
            *dest++ = static_cast<uint8_t>(data_bytes);

            for (std::size_t i = 0; i < count; ++i)
            {
                const Sample &s = g_sample_ring[g_sample_tail];
                std::memcpy(dest, &s, sizeof(Sample));
                dest += sizeof(Sample);
                g_sample_tail = (g_sample_tail + 1) % kSampleRingCapacity;
            }

            result_len = dest - result_bytes;
            return status;
        }

    } // namespace

    /**
     * @brief Send a msgpack-rpc response [1, msgid, error, result] to Linux.
     *
     * @param msgid      The id from the request being answered.
     * @param result     Msgpack-encoded result; ignored when @p error is set.
     * @param result_len Length of @p result in bytes.
     * @param error      nullptr on success. Otherwise a short protocol error
     *                   message (unknown method, malformed request); it goes
     *                   in the error slot and the result is nil. Hardware
     *                   failures are not errors here: they go in the result's
     *                   status field.
     */
    void send_response(uint32_t msgid, const uint8_t *result, std::size_t result_len,
                       const char *error = nullptr)
    {
        uint8_t tx[80];
        // Largest header: array + type + uint32 msgid (5) + fixstr error (32).
        constexpr std::size_t kMaxErrorLen = 31;
        std::size_t i = 0;
        tx[i++] = 0x94;              // array of 4
        tx[i++] = 0x01;              // type 1 = response
        // same id Linux sent, in the smallest msgpack uint form that fits
        if (msgid <= 0x7F)
        {
            tx[i++] = (uint8_t)msgid;                // positive fixint
        }
        else if (msgid <= 0xFF)
        {
            tx[i++] = 0xCC;                          // uint8
            tx[i++] = (uint8_t)msgid;
        }
        else if (msgid <= 0xFFFF)
        {
            tx[i++] = 0xCD;                          // uint16, big-endian
            tx[i++] = (uint8_t)(msgid >> 8);
            tx[i++] = (uint8_t)msgid;
        }
        else
        {
            tx[i++] = 0xCE;                          // uint32, big-endian
            tx[i++] = (uint8_t)(msgid >> 24);
            tx[i++] = (uint8_t)(msgid >> 16);
            tx[i++] = (uint8_t)(msgid >> 8);
            tx[i++] = (uint8_t)msgid;
        }
        if (error == nullptr && i + 1 + result_len > sizeof(tx))
        {
            error = "result too large";
        }

        if (error == nullptr)
        {
            tx[i++] = 0xC0;              // error = nil
            memcpy(&tx[i], result, result_len);   // e.g. 91 00
            i += result_len;
        }
        else
        {
            std::size_t error_len = strlen(error);
            if (error_len > kMaxErrorLen)
            {
                error_len = kMaxErrorLen;
            }
            tx[i++] = (uint8_t)(0xA0 | error_len);   // fixstr
            memcpy(&tx[i], error, error_len);
            i += error_len;
            tx[i++] = 0xC0;              // result = nil
        }
        transport_send(tx, i);
    }

    void peripheral_server_init()
    {
        g_sample_head = 0;
        g_sample_tail = 0;
        g_sample_overrun = false;

        // TODO(unoq_mcu): put GPIO and I2C peripherals into a known idle state.
        // The sampling timer itself is armed in main() via HAL_TIM_Base_Start_IT(&htim2).
        // Initialize the ICM-20948 IMU over I2C4
        imu_init();
    }

    void peripheral_server_run()
    {
        // TODO(unoq_mcu): read one msgpack-rpc message via
        // sf_mcu::transport_recv(), decode its message type and method name,
        // dispatch to the matching handle_* function above, and, if the
        // message was a request (not a notification), encode and send the
        // response via sf_mcu::transport_send().
        uint8_t rx[256];
        std::size_t n = transport_recv(rx, sizeof(rx));   // whole message from SPI
        if (n == 0) return;

        // Without a msgid there is nothing to answer to, so a message that
        // breaks before the msgid is dropped; Linux's rpc_call() then fails on
        // the empty read. Notifications (type 2) are not answered either.
        std::size_t i = 0;
        if (n < 3) return;
        if(rx[i++] != 0x94) return;
        if(rx[i++] != 0x00) return;

        uint32_t msgid;
        if      (rx[i] <= 0x7F) { msgid = rx[i];i += 1; }
        else if (rx[i] == 0xCC && i + 2 <= n) { msgid = rx[i+1]; i += 2; }
        else if (rx[i] == 0xCD && i + 3 <= n) { msgid = (rx[i+1] << 8) | rx[i+2]; i += 3; }
        else if (rx[i] == 0xCE && i + 5 <= n) { msgid = ((uint32_t)rx[i+1] << 24) | (rx[i+2] << 16) | (rx[i+3] << 8) | rx[i+4];                      i += 5; }
        else return;

        // From here on every request gets a well-formed response.
        if (i >= n || (rx[i] & 0xE0) != 0xA0)
        {
            send_response(msgid, nullptr, 0, "malformed request");
            return;
        }
        std::size_t name_len = rx[i] & 0x1F;
        const char *name = (const char *)&rx[i + 1];
        i += 1 + name_len;
        if (i > n)
        {
            send_response(msgid, nullptr, 0, "malformed request");
            return;
        }

        const uint8_t *params = &rx[i];
        std::size_t params_len = n - i;

        uint8_t result[64];
        std::size_t result_len = 0;

        if (name_len == 9 && memcmp(name, "i2c_write", 9) == 0) {
            int8_t st = handle_i2c_write(params, params_len);
            result[0] = 0x91; 
            result[1] = (uint8_t)st;
            result_len = 2;
        }
        else if (name_len == 8 && memcmp(name, "i2c_read", 8) == 0) {
            handle_i2c_read(params, params_len, result, sizeof(result), result_len);
        }
        else {
            send_response(msgid, nullptr, 0, "unknown method");
            return;
        }

        send_response(msgid, result, result_len);
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
    // TODO(unoq_mcu): read temp and water sensors into the sample if needed.
    if (sf_mcu::imu_read_sample(sample))
    {
        sf_mcu::sample_ring_push(sample);
    }
}

#endif // HAL_TIM_MODULE_ENABLED
