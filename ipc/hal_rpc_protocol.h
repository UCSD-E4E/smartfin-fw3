/**
 * @file hal_rpc_protocol.h
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief MessagePack-RPC method and message-type definitions shared between
 *        the QRB2210 Linux HAL client and the STM32U585 peripheral-server
 *        firmware.
 * @date 2026-07-23
 *
 * Uno Q's own inter-processor link (Arduino's "Bridge") is MessagePack-RPC
 * over SPI. This header adopts that same wire-level scheme rather than a
 * bespoke frame format, so both sides speak a documented protocol instead
 * of one invented for this project. It fixes only the method names and
 * message shape both sides must agree on; encoding/decoding is left to
 * whichever MessagePack library each side links (e.g. CMP on the bare-metal
 * MCU, msgpack-cxx on Linux), so this header contains no MessagePack
 * library types itself.
 */
#ifndef SF_HAL_RPC_PROTOCOL_H
#define SF_HAL_RPC_PROTOCOL_H

/**
 * @brief Suggested maximum size in bytes of a single encoded msgpack-rpc
 *        message.
 *
 * Used to size fixed receive/send buffers on the bare-metal MCU, which has
 * no dynamic allocation.
 */
#define SF_RPC_MAX_MESSAGE_LEN 128

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
};

#endif // SF_HAL_RPC_PROTOCOL_H
