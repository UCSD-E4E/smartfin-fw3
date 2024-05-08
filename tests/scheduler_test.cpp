#include <unistd.h>

#include "../src/scheduler.hpp"
#include <iostream>


void delay(uint32_t seconds)
{
    sleep(seconds);
}
static void SS_ensembleAInit(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensembleAFunc(DeploymentSchedule_t* pDeployment)
{
    uint32_t d = 4;
    if ( pDeployment->measurementCount == 1)
    {
        d = 6;
    }
    delay(d);
    
    return;
}


static void SS_ensembleBInit(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensembleBFunc(DeploymentSchedule_t* pDeployment)
{
    uint32_t d = 2;
    if ( pDeployment->measurementCount == 3)
    {
        d = 4;
    }
    delay(d);
    
    return;
}

static void SS_ensembleCInit(DeploymentSchedule_t* pDeployment){return;}
static void SS_ensembleCFunc(DeploymentSchedule_t* pDeployment)
{
    uint32_t d = 6;
    if ( pDeployment->measurementCount == 3)
    {
        d = 15;
    }
    delay(d);
    return;
}


int main() {
    
    uint32_t startTime = millis();
    DeploymentSchedule_t deploymentSchedule[] = {
        {SS_ensembleAFunc, SS_ensembleAInit, 1, startTime, 2000, UINT32_MAX, 0, 0, 0, nullptr,400,"A"},
        {SS_ensembleBFunc, SS_ensembleBInit, 1, startTime, 2000, UINT32_MAX, 0, 0, 0, nullptr,200,"B"},
        {SS_ensembleCFunc, SS_ensembleCInit, 1, startTime, 2000, UINT32_MAX, 0, 0, 0, nullptr,600,"C"},
        {nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr}
    };

    SCH_initializeSchedule(deploymentSchedule, startTime);

    DeploymentSchedule_t* nextEvent = nullptr;
    uint32_t nextEventTime = 0;
    int counter = 0;

    while (counter < 5) {  
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        if (nextEvent != nullptr) {
            while(millis() < startTime)
            {
                delay(1);
            }
            std::cout << "|" << millis();
            nextEvent->measure(nextEvent);
            std::cout << "|" << millis() << "\n";
            nextEvent->lastMeasurementTime = nextEventTime;
            nextEvent->measurementCount++;
        }
        counter++;
    }
    
    return 0;
}
