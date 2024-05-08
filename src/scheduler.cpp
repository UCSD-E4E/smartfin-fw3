#include "scheduler.hpp"

#ifndef TEST_SCHEDULER
#include "Particle.h"
#include "cli/conio.hpp"
#include <algorithm>
#else
#include <cstdint>
#include <chrono>
#include <stdio.h>
#include <cstdarg> 
int SF_OSAL_printf(const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs); // Use vprintf which sends formatted output to stdout
    va_end(vargs);
    return nBytes;
}
#endif // TEST_SCHEDULER

#define MIN_MEASUREMENT_SEPARATION 0
#include <cstdint>

void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment, system_tick_t startTime)
{
    while(pDeployment->init)
    {
        pDeployment->ensembleDelay = startTime;
        pDeployment->lastMeasurementTime = 0;
        pDeployment->measurementCount = 0;
        pDeployment->init(pDeployment);
        pDeployment++;
    }
}
void SCH_getNextEvent(DeploymentSchedule_t* scheduleTable, DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime)
{
    uint32_t currentTime = millis();
    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    SF_OSAL_printf("Current time: %u\n", currentTime);
    for (int i = 0; scheduleTable[i].measure; ++i) {
        DeploymentSchedule_t* currentEvent = &scheduleTable[i];

        // Calculate the next scheduled start time
        uint32_t scheduledStartTime = currentEvent->deploymentStartTime + currentEvent->ensembleDelay +
                                      (currentEvent->measurementCount * currentEvent->ensembleInterval);
        SF_OSAL_printf("Task %d: Scheduled Start Time: %u\n", i, scheduledStartTime);
        // If  past scheduled start time and it's time for the next interval
        uint32_t nextStartTime = scheduledStartTime;
        if (currentTime > scheduledStartTime) {
            nextStartTime += ((currentTime - scheduledStartTime) / currentEvent->ensembleInterval + 1) * currentEvent->ensembleInterval;
        }
        SF_OSAL_printf("Task %d: Next Start Time: %u\n", i, nextStartTime);
        // Calculate the expected end time if we started at the next start time
        uint32_t proposedEndTime = nextStartTime + currentEvent->meanDuration;
        SF_OSAL_printf("Task %d: Proposed End Time: %u\n", i, proposedEndTime);
        // Check if running this event would overlap with any subsequent events
        bool willOverlap = false;
        for (int j = 0; scheduleTable[j].measure; ++j) {
            if (i != j) {
                DeploymentSchedule_t* otherEvent = &scheduleTable[j];
                uint32_t otherScheduledStartTime = otherEvent->deploymentStartTime + otherEvent->ensembleDelay +
                                                   (otherEvent->measurementCount * otherEvent->ensembleInterval);
                uint32_t otherNextStartTime = otherScheduledStartTime;
                if (currentTime > otherScheduledStartTime) {
                    otherNextStartTime += ((currentTime - otherScheduledStartTime) / otherEvent->ensembleInterval + 1) * otherEvent->ensembleInterval;
                }

                if (proposedEndTime > otherNextStartTime && nextStartTime < otherNextStartTime) {
                    willOverlap = true;
                    SF_OSAL_printf("Task %d: Will overlap with Task %d\n", i, j);

                    break;
                }
            }
        }

        // Prioritize tasks that can be executed on time and ensure there will be no overlaps
        if (!willOverlap && nextStartTime >= currentTime && nextStartTime < minNextTime) {
            minNextTime = nextStartTime;
            nextEvent = currentEvent;
            SF_OSAL_printf("Task %d: Set as next event to execute at time %u\n", i, nextStartTime);
        }
    }
    if (nextEvent == nullptr) {
        SF_OSAL_printf("No suitable event found, all events overlap or are delayed\n");
    } else {
        SF_OSAL_printf("Next event to execute: Task %c at time %u\n", nextEvent->tas, minNextTime);
    }
    *p_nextEvent = nextEvent;
    *p_nextTime = minNextTime;
}
