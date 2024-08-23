/**
 *  @file   scheduler.cpp
 *  @brief  Contains definitions for functions defined in @ref scheduler.hpp
 *  @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 *  @version 1
 */
#include "scheduler.hpp"
#include "cli/conio.hpp"
#include "ensembles.hpp"
#include "consts.hpp"
#include "cli/flog.hpp"

#include <cstdint>
#include <string.h>

#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif

/**
 * @brief constructor for scheduler
 * 
 * @param schedule the schedule table
*/
Scheduler::Scheduler(DeploymentSchedule_t schedule[])
    : scheduleTable(schedule) {}
/**
 * @brief Initializes ensembles within schedule table.
 *
 */
void Scheduler::initializeScheduler()
{
    DeploymentSchedule_t* pDeployment = this->scheduleTable;
    std::uint32_t lastEndTime = 0;
    tableSize = 0;
    if (this->scheduleTable == nullptr)
    {
        return;
    }
    while (pDeployment->init)
    {
        
        memset(&(pDeployment->state), 0,
            sizeof(StateInformation));

        pDeployment->init(pDeployment);
        tableSize++;
        pDeployment++;
    }
}



/**
 * @brief Retrieves the next schedu2led event
 *
 * This function iterates through a schedule table to find the event that
 * should run next based on the current system time.
 * 
 *
 * @see tests/googletests.cpp for intended behavior
 *
 * 
 * @param p_nextEvent Pointer to a pointer where the next event to be executed
 * will be stored.
 * @param p_nextTime Pointer to where the time for the next event will
 * be stored.
 *
 * @param currentTime The current system time
 * @return Returns SUCCESS if a suitable event is found,
 * TASK_SEARCH_FAIL otherwise.
 * 
 */

SCH_error_e Scheduler::getNextTask(DeploymentSchedule_t** p_nextEvent,
                           std::uint32_t* p_nextTime,
                           std::uint32_t currentTime)
{
    std::uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;

    /*Iterate through each event in the schedule table in reverse order, with 
    the goal of determining if a lower prioirty task can run before a higher 
    priority task must run. */
    for (int idx = tableSize - 1; idx >= 0; idx--)
    {
        bool canSet = true;
        DeploymentSchedule_t& currentEvent = scheduleTable[idx];
        StateInformation& currentEventState = scheduleTable[idx].state;
        std::uint32_t runTime = currentEventState.nextRunTime;
        
        /* check if a delay exists. Current time comes from clock, which starts
        at time 0 and is increasing. if currentTime is ahead of runTime, there 
        is a delay. We will try to run the task immediately. */
        int delay = currentTime - runTime;
        
        if (delay > 0)
        {
            runTime = currentTime;
        }

        /*if delay is negative, clock has not reached the proposed runTime of a 
        task and there is no delay. Set delay to 0*/

        if(delay<=0)
        {
            delay = 0;
        }

        if (delay >= currentEvent.maxDelay)
        {
            //! TODO: send warning
        }
        //Finish time of a task is when we want to run the task plus its duration
        int expected_completion = runTime + currentEvent.maxDuration;

        /*Iterate through all tasks of higher prioirty. If the time the task 
         will finish overlaps with any task of higher prioirty, we cannot run this
         task next. The highest prioirty task will not be checked against other 
         tasks, so by default that will run*/
        int j = 0;
        for (int j = 0; (j < idx) && canSet; j++)
        {
            if (scheduleTable[j].state.nextRunTime < expected_completion)
            {
                canSet = false;
            }
        }
        /*If there were no conflicts with higher prioirity tasks, we can set the 
        next task to the task in question*/
        if (canSet)
        {
            *p_nextEvent = &currentEvent;
            *p_nextTime = runTime;
            currentEventState.nextRunTime = runTime +
                currentEvent.ensembleInterval;

            currentEventState.measurementCount++;
            
            /*If delay was greater than 0, we want to shift all future occurences
             of the task by delay amount to re-establish a constant frequency*/
            if (delay > 0)
            {
                FLOG_AddError(FLOG_SCHEDULER_DELAY_EXCEEDED,
                                        currentEventState.measurementCount);
            #ifdef TEST_VERSION
                std::ofstream logfile;
                if (currentTime == 0)
                {
                    logfile = std::ofstream("scheduler.log");

                }
                else
                {
                    logfile = std::ofstream("scheduler.log", std::ios::app);
                }
                if (logfile.is_open())
                {
                    logfile << currentEvent.taskName
                        << "|"
                        << currentEventState.measurementCount
                        << "\n";
                    logfile.close();
                }
            #endif

            }
            return SCHEDULER_SUCCESS;
        }



    }

    return TASK_SEARCH_FAIL;

}




