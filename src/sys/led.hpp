#ifndef __LED_HPP__
#define __LED_HPP__

#include <cstdint>
#include "Particle.h"

#define SF_LED_NUM_LEDS 2

#define SF_LED_ON_VALUE LOW
#define SF_LED_OFF_VALUE    HIGH
#define SF_LED_BLINK_MS 1000

class SFLed
{
    public:
    typedef enum SFLED_State_
    {
        SFLED_STATE_OFF,
        SFLED_STATE_ON,
        SFLED_STATE_BLINK
    }SFLED_State_e;
    SFLed(uint8_t pin, SFLED_State_e state);
    ~SFLed(void);

    void setState(SFLED_State_e state);
    SFLED_State_e getState(void) const;
    void toggle(void);
    void init(void);

    static void doLEDs(void);


    private:
    uint8_t pin;
    SFLED_State_e state;
    SFLed* nextLED;

    static SFLed* firstLED;
    static Timer ledTimer;
};
#endif
