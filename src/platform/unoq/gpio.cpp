/**
 * @file gpio.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Brent
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL GPIO functions.
 * @date 2026-07-23
 *
 * All physical GPIO lives on the STM32U585, so every function here is an
 * SF_RPC client call over the SPI link (via sf_unoq::rpc_call(), see
 * rpc_client.hpp) rather than a direct register access.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include "platform/unoq/pins.hpp"
#include "platform/unoq/rpc_client.hpp"
#include "ipc/hal_rpc_protocol.h"
#include <msgpack.h>

namespace SF_HAL
{

void gpio_set_mode(PinId pin, GpioMode mode)
{
    // Guard against global teardown passing invalid pin IDs during exit
    if (static_cast<int>(pin) < 0 || static_cast<int>(pin) >= 64) {
        return;
    }

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 2);
    msgpack_pack_uint8(&pk, pinToWireId(pin));
    msgpack_pack_uint8(&pk, static_cast<uint8_t>(mode));

    uint8_t result[16];
    std::size_t result_len = 0;
    sf_unoq::rpc_call(SF_RPC_METHOD_GPIO_SET_MODE,
                      reinterpret_cast<const uint8_t *>(sbuf.data),
                      sbuf.size,
                      result,
                      sizeof(result),
                      result_len);

    msgpack_sbuffer_destroy(&sbuf);
}

void gpio_write(PinId pin, GpioState state)
{
    // Guard against global teardown passing invalid pin IDs during exit
    if (static_cast<int>(pin) < 0 || static_cast<int>(pin) >= 64) {
        return;
    }

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 2);
    msgpack_pack_uint8(&pk, pinToWireId(pin));
    msgpack_pack_uint8(&pk, static_cast<uint8_t>(state));

    uint8_t result[16];
    std::size_t result_len = 0;
    sf_unoq::rpc_call(SF_RPC_METHOD_GPIO_WRITE,
                      reinterpret_cast<const uint8_t *>(sbuf.data),
                      sbuf.size,
                      result,
                      sizeof(result),
                      result_len);

    msgpack_sbuffer_destroy(&sbuf);
}

bool gpio_read(PinId pin)
{
    if (static_cast<int>(pin) < 0 || static_cast<int>(pin) >= 64) {
        return false;
    }

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 1);
    msgpack_pack_uint8(&pk, pinToWireId(pin));

    uint8_t result[16];
    std::size_t result_len = 0;
    const bool ok = sf_unoq::rpc_call(SF_RPC_METHOD_GPIO_READ,
                                      reinterpret_cast<const uint8_t *>(sbuf.data),
                                      sbuf.size,
                                      result,
                                      sizeof(result),
                                      result_len);

    msgpack_sbuffer_destroy(&sbuf);

    if (!ok)
    {
        return false;
    }

    msgpack_zone zone;
    msgpack_zone_init(&zone, 64);
    msgpack_object state_obj;
    std::size_t offset = 0;
    const msgpack_unpack_return unpack_status = msgpack_unpack(
        reinterpret_cast<const char *>(result), result_len, &offset, &zone, &state_obj);

    bool state = false;

    if (MSGPACK_UNPACK_SUCCESS == unpack_status && MSGPACK_OBJECT_BOOLEAN == state_obj.type)
    {
        state = state_obj.via.boolean;
    }

    msgpack_zone_destroy(&zone);
    return state;
}

void gpio_write_fast(PinId pin, GpioState state)
{
    gpio_write(pin, state);
}

bool gpio_read_fast(PinId pin)
{
    return gpio_read(pin);
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ