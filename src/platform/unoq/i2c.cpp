/**
 * @file i2c.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Brent Brewster
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL I2C functions.
 * @date 2026-09-22
 *
 * The physical I2C bus (temp sensor, wet/dry sensor, IMU) is wired to the
 * STM32U585, so i2c_read()/i2c_write() are SF_RPC client calls rather than
 * direct bus access. i2c_lock()/i2c_unlock() still guard this process's
 * side of the link, since concurrent RPC calls from multiple threads would
 * otherwise interleave on the wire.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "ipc/hal_rpc_protocol.h"
#include "platform/hal.hpp"
#include "platform/unoq/rpc_client.hpp"

#include <msgpack.h>
#include <cstring>

/**
 * @brief Placeholder TwoWire satisfying i2c_get_wire()'s return type.
 */
class TwoWire
{
};

namespace
{

TwoWire g_dummy_wire;
bool g_i2c_enabled = false;

/**
 * @brief Mutex serializing multi-threaded I2C access.
 * Wrapped in a function to prevent Static Initialization Order Fiascos (SIOF) on shutdown.
 */
SF_HAL::Mutex& get_i2c_mutex() {
    static SF_HAL::Mutex m;
    return m;
}

} // namespace

namespace SF_HAL
{

void i2c_begin()
{
    g_i2c_enabled = true;
}

bool i2c_is_enabled()
{
    return g_i2c_enabled;
}

int i2c_read(uint8_t address, char* data, int length, bool repeated)
{
    if (!g_i2c_enabled) return -1;

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Request format: [address, length, repeated]
    msgpack_pack_array(&pk, 3);
    msgpack_pack_uint8(&pk, address);
    msgpack_pack_int(&pk, length);
    if (repeated) msgpack_pack_true(&pk); else msgpack_pack_false(&pk);

    uint8_t result_buf[256]; // Assuming standard I2C payload sizes
    std::size_t result_len = 0;

    const bool ok = sf_unoq::rpc_call(SF_RPC_METHOD_I2C_READ,
                                      reinterpret_cast<const uint8_t *>(sbuf.data),
                                      sbuf.size,
                                      result_buf,
                                      sizeof(result_buf),
                                      result_len);
    msgpack_sbuffer_destroy(&sbuf);

    if (!ok) return -1;

    // Unpack response: expects [status_code, data_bin]
    msgpack_zone zone;
    msgpack_zone_init(&zone, 1024);
    msgpack_object obj;
    std::size_t offset = 0;
    int status = -1;

    if (msgpack_unpack(reinterpret_cast<const char *>(result_buf), result_len, &offset, &zone, &obj) == MSGPACK_UNPACK_SUCCESS)
    {
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size >= 1)
        {
            status = static_cast<int>(obj.via.array.ptr[0].via.i64);
            
            // If success (0) and there is a payload attached, copy it out
            if (status == 0 && obj.via.array.size >= 2)
            {
                msgpack_object& data_obj = obj.via.array.ptr[1];
                if (data_obj.type == MSGPACK_OBJECT_BIN && data_obj.via.bin.size <= static_cast<uint32_t>(length))
                {
                    std::memcpy(data, data_obj.via.bin.ptr, data_obj.via.bin.size);
                }
            }
        }
    }

    msgpack_zone_destroy(&zone);
    return status;
}

int i2c_write(uint8_t address, const char* data, int length, bool repeated)
{
    if (!g_i2c_enabled) return -1;

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Request format: [address, repeated, data_bin]
    msgpack_pack_array(&pk, 3);
    msgpack_pack_uint8(&pk, address);
    if (repeated) msgpack_pack_true(&pk); else msgpack_pack_false(&pk);
    msgpack_pack_bin(&pk, length);
    msgpack_pack_bin_body(&pk, data, length);

    uint8_t result_buf[64];
    std::size_t result_len = 0;

    const bool ok = sf_unoq::rpc_call(SF_RPC_METHOD_I2C_WRITE,
                                      reinterpret_cast<const uint8_t *>(sbuf.data),
                                      sbuf.size,
                                      result_buf,
                                      sizeof(result_buf),
                                      result_len);
    msgpack_sbuffer_destroy(&sbuf);

    if (!ok) return -1;

    // Unpack response: expects [status_code]
    msgpack_zone zone;
    msgpack_zone_init(&zone, 256);
    msgpack_object obj;
    std::size_t offset = 0;
    int status = -1;

    if (msgpack_unpack(reinterpret_cast<const char *>(result_buf), result_len, &offset, &zone, &obj) == MSGPACK_UNPACK_SUCCESS)
    {
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size >= 1)
        {
            status = static_cast<int>(obj.via.array.ptr[0].via.i64);
        }
    }

    msgpack_zone_destroy(&zone);
    return status;
}

::TwoWire& i2c_get_wire()
{
    return g_dummy_wire;
}

void i2c_lock()
{
    get_i2c_mutex().lock();
}

void i2c_unlock()
{
    get_i2c_mutex().unlock();
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ