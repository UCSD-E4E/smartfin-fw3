/**
 *  @file   scheduler.cpp
 *  @brief  Contains definitions for functions defined in @ref scheduler.hpp
 *  @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 *  @version 1
 */

#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif
#include "cli/conio.hpp"
#include "ensembles.hpp"
#include "scheduler.hpp"
#include "consts.hpp"


 /**
  * @brief Initializes ensembles within schedule table.
  *
  *
  * @param pDeployment contains schedule table, see @ref scheduler.hpp
  * @param startTime sets start time as given in RideTask.cpp
  *
  */
void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment,
                            system_tick_t startTime)
{
    uint32_t lastEndTime = 0;
    while (pDeployment->init)
    {
        pDeployment->deploymentStartTime = startTime;
        pDeployment->lastMeasurementTime = 0;
        pDeployment->measurementCount = 0;
        lastEndTime += pDeployment->maxDuration;

        pDeployment->init(pDeployment);
        pDeployment++;
    }
}



/**
 * @brief Retrieves the next scheduled event
 *
 * This function iterates through a schedule table to find the event that
 * should run next based on the current system time.
 * It ensures that no overlapping events are scheduled by checking with
 * @ref SCH_willOverlap().
 *
 * @see tests/gtest.cpp for intended behavior
 *
 * @param scheduleTable Pointer to the first element of an array of
 * DeploymentSchedule_t, which contains the scheduling information.
 * @param p_nextEvent Pointer to a pointer where the next event to be executed
 * will be stored.
 * @param p_nextTime Pointer to where the time for the next event will
 * be stored.
 *
 * @return Returns SUCCESS if a suitable event is found,
 * TASK_SEARCH_FAIL otherwise.
 */

uint32_t SCH_getNextEvent(DeploymentSchedule_t* scheduleTable,
                DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime,
                system_tick_t currentTime)
{

    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    


    // Iterate through each event in the schedule table.
    for (int idx = 0; scheduleTable[idx].measure; ++idx)
    {
        // Reference to current event in the schedule
        DeploymentSchedule_t& currentEvent = scheduleTable[idx];
        // Reset the next run time for the current event
        currentEvent.nextRunTime = UINT32_MAX;
        // Variable to store the calculated start time of the next event.
        uint32_t nextStartTime;
        // Calculate first start time after delay.
        uint32_t firstStartTime = currentEvent.deploymentStartTime +
            currentEvent.ensembleDelay;

        // check if first event
        if (currentTime <= firstStartTime)
        {
            if (firstStartTime < minNextTime &&
                                            !SCH_willOverlap(scheduleTable,
                                                idx, currentTime, firstStartTime))
            {
                minNextTime = firstStartTime;
                nextEvent = &currentEvent;

            }
            continue;
        }
        // Calculate intended count 
        uint32_t intendedCount = (currentTime - firstStartTime - 1) /
            currentEvent.ensembleInterval + 1;
        uint32_t lastInterval = firstStartTime + (intendedCount - 1) *
            currentEvent.ensembleInterval;
        uint32_t nextInterval = lastInterval + currentEvent.ensembleInterval;





        // check if measurement is late
        if ((currentEvent.lastMeasurementTime < lastInterval) ||
            ((currentEvent.lastMeasurementTime == firstStartTime) &&
                (firstStartTime < currentTime) &&
                (currentEvent.measurementCount == 0)))
        {

            if (currentTime + currentEvent.maxDuration > nextInterval)
            {
                nextStartTime = nextInterval;
            }
            else
            {
                nextStartTime = currentTime;
            }
        }
        else
        {
            nextStartTime = nextInterval;
        }

        if (!SCH_willOverlap(scheduleTable, idx, currentTime, nextStartTime))
        {
            currentEvent.nextRunTime = nextStartTime;
            if (nextStartTime < minNextTime)
            {
                nextEvent = &currentEvent;
                minNextTime = nextStartTime;
            }
        }

    }
    if (nextEvent == nullptr)
    {
        SF_OSAL_printf("No suitable event found"  __NL__);
        *p_nextEvent = nextEvent;
        *p_nextTime = 0;
        return TASK_SEARCH_FAIL;
    }
    else
    {


        // Sets ensemble variable lastMeasurementTime to next time it will run
        nextEvent->lastMeasurementTime = minNextTime;
        // Incraments ensemble variable lastMeasurementTime measurementCount
        nextEvent->measurementCount++;
        // Sets next event pointer to the next event
        *p_nextEvent = nextEvent;
        // Sets next time pointer to the next event time
        *p_nextTime = minNextTime;
        return SUCCESS;
    }
}



/**
 * @brief Determines if a task will overlap with other scheduled tasks.
 *
 * This function checks if the proposed start and end times of a task
 * overlap with any preceding tasks in the schedule table (defined earlier). 
 * It ensures that tasks are scheduled without conflicts.
 *
 * @param scheduleTable The table containing all scheduled tasks.
 * @param idx The index of the current task in the schedule table.
 * @param currentTime The current system time.
 * @param proposedEndTime The proposed end time of the current task.
 * @param nextStartTime The proposed start time of the current task.
 * @return True if there is an overlap with another task; false otherwise.
 */
bool SCH_willOverlap(DeploymentSchedule_t* scheduleTable, int idx,
                    system_tick_t currentTime, uint32_t nextStartTime)
{
    DeploymentSchedule_t& currentEvent = scheduleTable[idx];
    uint32_t proposedEndTime = nextStartTime + currentEvent.maxDuration;
    for (int other_idx = 0; other_idx < idx; ++other_idx)
    {

        DeploymentSchedule_t& otherEvent = scheduleTable[other_idx];
        // Calculate next start time for the other task
        uint32_t otherNextStartTime = otherEvent.nextRunTime;
        if (otherNextStartTime == UINT32_MAX)
            continue;
        // Calculate proposed end time for the other task
        uint32_t otherProposedEndTime = otherNextStartTime +
            otherEvent.maxDuration;
        // Check for overlap
        if (!((proposedEndTime <= otherNextStartTime) ||
            (otherProposedEndTime <= nextStartTime)))
        {
            return true; // Overlap detected
        }

    }
    return false;
}

