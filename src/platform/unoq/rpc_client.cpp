/**
 * @file rpc_client.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Brent
 * @brief Implementation of the shared msgpack-rpc client for the Uno Q
 *        Linux backend.
 * @date 2026-09-22
 *
 * Envelope framing (msgpack-rpc request/notification/response) is real
 * and uses msgpack-c directly. The physical SPI send/receive is mocked
 * for the WSL environment to return dummy successes safely.
 */
#include "rpc_client.hpp"
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include "ipc/hal_rpc_protocol.h"

#include <cstring>
#include <msgpack.h>

namespace sf_unoq
{

namespace
{

/**
 * @brief Mutex serialising every RPC call's use of the shared SPI wire.
 * Wrapped in a function to prevent Static Initialization Order Fiascos on shutdown.
 */
SF_HAL::Mutex& get_bus_mutex() {
    static SF_HAL::Mutex m;
    return m;
}

/**
 * @brief Next msgpack-rpc @c msgid to assign to a request.
 */
uint32_t g_next_msgid = 0;

/**
 * @brief Stores the msgid of the last request so the mock response can match it.
 */
uint32_t g_last_request_msgid = 0;

/**
 * @brief Transmit a complete, framed msgpack-rpc message over SPI.
 */
bool transport_send_raw(const uint8_t *data, std::size_t len)
{
    // Mock SPI Send: Unpack the payload just to extract the request's msgid
    msgpack_zone zone;
    msgpack_zone_init(&zone, 1024);
    msgpack_object obj;
    std::size_t off = 0;
    
    if (msgpack_unpack(reinterpret_cast<const char*>(data), len, &off, &zone, &obj) == MSGPACK_UNPACK_SUCCESS) {
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size >= 3) {
            // msgpack-rpc array format: [type, msgid, method, params]
            g_last_request_msgid = static_cast<uint32_t>(obj.via.array.ptr[1].via.u64);
        }
    }
    
    msgpack_zone_destroy(&zone);
    return true; // Pretend it was sent successfully
}

/**
 * @brief Receive one complete msgpack-rpc response message over SPI.
 */
bool transport_recv_raw(uint8_t *out, std::size_t out_capacity, std::size_t &received_len)
{
    // Mock SPI Receive: Build a valid msgpack-rpc response
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Array format: [type (response=1), msgid, error (nil), result (true)]
    msgpack_pack_array(&pk, 4);
    msgpack_pack_int(&pk, static_cast<int>(SF_RPC_MSG_RESPONSE)); 
    msgpack_pack_uint32(&pk, g_last_request_msgid); 
    msgpack_pack_nil(&pk);  
    msgpack_pack_true(&pk); // Dummy success boolean for GPIO/I2C checks

    bool success = false;
    if (sbuf.size <= out_capacity) {
        std::memcpy(out, sbuf.data, sbuf.size);
        received_len = sbuf.size;
        success = true;
    }

    msgpack_sbuffer_destroy(&sbuf);
    return success;
}

void pack_method_and_params(msgpack_packer &pk,
                            msgpack_sbuffer &sbuf,
                            const char *method,
                            const uint8_t *params_msgpack,
                            std::size_t params_len)
{
    const std::size_t method_len = std::strlen(method);
    msgpack_pack_str(&pk, method_len);
    msgpack_pack_str_body(&pk, method, method_len);
    msgpack_sbuffer_write(&sbuf, reinterpret_cast<const char *>(params_msgpack), params_len);
}

} // namespace

void rpc_client_init()
{
    // MOCK: No SPI device to open
}

bool rpc_call(const char *method,
             const uint8_t *params_msgpack,
             std::size_t params_len,
             uint8_t *result_out,
             std::size_t result_capacity,
             std::size_t &result_len)
{
    result_len = 0;
    rpc_bus_lock();

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    const uint32_t msgid = g_next_msgid++;

    msgpack_pack_array(&pk, 4);
    msgpack_pack_int(&pk, static_cast<int>(SF_RPC_MSG_REQUEST));
    msgpack_pack_uint32(&pk, msgid);
    pack_method_and_params(pk, sbuf, method, params_msgpack, params_len);

    const bool sent = transport_send_raw(reinterpret_cast<const uint8_t *>(sbuf.data), sbuf.size);
    msgpack_sbuffer_destroy(&sbuf);

    if (!sent)
    {
        rpc_bus_unlock();
        return false;
    }

    uint8_t response_buf[SF_RPC_MAX_MESSAGE_LEN];
    std::size_t response_len = 0;
    const bool received = transport_recv_raw(response_buf, sizeof(response_buf), response_len);

    if (!received)
    {
        rpc_bus_unlock();
        return false;
    }

    msgpack_zone zone;
    msgpack_zone_init(&zone, 2048);
    msgpack_object response;
    std::size_t offset = 0;
    const msgpack_unpack_return unpack_status = msgpack_unpack(
        reinterpret_cast<const char *>(response_buf), response_len, &offset, &zone, &response);

    bool success = false;

    if (MSGPACK_UNPACK_SUCCESS == unpack_status && MSGPACK_OBJECT_ARRAY == response.type &&
        4 == response.via.array.size)
    {
        const msgpack_object &type_obj = response.via.array.ptr[0];
        const msgpack_object &msgid_obj = response.via.array.ptr[1];
        const msgpack_object &error_obj = response.via.array.ptr[2];
        const msgpack_object &result_obj = response.via.array.ptr[3];

        const bool type_ok = MSGPACK_OBJECT_POSITIVE_INTEGER == type_obj.type &&
                             SF_RPC_MSG_RESPONSE == static_cast<int>(type_obj.via.u64);
        const bool msgid_ok = MSGPACK_OBJECT_POSITIVE_INTEGER == msgid_obj.type &&
                              msgid == static_cast<uint32_t>(msgid_obj.via.u64);
        const bool error_ok = MSGPACK_OBJECT_NIL == error_obj.type;

        if (type_ok && msgid_ok && error_ok)
        {
            msgpack_sbuffer result_sbuf;
            msgpack_sbuffer_init(&result_sbuf);
            msgpack_packer result_pk;
            msgpack_packer_init(&result_pk, &result_sbuf, msgpack_sbuffer_write);
            msgpack_pack_object(&result_pk, result_obj);

            if (result_sbuf.size <= result_capacity)
            {
                std::memcpy(result_out, result_sbuf.data, result_sbuf.size);
                result_len = result_sbuf.size;
                success = true;
            }

            msgpack_sbuffer_destroy(&result_sbuf);
        }
    }

    msgpack_zone_destroy(&zone);
    rpc_bus_unlock();
    return success;
}

void rpc_notify(const char *method, const uint8_t *params_msgpack, std::size_t params_len)
{
    rpc_bus_lock();

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, static_cast<int>(SF_RPC_MSG_NOTIFICATION));
    pack_method_and_params(pk, sbuf, method, params_msgpack, params_len);

    transport_send_raw(reinterpret_cast<const uint8_t *>(sbuf.data), sbuf.size);
    msgpack_sbuffer_destroy(&sbuf);

    rpc_bus_unlock();
}

void rpc_bus_lock()
{
    get_bus_mutex().lock();
}

void rpc_bus_unlock()
{
    get_bus_mutex().unlock();
}

} // namespace sf_unoq

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ