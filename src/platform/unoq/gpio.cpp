/**
 * @file gpio.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL GPIO functions.
 * @date 2026-07-23
 *
 * All physical GPIO lives on the STM32U585, so every function here is an
 * SF_RPC client call over the SPI link rather than a direct register
 * access. gpio_write_fast()/gpio_read_fast() fall back to the same RPC
 * path as gpio_write()/gpio_read(), per hal.hpp's documented fallback for
 * platforms without a dedicated fast path.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include "platform/unoq/pins.hpp"

#include "ipc/hal_rpc_protocol.h"

namespace SF_HAL
{

void gpio_set_mode(PinId pin, GpioMode mode)
{
    // TODO(unoq): send an SF_RPC_METHOD_GPIO_SET_MODE request with
    // [pinToWireId(pin), mode] and wait for the response.
}

void gpio_write(PinId pin, GpioState state)
{
    // TODO(unoq): send an SF_RPC_METHOD_GPIO_WRITE request with
    // [pinToWireId(pin), state] and wait for the response.
}

bool gpio_read(PinId pin)
{
    // TODO(unoq): send an SF_RPC_METHOD_GPIO_READ request with
    // [pinToWireId(pin)] and decode the returned state.
    return false;
}

void gpio_write_fast(PinId pin, GpioState state)
{
    // TODO(unoq): no dedicated fast path over SPI; forward to gpio_write().
}

bool gpio_read_fast(PinId pin)
{
    // TODO(unoq): no dedicated fast path over SPI; forward to gpio_read().
    return false;
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
