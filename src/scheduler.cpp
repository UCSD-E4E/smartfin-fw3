#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif
#include "cli/conio.hpp"
#include "ensembles.hpp"
#include "scheduler.hpp"
#include "consts.hpp"
void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment,
                            system_tick_t startTime)
{
    uint32_t lastEndTime = 0;
    while (pDeployment->init)
    {
        pDeployment->deploymentStartTime = startTime;
        pDeployment->ensembleDelay = lastEndTime;
        pDeployment->lastMeasurementTime = 0;
        pDeployment->measurementCount = 0;
        lastEndTime += pDeployment->maxDuration;
        pDeployment->init(pDeployment);
        pDeployment++;
    }
}

void SCH_getNextEvent(DeploymentSchedule_t* scheduleTable,
                DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime)
{

    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    system_tick_t currentTime = millis();

    for (int i = 0; scheduleTable[i].measure; ++i)
    {

        DeploymentSchedule_t* currentEvent = &scheduleTable[i];
        if (currentEvent->measurementCount == 0)
        {

            if (!i || !SCH_willOverlap(scheduleTable, i, currentTime, currentTime +
                currentEvent->maxDuration, currentTime))
            {
                nextEvent = currentEvent;
                minNextTime = currentTime;
                currentEvent->ensembleDelay = currentTime -
                    currentEvent->deploymentStartTime;
                break;
            }
        }
        bool on_interval = true;
        // Calculate the next scheduled start time
        uint32_t nextStartTime = currentEvent->deploymentStartTime +
            currentEvent->ensembleDelay +
            (currentEvent->measurementCount *
            currentEvent->ensembleInterval);
        // If  past scheduled start time and it's time for the next interval
        if (currentTime > nextStartTime)
        {
            nextStartTime = currentTime;
            on_interval = false;
        }
        // Calculate the expected end time if we started at the next start time
        uint32_t proposedEndTime = nextStartTime + currentEvent->maxDuration;
        // Check if running this event would overlap with any subsequent events
        bool willOverlap = SCH_willOverlap(scheduleTable, i, currentTime,
                                            proposedEndTime, nextStartTime);
        if (willOverlap && !on_interval)
        {
            continue;
        }
        // Check for overlap with next time the same ensemble is scheduled
        uint32_t nextScheduled = currentEvent->deploymentStartTime +
            currentEvent->ensembleDelay;
        uint32_t intendedMeasureCount = (nextStartTime - nextScheduled) /
            currentEvent->ensembleInterval + 1;
        nextScheduled += intendedMeasureCount * currentEvent->ensembleInterval;
        if (nextScheduled < proposedEndTime)
        {
            //update measurementCount if past measurement was skipped
            currentEvent->measurementCount = intendedMeasureCount;
            nextStartTime = nextScheduled;
        }
        // Prioritize tasks that can be executed on time and ensure no overlaps
        if ((nextStartTime >= currentTime) && (nextStartTime < minNextTime))
        {
            minNextTime = nextStartTime;
            nextEvent = currentEvent;
        }
    }
    if (nextEvent == nullptr)
    {
        SF_OSAL_printf("No suitable event found"  __NL__);
    }
    else
    {
        SF_OSAL_printf("%c", nextEvent->taskName);
        if (nextEvent->measurementCount == 0 && nextEvent->ensembleDelay == 0)
        {
            nextEvent->ensembleDelay = minNextTime - nextEvent->deploymentStartTime;
        }
        nextEvent->lastMeasurementTime = minNextTime;
        nextEvent->measurementCount++;
        *p_nextEvent = nextEvent;
        *p_nextTime = minNextTime;
    }

}

bool SCH_willOverlap(DeploymentSchedule_t* scheduleTable, int i,
                    system_tick_t currentTime, uint32_t proposedEndTime,
                    uint32_t nextStartTime)
{
    for (int j = 0; scheduleTable[j].measure; ++j)
    {
        if (i != j)
        {
            DeploymentSchedule_t* otherEvent = &scheduleTable[j];
            uint32_t otherNextStartTime = otherEvent->deploymentStartTime +
                otherEvent->ensembleDelay +
                (otherEvent->measurementCount *
                otherEvent->ensembleInterval);
            if (currentTime > otherNextStartTime)
            {
                otherNextStartTime += ((currentTime - otherNextStartTime) /
                otherEvent->ensembleInterval + 1) *
                    otherEvent->ensembleInterval;
            }
            uint32_t otherProposedEndTime = otherNextStartTime +
                otherEvent->maxDuration;
            // Check for overlap
            if (!((proposedEndTime <= otherNextStartTime) ||
                (otherProposedEndTime <= nextStartTime)))
            {
                return true;
            }


        }
    }
    return false;
}

