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

    typedef enum BOOT_BEHAVIOR_
    {
        BOOT_BEHAVIOR_NORMAL=0,
        BOOT_BEHAVIOR_TMP_CAL_START=1,
        BOOT_BEHAVIOR_TMP_CAL_CONTINUE=2,
        BOOT_BEHAVIOR_TMP_CAL_END=3,
        BOOT_BEHAVIOR_UPLOAD_REATTEMPT=4,
        BOOT_BEHAVIOR_NOT_SET=255
    } BOOT_BEHAVIOR_e;

    static void setBootBehavior(BOOT_BEHAVIOR_e);
    static BOOT_BEHAVIOR_e getBootBehavior(void);
    static void loadBootBehavior(void);
    private:
    static BOOT_BEHAVIOR_e bootBehavior;
    LEDStatus ledStatus;
};
#endif