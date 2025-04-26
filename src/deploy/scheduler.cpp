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

#include "cli/flog.hpp"

#include <cstring>

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
    DeploymentSchedule_t *pDeployment = this->scheduleTable;
    std::uint32_t lastEndTime = 0;
    this->tableSize = 0;
    if (this->scheduleTable == nullptr)
    {
        return;
    }
    while (pDeployment->init)
    {

        memset(&(pDeployment->state), 0, sizeof(StateInformation));

        pDeployment->init(pDeployment);
        this->tableSize++;
        pDeployment++;
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

        // check if a delay exists
        std::uint32_t difference = currentTime - runTime;

        if (difference > 0)
        {
            runTime = currentTime;
        }

        if (difference >= (int)currentEvent.maxDelay)
        {
            //! TODO: send warning
        }
        std::uint32_t delay = difference > 0 ? difference : 0;

        // Finish time of task
        int expected_completion = runTime + currentEvent.maxDuration;

        bool canRun = true;
        // Iterate through all tasks of higher prioirty.
        for (int j = 0; (j < idx) && canRun; j++)
        {
            if ((int)scheduleTable[j].state.nextRunTime < expected_completion)
            {
                canRun = false;
            }
        }
        /*If there were no conflicts with higher priority tasks, we can set
        the next task to the task in question*/
        if (canRun)
        {
            *p_nextEvent = &currentEvent;
            *p_nextTime = runTime;
            currentEventState.nextRunTime = runTime + currentEvent.ensembleInterval;

            currentEventState.measurementCount++;

            /*
            If delay greater than 0, shift all future occurences of the task by
            delay amount to re-establish a constant frequency
            */
            if (delay > 0)
            {
                FLOG_AddError(FLOG_SCHEDULER_DELAY_EXCEEDED, currentEventState.measurementCount);
            }
            return SCHEDULER_SUCCESS;
        }
    }

    return TASK_SEARCH_FAIL;
}