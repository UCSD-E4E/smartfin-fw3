#ifndef __SLEEPTASK_HPP__
#define __SLEEPTASK_HPP__

#include "task.hpp"
#include "Particle.h"

#define SLEEP_RGB_LED_COLOR 0x4A192C
#define SLEEP_RGB_LED_PATTERN LED_PATTERN_BLINK
#define SLEEP_RGB_LED_PERIOD 20
#define SLEEP_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT
/**
 * @brief Handles process of transitioning to deep sleep
 * @details Provides methods to setup deep sleep task, defining potential boot behavior (and methods to access them) and checking for them before entering deep sleep.
 */
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
         * @brief Initiate temperature calibration on startup
         */
        BOOT_BEHAVIOR_TMP_CAL_START=1,
        /**
         * @brief Continue temperature calibration sequence
         */
        BOOT_BEHAVIOR_TMP_CAL_CONTINUE=2,
        /**
         * @brief End temperature calibration
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
     * @param behavior Boot Behavior to set
     */
    static void setBootBehavior(BOOT_BEHAVIOR_e behavior);
    /**
     * @brief Get's current boot behavior from NVRAM
     * 
     * @return Current Boot Behavior
     */
    static BOOT_BEHAVIOR_e getBootBehavior(void);
    /**
     * @brief Load initial boot behavior to NVRAM
     */
    static void loadBootBehavior(void);
    private:
    /**
     * @brief Manages LED status display settings
     */
    LEDStatus ledStatus;
};
#endif
