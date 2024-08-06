#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif
#include "cli/conio.hpp"

#include "ensembles.hpp"
#include "antara_scheduler.hpp"
#include "consts.hpp"

#include <cli/flog.hpp>
#include <string.h>

#if SCHEDULER_VERSION == ANTARA_VERSION
Scheduler::Scheduler(DeploymentSchedule_t schedule[], int numTasks)
{
    
    tasks = schedule;
    
    this->numTasks=numTasks;
    for (int i = 0; i<numTasks; i++)
    {
        if (i == 0)
        {
            tasks[i].startDelay = 0;
        }
        else
        {
            tasks[i].nextRunTime=tasks[i - 1].nextRunTime + tasks[i - 1].maxDuration;
            std::cout<<tasks[i].nextRunTime<< std::endl;
        }
    }
}
void Scheduler::initializeScheduler()
{}
int Scheduler::getNextTask(DeploymentSchedule_t **p_next_task, std::uint32_t *p_next_runtime, std::uint32_t current_time)
{
    int i = numTasks - 1;

    for(; i>=0; i--)
    {
        bool canSet = true;
        std::uint32_t runTime = tasks[i].nextRunTime;
        int delay = current_time - runTime;
        if (delay > 0)
        {
            runTime = current_time;
        }
        else
        {
            delay = 0;
        }
        if (delay == tasks[i].maxDelay)
        {
            //send warning
        }
        int completion = runTime + tasks[i].maxDuration;
        int j = 0;
        while (j < i && canSet)
        {
            if (tasks[j].nextRunTime< completion)
            {
                canSet = false;
            }
            j++;
        }
        if (canSet)
        {
            *p_next_task = &(tasks[i]);
            tasks[i].nextRunTime = runTime + tasks[i].ensembleInterval;
            tasks[i].nextRunTime += delay + tasks[i].ensembleInterval;
            *p_next_runtime = runTime;
            return runTime;
        }

        
    }

   return -1;
}
#endif