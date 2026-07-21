/**
 * @file led.cpp
 * @brief Manages onboard LED
 * @version 0.1
 * @date 2023-08-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "led.hpp"

#include "platform/hal.hpp"
SFLed* SFLed::firstLED = NULL;

SFLed::SFLed(SF_HAL::PinId pin, SFLed::SFLED_State_e state)
{
    this->pin = pin;
    this->state = state;
}

void SFLed::init(void)
{
    SF_HAL::gpio_set_mode(this->pin, SF_HAL::GpioMode::OUTPUT);

    switch(this->state)
    {
        case SFLed::SFLED_STATE_BLINK:
        case SFLed::SFLED_STATE_ON:
            SF_HAL::gpio_write(this->pin, SF_LED_ON_VALUE);
            break;
        default:
        case SFLed::SFLED_STATE_OFF:
            SF_HAL::gpio_write(this->pin, SF_LED_OFF_VALUE);
            break;
    }

    // Stack to top of LED linkedlist
    this->nextLED = SFLed::firstLED;
    SFLed::firstLED = this;
}

SFLed::~SFLed(void)
{
    SFLed* node = SFLed::firstLED;
    SF_HAL::gpio_set_mode(this->pin, SF_HAL::GpioMode::INPUT);

    // remove self from linkedlist
    while(node != this)
    {
        node = node->nextLED;
    }
    if(node == this)
    {
        SFLed::firstLED = this->nextLED;
    }
}

void SFLed::setState(SFLED_State_e state)
{
    this->state = state;
}

SFLed::SFLED_State_e SFLed::getState(void) const
{
    return this->state;
}

void SFLed::toggle(void)
{
    switch(this->state)
    {
        case SFLed::SFLED_STATE_BLINK:
            return;
        
        case SFLed::SFLED_STATE_OFF:
            this->state = SFLed::SFLED_STATE_ON;
            return;
        
        case SFLed::SFLED_STATE_ON:
            this->state = SFLed::SFLED_STATE_OFF;
            return;
    }
}

void SFLed::doLEDs(void)
{
    const SFLed* node = SFLed::firstLED;
    for(; node; node = node->nextLED)
    {
        switch(node->state)
        {
            case SFLed::SFLED_STATE_OFF:
                SF_HAL::gpio_write_fast(node->pin, SF_LED_OFF_VALUE);
                break;
            
            case SFLed::SFLED_STATE_ON:
                SF_HAL::gpio_write_fast(node->pin, SF_LED_ON_VALUE);
                break;

            case SFLed::SFLED_STATE_BLINK:
            {
                SF_HAL::GpioState next = SF_HAL::gpio_read_fast(node->pin)
                    ? SF_HAL::GpioState::LOW : SF_HAL::GpioState::HIGH;
                SF_HAL::gpio_write_fast(node->pin, next);
                break;
            }
        }
    }
}
