#ifndef __SLEEPTASK_HPP__
#define __SLEEPTASK_HPP__

#include "task.hpp"
#include "Particle.h"

#define SLEEP_RGB_LED_COLOR    0x000000
#define SLEEP_RGB_LED_PATTERN  LED_PATTERN_SOLID
#define SLEEP_RGB_LED_PERIOD   0
#define SLEEP_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT

class SleepTask : public Task {
    public:
    void init(void);
    STATES_e run(void);
    void exit(void);

    /**
     * @brief Defines possible boot behaviors
     */
    typedef enum BOOT_BEHAVIOR_
    {
        /**
         * @brief Standard boot behavior
         */
        BOOT_BEHAVIOR_NORMAL=0,
        /**
         * @brief Initiate temporary calibration on startup
         */
        BOOT_BEHAVIOR_TMP_CAL_START=1,
        /**
         * @brief Continue temporary calibration sequence
         */
        BOOT_BEHAVIOR_TMP_CAL_CONTINUE=2,
        /**
         * @brief End temporary calibration
         */
        BOOT_BEHAVIOR_TMP_CAL_END=3,
        /**
         * @brief Reattempts data upload
         */
        BOOT_BEHAVIOR_UPLOAD_REATTEMPT=4,
        /**
         * @brief Boot behavior is not specified
         */
        BOOT_BEHAVIOR_NOT_SET=255
    /**
     * @brief Type definition for Boot Behavior enum
     */
    } BOOT_BEHAVIOR_e;

    /**

     * @brief Converts the boot behavior to a string
     * 
     * @param behavior Boot Behavior to convert
     * 
     * @return const char* Behavior name
     */
    static const char* strBootBehavior(BOOT_BEHAVIOR_e behavior);
    /**
     * @brief Updates boot behavior to NVRAM
     * 
     * @param BOOT_BEHAVIOR_e Boot Behavior to set
     */
    static void setBootBehavior(BOOT_BEHAVIOR_e);
    /**
     * @brief Get's current boot behavior from NVRAM
     * 
     * @return Current Boot Behavior
     */
    static BOOT_BEHAVIOR_e getBootBehavior(void);
    /**
     * @brief Load boot behavior onto board
     */
    static void loadBootBehavior(void);
    private:
    /**
     * @brief Stores current Boot Behavior
     */
    static BOOT_BEHAVIOR_e bootBehavior;
    /**
     * @brief Manages LED status display settings
     */
    LEDStatus ledStatus;
};
#endif
