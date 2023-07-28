#include "led.hpp"

#include "Particle.h"

Timer SFLed::ledTimer(SF_LED_BLINK_MS, SFLed::doLEDs, false);
SFLed* SFLed::firstLED = NULL;

SFLed::SFLed(uint8_t pin, SFLed::SFLED_State_e state)
{
    this->pin = pin;
    this->state = state;
}

void SFLed::init(void)
{
    pinMode(this->pin, OUTPUT);
    switch(this->state)
    {
        case SFLed::SFLED_STATE_BLINK:
        case SFLed::SFLED_STATE_ON:
            digitalWrite(this->pin, SF_LED_ON_VALUE);
            break;
        default:
        case SFLed::SFLED_STATE_OFF:
            digitalWrite(this->pin, SF_LED_OFF_VALUE);
            break;
    }

    // Stack to top of LED linkedlist
    this->nextLED = SFLed::firstLED;
    SFLed::firstLED = this;
    this->ledTimer.start();
}

SFLed::~SFLed(void)
{
    SFLed* node = SFLed::firstLED;
    pinMode(this->pin, INPUT);
    
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
                digitalWriteFast(node->pin, SF_LED_OFF_VALUE);
                break;
            
            case SFLed::SFLED_STATE_ON:
                digitalWriteFast(node->pin, SF_LED_ON_VALUE);
                break;

            case SFLed::SFLED_STATE_BLINK:
                digitalWriteFast(node->pin, !pinReadFast(node->pin));
                break;
        }
    }
}