#include "rideTask.hpp"
#include "Particle.h"
#include <time.h>
#include "cli/conio.hpp"
#include "consts.hpp"
#include "product.hpp"
#include "system.hpp"
#include "sleepTask.hpp"
#include "util.hpp"
#include "vers.hpp"
STATES_e RideTask::run(void)
{
    SF_OSAL_printf("Running SYSTEM_STATE_RIDE" __NL__);
    
    for (int i = 0; i < 1000000; i++)
    {
        for (int x = 0; x < 1000000; x++)
        {
            int p = 10;
            p += 1;
        }
    }

    return STATE_CLI;

        
}
void RideTask::init()
{
    SF_OSAL_printf("Entering SYSTEM_STATE_RUNNING" __NL__);
    pSystemDesc->pChargerCheck->start();
    this->ledStatus.setColor(RIDE_RGB_LED_COLOR);
    this->ledStatus.setPattern(RIDE_RGB_LED_PATTERN_GPS);
    this->ledStatus.setPeriod(RIDE_RGB_LED_PERIOD_GPS);
    this->ledStatus.setPriority(RIDE_RGB_LED_PRIORITY);
    this->ledStatus.setActive();
    this->startTime = millis();
}
void RideTask::exit()
{

}