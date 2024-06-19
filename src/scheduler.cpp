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
        pDeployment->ensembleDelay = lastEndTime;
        pDeployment->lastMeasurementTime = 0;
        pDeployment->measurementCount = 0;
        lastEndTime += pDeployment->maxDuration;

        pDeployment->init(pDeployment);
        pDeployment++;
    }
}
/**
 * @brief Handles scheduling logic
 *
 * @param scheduleTable The table containing all scheduled tasks
 * @param p_nextEvent Pointer to the next event to be scheduled
 * @param p_nextTime Pointer to the time of the next scheduled event
 * 
 * @see tests/gtest.cpp for intended behavior
 * @note For scheduler documentation, the terms "task" and "ensemble" will
 *  be interchangeably used
 * 
 * This scheduler minimizes delays in tasks with respect to their original 
 *  start time. 
 * For a task that runs every X ms and takes Y ms to run where X > Y:
 * - If the start of that task is Z ms late, where X > (Y + Z), then
 *  it will run at X + Z ms.
 * - The goal of this scheduler is to ensure that the next time this task runs
 *  is at 2X ms.
 * - If the delay is greater, such that X < (Y + Z), the scheduler will skip
 *  this iteration of the task and run it at the next interval of X from the
 *  start time.
 * 
 * The scheduling is done by taking a predetermined maxDuration and using
 *  that worst-case scenario to check for overlaps with other tasks. If it
 *  can run without overlaps, it will be run; otherwise, the task will be
 *  skipped.
 * 
 * In cases where all tasks cause an overlap, the first ensemble in the
 *  schedule table will be run. If multiple tasks do not overlap with another
 *  task, the earliest ensemble that does not overlap will be run.
 */
int SCH_getNextEvent(DeploymentSchedule_t* scheduleTable,
                DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime)
{

    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    system_tick_t currentTime = millis();

    for (int idx = 0; scheduleTable[idx].measure; ++idx)
    {

        DeploymentSchedule_t& currentEvent = scheduleTable[idx];
        if (currentEvent.measurementCount == 0)
        {
            //! Check for no overlap or if it's the first task
            if (!idx || !SCH_willOverlap(scheduleTable, idx, currentTime,
                currentTime + currentEvent.maxDuration, currentTime))
            {
                nextEvent = &currentEvent;
                minNextTime = currentTime;
                currentEvent.ensembleDelay = currentTime -
                    currentEvent.deploymentStartTime;
                break;
            }
        }
        bool on_interval = true;
        //! Calculate the next scheduled start time
        uint32_t nextStartTime = currentEvent.deploymentStartTime +
            currentEvent.ensembleDelay +
            (currentEvent.measurementCount *
            currentEvent.ensembleInterval);
        //! If  past scheduled start time 
        if (currentTime > nextStartTime)
        {
            nextStartTime = currentTime;
            on_interval = false;
        }
        //! Calculate the expected end time if we started at the next start time
        uint32_t proposedEndTime = nextStartTime + currentEvent.maxDuration;
        //! Check if running this event would overlap with any subsequent events
        bool willOverlap = SCH_willOverlap(scheduleTable, idx, currentTime,
                                            proposedEndTime, nextStartTime);
        if (willOverlap && !on_interval)
        {
            continue;
        }
        //! Check for overlap with next time the same ensemble is scheduled
        uint32_t nextScheduled = currentEvent.deploymentStartTime +
            currentEvent.ensembleDelay;
        //! Update intendedMeasureCount
        uint32_t intendedMeasureCount = (nextStartTime - nextScheduled) /
            currentEvent.ensembleInterval + 1;
        nextScheduled += intendedMeasureCount * currentEvent.ensembleInterval;
        //! if past measurement was skipped
        if (nextScheduled < proposedEndTime)
        {
            //update measurementCount 
            currentEvent.measurementCount = intendedMeasureCount;
            nextStartTime = nextScheduled;
        }
        //! Prioritize tasks that can execute on time and ensure no overlaps
        if ((nextStartTime >= currentTime) && (nextStartTime < minNextTime))
        {
            minNextTime = nextStartTime;
            nextEvent = &currentEvent;
        }
    }
    if (nextEvent == nullptr)
    {
        
        SF_OSAL_printf("No suitable event found"  __NL__);
        return TASK_SEARCH_FAIL;
    }
    else
    {
        SF_OSAL_printf("%c", nextEvent->taskName);
        if (nextEvent->measurementCount == 0 && nextEvent->ensembleDelay == 0)
        {
            nextEvent->ensembleDelay = minNextTime -
                nextEvent->deploymentStartTime;
        }
        //! Sets ensemble variable lastMeasurementTime to next time it will run
        nextEvent->lastMeasurementTime = minNextTime;
        //! Incraments ensemble variable lastMeasurementTime measurementCount
        nextEvent->measurementCount++;
        //! Sets next event pointer to the next event
        *p_nextEvent = nextEvent;
        //! Sets next time pointer to the next event time
        *p_nextTime = minNextTime;
        return SUCCESS;
    }

}

/**
 * @brief Determines if a task will overlap with other scheduled tasks.
 * 
 * This function checks if the proposed start and end times of a task
 * overlap with any other tasks in the schedule table. It ensures that 
 * tasks are scheduled without conflicts.
 *
 * @param scheduleTable The table containing all scheduled tasks.
 * @param idx The index of the current task in the schedule table.
 * @param currentTime The current system time.
 * @param proposedEndTime The proposed end time of the current task.
 * @param nextStartTime The proposed start time of the current task.
 * @return True if there is an overlap with another task; false otherwise.
 */
bool SCH_willOverlap(DeploymentSchedule_t* scheduleTable, int idx,
                    system_tick_t currentTime, uint32_t proposedEndTime,
                    uint32_t nextStartTime)
{
    for (int other_idx = 0; scheduleTable[other_idx].measure; ++other_idx)
    {
        //! Skip  current task 
        if (idx != other_idx)
        {
            DeploymentSchedule_t& otherEvent = scheduleTable[other_idx];
            //! Calculate next start time for the other task
            uint32_t otherNextStartTime = otherEvent.deploymentStartTime +
                otherEvent.ensembleDelay +
                (otherEvent.measurementCount *
                otherEvent.ensembleInterval);
            //! Adjust next start time if current time is beyond it
            if (currentTime > otherNextStartTime)
            {
                otherNextStartTime += ((currentTime - otherNextStartTime) /
                otherEvent.ensembleInterval + 1) *
                    otherEvent.ensembleInterval;
            }
            // Calculate proposed end time for the other task
            uint32_t otherProposedEndTime = otherNextStartTime +
                otherEvent.maxDuration;
            //! Check for overlap
            if (!((proposedEndTime <= otherNextStartTime) ||
                (otherProposedEndTime <= nextStartTime)))
            {
                return true; // Overlap detected
            }


        }
    }
    return false;
}

