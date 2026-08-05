/**
 * @file i2c.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL I2C functions.
 * @date 2026-07-23
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

/**
 * @brief Placeholder TwoWire satisfying i2c_get_wire()'s return type.
 *
 * Must be defined at global scope to complete hal.hpp's
 * @c class @c TwoWire; forward declaration; an anonymous-namespace
 * definition would be a distinct, incompatible type. Exists only because
 * the IMU driver (src/imu/newIMU.hpp) has not yet been decoupled from
 * Particle's TwoWire type (see the TODO in that file). Never used for
 * real I2C traffic on this platform; i2c_read()/i2c_write() are the real
 * path. Remove alongside hal.hpp's i2c_get_wire() and this forward
 * declaration once the IMU rework lands.
 */
class TwoWire
{
};

namespace
{

TwoWire g_dummy_wire;

} // namespace

namespace SF_HAL
{

void i2c_begin()
{
    // TODO(unoq): mark the RPC-backed I2C bus as initialised.
}

bool i2c_is_enabled()
{
    // TODO(unoq): report whether i2c_begin() has been called.
    return false;
}

int i2c_read(uint8_t address, char* data, int length, bool repeated)
{
    // TODO(unoq): send an SF_RPC_METHOD_I2C_READ request with
    // [address, length, repeated], copy the returned data into data, and
    // return the returned SF_RPC_Status.
    return SF_RPC_STATUS_ERR_GENERIC;
}

int i2c_write(uint8_t address, const char* data, int length, bool repeated)
{
    // TODO(unoq): send an SF_RPC_METHOD_I2C_WRITE request with
    // [address, repeated, data] and return the returned SF_RPC_Status.
    return SF_RPC_STATUS_ERR_GENERIC;
}

::TwoWire& i2c_get_wire()
{
    return g_dummy_wire;
}

void i2c_lock()
{
    // TODO(unoq): acquire a mutex guarding this process's SF_RPC calls.
}

void i2c_unlock()
{
    // TODO(unoq): release the mutex acquired in i2c_lock().
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
