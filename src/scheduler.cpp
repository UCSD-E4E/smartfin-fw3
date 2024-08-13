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
#include <cstdint>
#include "cli/conio.hpp"
#include "ensembles.hpp"
#include "scheduler.hpp"
#include "consts.hpp"
#include <cli/flog.hpp>
#include <string.h>


#if SCHEDULER_VERSION == CONSOLODATED_VERSION
Scheduler::Scheduler(DeploymentSchedule_t schedule[])
    :  scheduleTable(schedule){}
 /**
  * @brief Initializes ensembles within schedule table.
  *
  */
void Scheduler::initializeScheduler()
{
    DeploymentSchedule_t* pDeployment = scheduleTable;
    uint32_t lastEndTime = 0;
    tableSize = 0;
    while (pDeployment->init)
    {
        tableSize++;
        memset(&(pDeployment->state), 0, 
            sizeof(StateInformation));
        
        pDeployment->init(pDeployment);
        pDeployment++;
    }
}



/**
 * @brief Retrieves the next schedu2led event
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

SCH_error_e Scheduler::getNextTask(DeploymentSchedule_t** p_nextEvent, 
                           std::uint32_t* p_nextTime,
                           std::uint32_t currentTime)
{
    uint32_t minNextTime = UINT32_MAX;
    DeploymentSchedule_t* nextEvent = nullptr;
    
    // Iterate through each event in the schedule table.
    
    for(int idx = tableSize - 1; idx>=0; idx--)
    {
        bool canSet = true;
        DeploymentSchedule_t& currentEvent = scheduleTable[idx];
        StateInformation& state = scheduleTable[idx].state;
        std::uint32_t runTime = state.nextRunTime;
        int delay = currentTime - runTime;
        if (delay > 0)
        {
            runTime = currentTime;
        }
        else
        {
            delay = 0;
        }
        if (delay >= currentEvent.maxDelay)
        {
            //send warning
        }
        int completion = runTime + currentEvent.maxDuration;
        int j = 0;
        while (j < idx && canSet)
        {
            if (scheduleTable[j].state.nextRunTime < completion)
            {
                canSet = false;
            }
            j++;
        }
        if (canSet)
        {
            *p_nextEvent = &currentEvent;
            state.nextRunTime = runTime + currentEvent.ensembleInterval;
            
            state.measurementCount++;
            *p_nextTime = runTime;
            if(delay>0){
                 FLOG_AddError(FLOG_SCHEDULER_DELAY_EXCEEDED,
                                            state.measurementCount);
                        SF_OSAL_printf("Task %s shifted at time %zu"  __NL__ ,
                                currentEvent.taskName,currentTime);
            }
            return SCHEDULER_SUCCESS;
        }
       

        
    }

   return TASK_SEARCH_FAIL;

}






#endif