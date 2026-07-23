/**
 * @file pins.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Translates SF_HAL::PinId to the wire pin id sent to the STM32U585
 *        peripheral server.
 * @date 2026-07-23
 */
#pragma once

#include "ipc/hal_rpc_protocol.h"
#include "platform/hal.hpp"

#include <cstdint>

namespace SF_HAL
{

/**
 * @brief Translate a logical @c PinId to the wire pin id used in SF_RPC
 *        GPIO calls.
 *
 * The wire id is the ordinal of @p id; unoq_mcu/pins.hpp maps that same
 * ordinal to a physical STM32U585 pin. Only valid inside
 * platform/unoq translation units. Never call from board-agnostic code.
 *
 * @param id Logical pin identifier.
 * @return Wire pin id corresponding to @p id.
 */
inline uint8_t pinToWireId(PinId id)
{
    return static_cast<uint8_t>(id);
}

} // namespace SF_HAL
