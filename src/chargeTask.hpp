#ifndef CHARGETASK_HPP
#define CHARGETASK_HPP

#include "platform/hal.hpp"
#include "task.hpp"
#define CLI_INTERRUPT_PHRASE  "#CLI"
#define CLI_BUFFER_LEN  5

#define CHARGE_RGB_LED_COLOR    0x00FFFF00u

#define CHARGE_RGB_LED_PATTERN SF_HAL::LedPattern::SOLID
#define CHARGE_RGB_LED_PERIOD   0
#define CHARGE_RGB_LED_PRIORITY SF_HAL::LedPriority::IMPORTANT

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
    SF_HAL::LedStatus ledStatus;
    /**
     * @brief Timestamp for tracking start of charging process
     * 
     * startTime = 0 indicates the device charging since
     * the powering on of the device
     */
    SF_HAL::tick_t startTime;

    /**
     * @brief Flag storing if data upload state successfully initializes.
     * Variable storing 1 if system successfully enters data upload state, or 0
     * if system times out before entering data upload state.
     */
    int initSuccess;
};
#endif // CHARGETASK_HPP
