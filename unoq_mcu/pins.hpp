/**
 * @file pins.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Maps SF_RPC wire pin identifiers to physical STM32U585 GPIO ports
 *        and pins for the Uno Q board.
 * @date 2026-07-23
 *
 * The wire pin ids received over SF_RPC calls correspond to the ordinal
 * values of SF_HAL::PinId (src/platform/hal_types.hpp). This header owns
 * the one place that translates those ids to real hardware for this board.
 */
#ifndef SF_MCU_PINS_HPP
#define SF_MCU_PINS_HPP

#include <cstdint>

namespace sf_mcu
{

/**
 * @brief A resolved physical GPIO location on the STM32U585.
 */
struct ResolvedPin
{
    void *port;   ///< GPIO port base (e.g. GPIOA), typed void* to avoid
                 ///< pulling the STM32 HAL headers into this declaration.
    uint16_t pin; ///< Pin bitmask within the port.
};

/**
 * @brief Resolve a wire protocol pin id to a physical STM32U585 pin.
 *
 * @param wire_pin_id Pin id as received in an SF_RPC GPIO call, matching
 *                     the ordinal value of a @c SF_HAL::PinId member.
 * @return The resolved port/pin location for this board.
 */
ResolvedPin resolve_pin(uint8_t wire_pin_id);

} // namespace sf_mcu

#endif // SF_MCU_PINS_HPP
