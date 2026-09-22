/**
 * @file hal_rpc_protocol.h
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief MessagePack-RPC method and message-type definitions shared between
 *        the QRB2210 Linux HAL client and the STM32U585 peripheral-server
 *        firmware.
 * @date 2026-07-23
 *
 * MessagePack-RPC was adopted rather than a bespoke frame format so that
 * both sides speak a documented protocol instead of one invented for this
 * project. Note this is our own choice, not compatibility with anything on
 * the board: Arduino's "Bridge" link runs over LPUART1 on the Uno Q and we
 * replace the stock MCU firmware entirely, so both ends of this link are
 * ours. (An earlier version of this comment asserted Bridge itself is
 * MessagePack-RPC over SPI. That is unverified and, either way, nothing
 * here depends on it.) It fixes only the method names and
 * message shape both sides must agree on; encoding/decoding is left to
 * whichever MessagePack library each side links (e.g. CMP on the bare-metal
 * MCU, msgpack-cxx on Linux), so this header contains no MessagePack
 * library types itself.
 */
#ifndef SF_HAL_RPC_PROTOCOL_H
#define SF_HAL_RPC_PROTOCOL_H

#include <stdint.h>

/**
 * @brief Suggested maximum size in bytes of a single encoded msgpack-rpc
 *        message.
 *
 * Used to size fixed receive/send buffers on the bare-metal MCU, which has
 * no dynamic allocation.
 */
#define SF_RPC_MAX_MESSAGE_LEN 128

/**
 * @brief Size in bytes of one SPI frame on the wire.
 *
 * Every SPI transaction exchanges exactly this many bytes in each
 * direction. Messages shorter than a frame are padded; see
 * @c SF_RPC_FRAME_EMPTY.
 */
#define SF_RPC_FRAME_LEN SF_RPC_MAX_MESSAGE_LEN

/**
 * @brief First byte of a frame carrying no message.
 *
 * This is MessagePack @c nil. Every real message is an array, whose first
 * byte is a fixarray header (0x93 or 0x94), so @c nil is unambiguous as an
 * "empty frame" marker and needs no out-of-band signalling. Used to pad
 * the master's poll frames and the MCU's frames when no response is ready.
 */
#define SF_RPC_FRAME_EMPTY 0xC0

/**
 * @brief Responses are deferred by one transaction.
 *
 * SPI slaves cannot initiate: the QRB2210 master clocks every bit, so the
 * MCU has no way to signal "my answer is ready". Rather than spend a GPIO
 * on a ready line (which on this board would mean routing an MCU pin out
 * to JMISC and level-shifting 3.3 V down to the MPU's 1.8 V, so a carrier
 * respin), the link uses split transactions:
 *
 *   transaction N:   master sends a request frame, MCU returns whatever
 *                    response it had ready, or @c SF_RPC_FRAME_EMPTY.
 *   transaction N+1: master sends the next request (or an empty poll
 *                    frame), MCU returns the response to request N.
 *
 * Every exchange is therefore exactly two transactions, and responses ride
 * along on traffic the master was sending anyway, such as the regular
 * sample-buffer drain. Notifications (@c SF_RPC_MSG_NOTIFICATION) produce
 * no response and so occupy only one transaction.
 *
 * A ready GPIO remains a possible later optimisation for latency; it would
 * not change the message shapes defined here.
 */

/**
 * @brief msgpack-rpc message type, the first element of every message array.
 *
 * Matches the msgpack-rpc specification: a request expects a matching
 * response identified by @c msgid; a notification does not.
 */
enum SF_RPC_MessageType
{
    SF_RPC_MSG_REQUEST = 0,      ///< [type, msgid, method, params]; caller awaits a response.
    SF_RPC_MSG_RESPONSE = 1,     ///< [type, msgid, error, result]; sent in reply to a request.
    SF_RPC_MSG_NOTIFICATION = 2, ///< [type, method, params]; no response is sent.
};

/**
 * @brief Method name for @c SF_HAL::gpio_set_mode().
 *
 * Params: [pin_id, mode]. Result: nil.
 */
#define SF_RPC_METHOD_GPIO_SET_MODE "gpio_set_mode"

/**
 * @brief Method name for @c SF_HAL::gpio_write().
 *
 * Params: [pin_id, state]. Result: nil.
 */
#define SF_RPC_METHOD_GPIO_WRITE "gpio_write"

/**
 * @brief Method name for @c SF_HAL::gpio_read().
 *
 * Params: [pin_id]. Result: state (bool).
 */
#define SF_RPC_METHOD_GPIO_READ "gpio_read"

/**
 * @brief Method name for @c SF_HAL::i2c_read().
 *
 * Params: [address, length, repeated]. Result: [status, data (bin)].
 */
#define SF_RPC_METHOD_I2C_READ "i2c_read"

/**
 * @brief Method name for @c SF_HAL::i2c_write().
 *
 * Params: [address, repeated, data (bin)]. Result: status (int).
 */
#define SF_RPC_METHOD_I2C_WRITE "i2c_write"

/**
 * @brief Method name for a coordinated reset of both processors.
 *
 * Sent as a @c SF_RPC_MSG_NOTIFICATION, not a request: the MCU resets
 * itself immediately rather than replying, since a request that waited for
 * a response could race the reset it is asking for.
 *
 * Params: none.
 */
#define SF_RPC_METHOD_RESET "reset"

/**
 * @brief Method name to query the MCU's last reset cause.
 *
 * Params: none. Result: [reason, reason_data], mirroring
 * @c SF_HAL::reset_reason() / @c SF_HAL::reset_reason_data().
 */
#define SF_RPC_METHOD_GET_RESET_REASON "get_reset_reason"

/**
 * @brief Method name to query the MCU firmware's version string.
 *
 * Params: none. Result: version string.
 */
#define SF_RPC_METHOD_GET_FW_VERSION "get_fw_version"

/**
 * @brief Method name to drain buffered sensor samples from the MCU.
 *
 * The MCU samples on its own hardware-timer schedule into a ring buffer
 * and never pushes; Linux drains the buffer by calling this. That keeps
 * capture timing off Linux's scheduler, which is the whole point of the
 * buffered/polled design.
 *
 * Params: [max_samples]. Result: [status, count, data (bin)], where
 * @c count samples are packed contiguously in @c data, oldest first, and
 * @c count may be less than @c max_samples (including zero) when fewer are
 * buffered. Draining is destructive: returned samples are dropped from the
 * ring.
 *
 * @c status is @c SF_RPC_STATUS_ERR_OVERRUN if the ring overflowed since
 * the previous drain, meaning samples were lost; the samples returned are
 * still valid.
 *
 * @note Reflects the decision to place the IMU on the STM32 MCU.
 *       The per-sample byte layout is fixed by @c SF_RPC_IMUSampleRecord below.
 */
#define SF_RPC_METHOD_READ_SAMPLES "read_samples"

/**
 * @brief Binary layout of one high-rate IMU sample record packed inside
 *        @c SF_RPC_METHOD_READ_SAMPLES responses.
 *
 * Packed format (36 bytes) matching Smartfin Ensemble13 (High-Rate IMU + Quat9).
 */
#pragma pack(push, 1)
struct SF_RPC_IMUSampleRecord
{
    uint32_t timestamp_ms;              ///< HAL tick at the instant of capture (ms).
    int16_t  accel_x, accel_y, accel_z; ///< Raw Accelerometer (x, y, z).
    int16_t  gyro_x,  gyro_y,  gyro_z;  ///< Raw Gyroscope (x, y, z).
    int16_t  mag_x,   mag_y,   mag_z;   ///< Raw Magnetometer (x, y, z).
    int32_t  quat9_1, quat9_2, quat9_3; ///< 9-axis DMP Quaternion (Q30).
    int16_t  quat9_accuracy;            ///< DMP accuracy estimate (Q12 rad).
};
#pragma pack(pop)

/**
 * @brief Application-level result status, distinct from the msgpack-rpc
 *        protocol-level error slot.
 *
 * Carried as an element of a method's result (e.g. @c SF_RPC_METHOD_I2C_READ)
 * for operations that can fail at the hardware level. The msgpack-rpc
 * @c error slot itself is reserved for protocol failures (unknown method,
 * malformed params) and stays nil on a well-formed call that simply
 * returned a hardware error via this status.
 *
 * Mirrors the 0-success / negative-error convention used by
 * @c SF_HAL::i2c_read() and @c SF_HAL::i2c_write().
 */
enum SF_RPC_Status
{
    SF_RPC_STATUS_OK = 0,           ///< Request completed successfully.
    SF_RPC_STATUS_ERR_GENERIC = -1, ///< Unspecified failure.
    SF_RPC_STATUS_ERR_TIMEOUT = -2, ///< Peripheral did not respond in time.
    SF_RPC_STATUS_ERR_OVERRUN = -3, ///< Sample ring overflowed; samples were lost.
};

#endif // SF_HAL_RPC_PROTOCOL_H
