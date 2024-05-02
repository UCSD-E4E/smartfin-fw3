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

static void RIDE_setFileName(system_tick_t startTime){return;}

static void SS_ensemble10Func(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensemble10Init(DeploymentSchedule_t* pDeployment);

static void SS_ensemble07Func(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensemble07Init(DeploymentSchedule_t* pDeployment);

static void SS_ensemble08Func(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensemble08Init(DeploymentSchedule_t* pDeployment);

static void SS_fwVerInit(DeploymentSchedule_t* pDeployment){return;}
static void SS_fwVerFunc(DeploymentSchedule_t* pDeployment){return;}


typedef struct Ensemble10_eventData_
{
    double temperature;
    int32_t water;
    int32_t acc[3];
    int32_t ang[3];
    int32_t mag[3];
    int32_t location[2];
    uint8_t hasGPS;
    uint32_t accumulateCount;
}Ensemble10_eventData_t;

typedef struct Ensemble07_eventData_
{
    float battVoltage;
    uint32_t accumulateCount;
}Ensemble07_eventData_t;

typedef struct Ensemble08_eventData_
{
    double temperature;
    int32_t water;

    uint32_t accumulateCount;
}Ensemble08_eventData_t;

static Ensemble10_eventData_t ensemble10Data;
static Ensemble07_eventData_t ensemble07Data;
static Ensemble08_eventData_t ensemble08Data;

DeploymentSchedule_t deploymentSchedule[] = 
{
    {&SS_ensemble10Func, &SS_ensemble10Init, 1, 0, 1000, UINT32_MAX, 0, 0, 0, &ensemble10Data},
    {&SS_ensemble07Func, &SS_ensemble07Init, 1, 0, 10000, UINT32_MAX, 0, 0, 0, &ensemble07Data},
    {&SS_ensemble08Func, &SS_ensemble08Init, 1, 0, UINT32_MAX, UINT32_MAX, 0, 0, 0, &ensemble08Data},
    {&SS_fwVerFunc, &SS_fwVerInit, 1, 0, UINT32_MAX, UINT32_MAX, 0, 0, 0, NULL},
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, NULL}
};

uint32_t rollingMean(DeploymentSchedule_t* event, int duration){
    
    int next = (duration - (int)event->meanDuration)/((int)event->measurementCount + 1);
    return (uint32_t) (event->meanDuration + next);
}

STATES_e RideTask::run(void){
    SF_OSAL_printf("\nStarting RideTask::run\n");
    DeploymentSchedule_t* pNextEvent = NULL;
    system_tick_t nextEventTime;
    int counter = 0;
    while(1)
    {
        RIDE_setFileName(this->startTime);

        SCH_getNextEvent(deploymentSchedule, &pNextEvent, &nextEventTime);
        {
        
            while(millis() < nextEventTime)
            {
                continue;
            }
            
            pNextEvent->measure(pNextEvent);
            pNextEvent->meanDuration = rollingMean(pNextEvent, millis() - nextEventTime);
            pNextEvent->lastMeasurementTime = nextEventTime;
            pNextEvent->measurementCount++;
        
            if(pSystemDesc->pWaterSensor->getLastStatus() == WATER_SENSOR_LOW_STATE)
            {
                SF_OSAL_printf("Out of water!\n");
                //return STATE_UPLOAD;
            }

            if(pSystemDesc->flags->batteryLow)
            {
                //SF_OSAL_printf("Low Battery!\n");
                //return STATE_DEEP_SLEEP;
            }
            if (counter++ > 20)
            {
                return STATE_UPLOAD;
            }
        
        }
    }
}

void RideTask::init()
{
    SF_OSAL_printf("Entering STATE_DEPLOYED\n");
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
    SF_OSAL_printf("Closing session\n");
    pSystemDesc->pRecorder->closeSession();
    // Deinitialize sensors
    //pSystemDesc->pTempSensor->stop();
    //pSystemDesc->pCompass->close();
    //pSystemDesc->pIMU->close();
    //pSystemDesc->pGPS->gpsModuleStop();

}
static void SS_ensemble10Init(DeploymentSchedule_t* pDeployment)
{
    memset(&ensemble10Data, 0, sizeof(Ensemble10_eventData_t));
    pDeployment->pData = &ensemble10Data;
    SF_OSAL_printf("SS_ensemble10Init run at %u!\n", millis());
}

static void SS_ensemble07Init(DeploymentSchedule_t* pDeployment)
{
    memset(&ensemble07Data, 0, sizeof(Ensemble07_eventData_t));
    pDeployment->pData = &ensemble07Data;
    SF_OSAL_printf("SS_ensemble07Init run at %u!\n", millis());

}

static void SS_ensemble08Init(DeploymentSchedule_t* pDeployment)
{
    memset(&ensemble08Data, 0, sizeof(Ensemble08_eventData_t));
    pDeployment->pData = &ensemble08Data;
    SF_OSAL_printf("SS_ensemble08Init run at %u!\n", millis());
}
