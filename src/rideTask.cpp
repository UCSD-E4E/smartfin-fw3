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
#include "scheduler.hpp"
#include "ensembles.hpp"

static void RIDE_setFileName(system_tick_t startTime){return;}
DeploymentSchedule_t deploymentSchedule[] = 
{ 
    {SS_ensembleAFunc, SS_ensembleAInit, 1, 100, 2000, UINT32_MAX, 0, 0, 0, 
                                                        nullptr,400,(char) 65},
    {SS_ensembleBFunc, SS_ensembleBInit, 1, 600, 2000, UINT32_MAX, 0, 0, 0, 
                                                        nullptr,200,(char) 66}, 
    {SS_ensembleCFunc, SS_ensembleCInit, 1, 900, 2000, UINT32_MAX, 0, 0, 0, 
                                                        nullptr,600,(char) 67}, 
    {nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr}
};

uint32_t rollingMean(DeploymentSchedule_t* event, int duration)
{

    int next = (duration - (int)event->maxDuration) /
    ((int)event->measurementCount + 1);
    return (uint32_t) (event->maxDuration + next);
}

STATES_e RideTask::run(void)
{
    
    
    DeploymentSchedule_t* pNextEvent = NULL;
    system_tick_t nextEventTime;
    int counter = 0;
    
    uint32_t d;
    SF_OSAL_printf("\nDeployment started at %" PRId32  __NL__,millis());
    while(1)
    {
        
        RIDE_setFileName(this->startTime);
        
        SCH_getNextEvent(deploymentSchedule, &pNextEvent, &nextEventTime);
        
        d = 0;
        int m = counter % 17;
        if(m == 3)
        {
            d = 200; 
        }
        else if (m == 8 || m == 13)
        {
            d = 800;
        }
        while(millis() < nextEventTime)
        {
            continue;
        }
        
        SF_OSAL_printf("|%" PRId32,(uint32_t) millis());
        pNextEvent->measure(pNextEvent);
        delay(d);
        SF_OSAL_printf("|%" PRId32  __NL__, (uint32_t) millis());
        /*pNextEvent->maxDuration = millis()-nextEventTime > 
                            pNextEvent->maxDuration ? 
                            millis()-nextEventTime:pNextEvent->maxDuration;*/
        pNextEvent->lastMeasurementTime = nextEventTime;

        /*if(pSystemDesc->pWaterSensor->getLastStatus() == 
                            WATER_SENSOR_LOW_STATE)
        {
            SF_OSAL_printf("Out of water!"  __NL__);
            return STATE_UPLOAD;
        }

        if(pSystemDesc->flags->batteryLow)
        {
            SF_OSAL_printf("Low Battery!"  __NL__);
            return STATE_DEEP_SLEEP;
        }*/
        if (counter++ > 34)
        {
            return STATE_UPLOAD;
        }

        
    }
    return STATE_UPLOAD;
}

void RideTask::init()
{
    SF_OSAL_printf("Entering STATE_DEPLOYED"  __NL__);
    pSystemDesc->pChargerCheck->start();
    this->ledStatus.setColor(RIDE_RGB_LED_COLOR);
    this->ledStatus.setPattern(RIDE_RGB_LED_PATTERN_GPS);
    this->ledStatus.setPeriod(RIDE_RGB_LED_PERIOD_GPS);
    this->ledStatus.setPriority(RIDE_RGB_LED_PRIORITY);
    this->ledStatus.setActive();
    this->startTime = millis();
    SCH_initializeSchedule(deploymentSchedule, this->startTime);
    pSystemDesc->pRecorder->openSession();

}


void RideTask::exit(void)
{
    SF_OSAL_printf("Closing session"  __NL__);
    pSystemDesc->pRecorder->closeSession();
    // Deinitialize sensors
    //pSystemDesc->pTempSensor->stop();
    //pSystemDesc->pCompass->close();
    //pSystemDesc->pIMU->close();
    //pSystemDesc->pGPS->gpsModuleStop();

}
