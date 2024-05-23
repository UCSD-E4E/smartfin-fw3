#ifndef __RIDE_HPP__
#define __RIDE_HPP__
#include "task.hpp"

#include "Particle.h"
#include "product.hpp"

#define RIDE_RGB_LED_COLOR    RGB_COLOR_WHITE
#define RIDE_RGB_LED_PATTERN_GPS  LED_PATTERN_BLINK
#define RIDE_RGB_LED_PERIOD_GPS   500
#define RIDE_RGB_LED_PATTERN_NOGPS  LED_PATTERN_SOLID
#define RIDE_RGB_LED_PERIOD_NOGPS   0
#define RIDE_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT

#define RIDE_GPS_STARTUP_MS 500
#define RIDE_WATER_DETECT_SURF_SESSION_INIT_WINDOW  WATER_DETECT_SURF_SESSION_INIT_WINDOW

#define RIDE_INIT_WATER_TIMEOUT_MS  SURF_SESSION_GET_INTO_WATER_TIMEOUT_MS


class RideTask : public Task
{
public:
    void init(void);
    STATES_e run(void);
    void exit(void);
private:
    LEDStatus ledStatus;
    bool gpsLocked;
    system_tick_t startTime;

};


#endif