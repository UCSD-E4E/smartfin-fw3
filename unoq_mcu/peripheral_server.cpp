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

#include "pins.hpp"
#include "transport.hpp"

#include "../ipc/hal_rpc_protocol.h"

#include <cstddef>
#include <cstdint>

namespace sf_mcu
{

namespace
{

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
int8_t handle_gpio_read(const uint8_t *params_bytes, std::size_t params_len,
                        uint8_t *result_bytes, std::size_t result_capacity,
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
int8_t handle_i2c_read(const uint8_t *params_bytes, std::size_t params_len,
                       uint8_t *result_bytes, std::size_t result_capacity,
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
int8_t handle_get_reset_reason(uint8_t *result_bytes, std::size_t result_capacity,
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
int8_t handle_get_fw_version(uint8_t *result_bytes, std::size_t result_capacity,
                             std::size_t &result_len)
{
    // TODO(unoq_mcu): encode this firmware's version string as the result.
    result_len = 0;
    return SF_RPC_STATUS_ERR_GENERIC;
}

} // namespace

void peripheral_server_init()
{
    // TODO(unoq_mcu): put GPIO and I2C peripherals into a known idle state
    // and arm the sensor sampling timer (buffered/polled: the timer ISR
    // samples temp/water/IMU on schedule into a ring buffer; nothing here
    // pushes samples over the transport).
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
