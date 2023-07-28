#ifndef __CHARGETASK_HPP__
#define __CHARGETASK_HPP__
#include "task.hpp"

#include "Particle.h"

#define CLI_INTERRUPT_PHRASE  "#CLI"
#define CLI_BUFFER_LEN  5

#define CHARGE_RGB_LED_COLOR    0x000000
#define CHARGE_RGB_LED_PATTERN  LED_PATTERN_SOLID
#define CHARGE_RGB_LED_PERIOD   0
#define CHARGE_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT

class ChargeTask : public Task{
    public:
    void init(void);
    STATES_e run(void);
    void exit(void);

    private:
    char inputBuffer[CLI_BUFFER_LEN];
    LEDStatus ledStatus;
    system_tick_t startTime;
};
#endif