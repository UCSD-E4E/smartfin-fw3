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

/**
 * @brief Handles charging process
 * @details Contains methods for starting charging task (setting LEDs, etc), detecting if charger is connected, and ending charging task
 */
class ChargeTask : public Task{
    public:
    /**
     * @brief initialize charge task
     * Sets LEDs
    */
    void init(void);
    /**
     * @brief Charges the device, and exits to CLI on command
     * 
     * @return STATE_CLI when CLI pattern entered or
     * STATE_DEEP_SLEEP when device not charging
     */
    STATES_e run(void);
    void exit(void);

    private:
    /**
     * @brief Buffer for storing command-line input
     * 
     * Character array holds input from CLI with a maxiumum size of 'CLI_BUFFER_LEN'
    */
    char inputBuffer[CLI_BUFFER_LEN];
    /**
     * @brief LED status indicator for charging task
     *
     * Object defines visual status of device's charging state
     */
    LEDStatus ledStatus;
    /**
     * @brief Timestamp for tracking start of charging process
     * 
     * startTime = 0 indicates the device charging since
     * the powering on of the device
     */
    system_tick_t startTime;
};
#endif
