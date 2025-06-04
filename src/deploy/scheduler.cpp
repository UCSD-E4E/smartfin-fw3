/**
 * @file scheduler.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2025-04-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "deploy/scheduler.hpp"

#include "Particle.h"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"

#include <cstring>

// #define SCHEDULER_DEBUG

/**
 * @brief constructor for scheduler
 *
 * @param schedule the schedule table
 */
Scheduler::Scheduler(DeploymentSchedule_t schedule[]) : scheduleTable(schedule)
{
}

/**
 * @brief Initializes ensembles within schedule table.
 *
 */
void Scheduler::initializeScheduler()
{
    this->tableSize = 0;
    if (this->scheduleTable == nullptr)
    {
        return;
    }
    for (DeploymentSchedule_t *pDeployment = this->scheduleTable; pDeployment->init; pDeployment++)
    {
        memset(&(pDeployment->state), 0, sizeof(StateInformation));
        pDeployment->state.nextRunTime = millis();
        pDeployment->init(pDeployment);
        this->tableSize++;
    }
}

/**
 * @brief Retrieves the next scheduled event
 *
 * This function iterates through a schedule table to find the event that
 * should run next based on the current system time. It also modifies the
 * current state table to assume that the event will be run.
 *
 *
 *
 *
 * @param p_nextEvent Pointer to a pointer where the next event to be executed
 * will be stored.
 * @param p_nextTime Pointer to where the time for the next event will
 * be stored.
 *
 * @param currentTime The current system time, defined as time since system
 * boot
 * @return Returns SUCCESS if a suitable event is found,
 * TASK_SEARCH_FAIL otherwise.
 *
 */

SCH_error_e Scheduler::getNextTask(DeploymentSchedule_t **p_nextEvent,
                                   std::uint32_t *p_nextTime,
                                   std::uint32_t currentTime)
{

    // Iterate through each event in the schedule table in reverse order,
    for (int idx = this->tableSize - 1; idx >= 0; idx--)
    {

        DeploymentSchedule_t &currentEvent = scheduleTable[idx];
        StateInformation &currentEventState = scheduleTable[idx].state;
        std::uint32_t runTime = currentEventState.nextRunTime;
#ifdef SCHEDULER_DEBUG
        SF_OSAL_printf("Ensemble %s: {'nextRunTime': %u, 'currentTime': %u, "
                       "'maxDelay': %u, 'interval': %u}" __NL__,
                       currentEvent.taskName,
                       currentEvent.state.nextRunTime,
                       currentTime,
                       currentEvent.maxDelay,
                       currentEvent.ensembleInterval);
#endif
        // if runTime is infinite, skip
        if (runTime == UINT32_MAX)
        {
#ifdef SCHEDULER_DEBUG
            SF_OSAL_printf("Skipping - runTime is inf" __NL__);
#endif
            continue;
        }

        // check if a delay exists
        std::int64_t difference = currentTime - runTime;

        if (difference > 0)
        {
            // we are behind!
            runTime = currentTime;
        }

        if (difference >= currentEvent.maxDelay)
        {
            //! TODO: send warning
        }
        std::uint32_t delay = difference > 0 ? difference : 0;

        // Finish time of task
        uint32_t expected_completion = runTime + currentEvent.maxDuration;

        bool canRun = true;
        // Iterate through all tasks of higher prioirty.
        for (int j = 0; (j < idx) && canRun; j++)
        {
            if (scheduleTable[j].state.nextRunTime < expected_completion)
            {
#ifdef SCHEDULER_DEBUG
                SF_OSAL_printf("This can't run because %s needs to run at %u" __NL__,
                               scheduleTable[j].taskName,
                               scheduleTable[j].state.nextRunTime);
#endif
                canRun = false;
            }
        }
        /*If there were no conflicts with higher priority tasks, we can set
        the next task to the task in question*/
        if (canRun)
        {
            *p_nextEvent = &currentEvent;
            if (currentTime > currentEvent.state.nextRunTime)
            {
                *p_nextTime = currentTime;
            }
            else
            {
                *p_nextTime = currentEvent.state.nextRunTime;
            }
            if (currentEvent.ensembleInterval == UINT32_MAX)
            {
                currentEventState.nextRunTime = UINT32_MAX;
            }
            else
            {
                currentEventState.nextRunTime = *p_nextTime + currentEvent.ensembleInterval;
            }

            currentEventState.measurementCount++;

            /*
            If delay greater than 0, shift all future occurences of the task by
            delay amount to re-establish a constant frequency
            */
            if (delay > 0)
            {
#ifdef SCHEDULER_DEBUG
                SF_OSAL_printf("Delay exceeded: %" PRIu32 __NL__, delay);
#endif
                FLOG_AddError(FLOG_SCHEDULER_DELAY_EXCEEDED,
                              currentEventState.measurementCount | (idx << 24));
            }
            return SCHEDULER_SUCCESS;
        }
    }

    return TASK_SEARCH_FAIL;
}