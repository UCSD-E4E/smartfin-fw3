#ifndef TEST_VERSION
#include "scheduler.hpp"
#include "Particle.h"
#include "cli/conio.hpp"

#else
#include "../tests/scheduler_test_system.hpp"
#define MIN_MEASUREMENT_SEPARATION 0
#endif

void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment, system_tick_t startTime) {
    while(pDeployment->init) {
        pDeployment->ensembleDelay = 0;
        pDeployment->lastMeasurementTime = 0;
        pDeployment->measurementCount = 0;
        pDeployment->init(pDeployment);
        pDeployment++;
    }
}
void SCH_getNextEvent(DeploymentSchedule_t* scheduleTable, DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime, uint32_t* currentTime) {
    
    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    
    for (int i = 0; scheduleTable[i].measure; ++i)  {
        DeploymentSchedule_t* currentEvent = &scheduleTable[i];

        // Calculate the next scheduled start time
        
        uint32_t nextStartTime = currentEvent->deploymentStartTime + currentEvent->ensembleDelay + (currentEvent->measurementCount * currentEvent->ensembleInterval);
        
        // If  past scheduled start time and it's time for the next interval
        if (*currentTime > nextStartTime)  {
            nextStartTime = *currentTime;
        }
        
        // Calculate the expected end time if we started at the next start time
        uint32_t proposedEndTime = nextStartTime + currentEvent->meanDuration;
        
        // Check if running this event would overlap with any subsequent events
        bool willOverlap = false;
        
        for (int j = 0; scheduleTable[j].measure; ++j)  {
            if (i != j)  {
                DeploymentSchedule_t* otherEvent = &scheduleTable[j];
                uint32_t otherNextStartTime = otherEvent->deploymentStartTime + otherEvent->ensembleDelay +
                                                   (otherEvent->measurementCount * otherEvent->ensembleInterval);
                if (*currentTime > otherNextStartTime) {
                    otherNextStartTime += ((*currentTime - otherNextStartTime) / otherEvent->ensembleInterval + 1) * otherEvent->ensembleInterval;
                }

                if (proposedEndTime > otherNextStartTime && nextStartTime < otherNextStartTime) {
                    willOverlap = true;
                    
                    break;
                }
            }
        }
        // Check if ensemble will overlap with next time the same ensemble is scheduled
        if (!willOverlap && currentEvent->measurementCount != 0) {
            uint32_t nextScheduled = currentEvent->deploymentStartTime + currentEvent->ensembleDelay;
            uint32_t intendedMeasureCount = (nextStartTime - nextScheduled) / currentEvent->ensembleInterval + 1;
            nextScheduled += intendedMeasureCount * currentEvent->ensembleInterval;
            if (nextScheduled < proposedEndTime)
            {
                currentEvent->measurementCount = intendedMeasureCount;
                nextStartTime = nextScheduled;
            }
        }
        // Prioritize tasks that can be executed on time and ensure no overlaps
        if (!willOverlap && nextStartTime >= *currentTime && nextStartTime < minNextTime) {
            minNextTime = nextStartTime;
            nextEvent = currentEvent;
            
        }
    }
    if (nextEvent == nullptr) {
        SF_OSAL_printf("No suitable event found\n");
    } else {
        SF_OSAL_printf("%c",nextEvent->taskName);
        
    }
    if(nextEvent->measurementCount == 0) {
        nextEvent->ensembleDelay = minNextTime;
    }
    nextEvent->lastMeasurementTime = minNextTime;
    nextEvent->measurementCount++;
    *p_nextEvent = nextEvent;
    *p_nextTime = minNextTime;
}
