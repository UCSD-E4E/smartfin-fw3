/**
 * @file pins.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Board pin table mapping SF_RPC wire pin identifiers to physical
 *        STM32U585 GPIO ports and pins on the Arduino Uno Q.
 * @date 2026-09-18
 *
 * The wire pin ids are the ordinal values of SF_HAL::PinId, so the order of
 * the entries below must stay in lockstep with that enum. The Arduino
 * header designation for each pin is given because the peripheral board
 * mates to the Uno Q's JDIGITAL/JANALOG headers, and the header pin is the
 * stable reference between the schematic and this table.
 *
 * Pin selection constraints this table respects:
 * - Outputs avoid wake-capable pins, keeping those free for future wake
 *   sources.
 * - Wkp must sit on a wake-capable pin or SF_HAL::system_sleep_gpio_wake()
 *   cannot work; PB2 is WKUP1.
 * - The PWM-capable header pins (~D3, ~D5, ~D6, ~D9), SPI2 (D10 to D13),
 *   I2C3 (A4/A5) and USART1 (D0/D1) are all left unclaimed.
 *
 * Assigning a pin here does not configure it. The GPIO mode, pull and speed
 * live in the STM32CubeMX project and are applied by MX_GPIO_Init().
 */
#include "pins.hpp"

#include "stm32u5xx_hal.h"

#include <cstddef>

namespace sf_mcu
{

    namespace
    {

        /**
         * @brief One row of the board pin table.
         */
        struct PinTableEntry
        {
            GPIO_TypeDef *port; ///< GPIO port base for this pin.
            uint16_t pin;       ///< Pin bitmask within the port.
        };

        /**
         * @brief Wire pin id to physical pin, indexed by SF_HAL::PinId ordinal.
         */
        constexpr PinTableEntry kPinTable[] = {
            {GPIOB, GPIO_PIN_10}, ///< 0 UsbPwrDetect   D21, WKUP8 if ever needed.
            {GPIOB, GPIO_PIN_3},  ///< 1 StatLed        D2.
            {GPIOB, GPIO_PIN_4},  ///< 2 WaterDetectEn  D8.
            {GPIOA, GPIO_PIN_7},  ///< 3 WaterDetect    A3, ADC-capable if the
                                  ///< wet/dry sense ever needs a threshold.
            {GPIOA, GPIO_PIN_12}, ///< 4 WaterStatusLed D4.
            {GPIOB, GPIO_PIN_11}, ///< 5 WaterMfgTestEn D20.
            {GPIOB, GPIO_PIN_2},  ///< 6 Wkp            D7, WKUP1.
        };

        constexpr std::size_t kPinCount = sizeof(kPinTable) / sizeof(kPinTable[0]);

    } // namespace

    ResolvedPin resolve_pin(uint8_t wire_pin_id)
    {
        if (wire_pin_id >= kPinCount)
        {
            return ResolvedPin{nullptr, 0};
        }

        const PinTableEntry &entry = kPinTable[wire_pin_id];
        return ResolvedPin{static_cast<void *>(entry.port), entry.pin};
    }

} // namespace sf_mcu
