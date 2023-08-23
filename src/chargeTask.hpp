#ifndef __CHARGETASK_HPP__
#define __CHARGETASK_HPP__
#include "task.hpp"

#include "Particle.h"

#define CLI_INTERRUPT_PHRASE  "#CLI"
#define CLI_BUFFER_LEN  5

#define CHARGE_RGB_LED_COLOR    RGB_COLOR_YELLOW

#define CHARGE_RGB_LED_PATTERN  LED_PATTERN_SOLID
#define CHARGE_RGB_LED_PERIOD   0
#define CHARGE_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT

class ChargeTask : public Task{
    public:
     * @brief initialize charge task
     * Sets LEDs
    */
    void init(void);
    /**
     * @brief Charges the device, and exits to CLI on command
    */
    STATES_e run(void);
    void exit(void);

    private:
    char inputBuffer[CLI_BUFFER_LEN];
    LEDStatus ledStatus;
    system_tick_t startTime;
};
#endif
