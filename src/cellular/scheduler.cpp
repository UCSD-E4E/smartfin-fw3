#include "scheduler.hpp"
#include "cli/conio.hpp"

void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment, system_tick_t startTime)
{
    for(; pDeployment->init; pDeployment++)
    {
        pDeployment->startTime = startTime;
        pDeployment->lastExecuteTime = 0;
        pDeployment->measurementCount = 0;
        pDeployment->init(pDeployment);
    }
}

void SCH_getNextEvent(DeploymentSchedule_t* deploymentSchedule, DeploymentSchedule_t ** pEventPtr, size_t* pNextTime)
{
    size_t earliestExecution = 0;
    uint32_t earliestEvent = 0;
    size_t timeToCompare;
    uint32_t i = 0;

    *pEventPtr = 0;
    *pNextTime = 0;

    for(i = 0; deploymentSchedule[i].func; i++)
    {
        if(deploymentSchedule[i].lastExecuteTime == 0)
        {
            timeToCompare = deploymentSchedule[i].startTime + deploymentSchedule[i].ensembleDelay;
        }
        else
        {
            if(deploymentSchedule[i].ensembleInterval != UINT32_MAX)
            {
                timeToCompare = deploymentSchedule[i].lastExecuteTime + deploymentSchedule[i].ensembleInterval;
            }
            else
            {
                continue;
            }
        }
        if(deploymentSchedule[i].measurementCount > deploymentSchedule[i].nMeasurements)
        {
            continue;
        }
        // SF_OSAL_printf("Event %d nextExecute: %d\n", i, timeToCompare);

        if(earliestExecution == 0)
        {
            earliestExecution = timeToCompare;
        }

        if(timeToCompare < earliestExecution)
        {
            earliestExecution = timeToCompare;
            earliestEvent = i;
        }
    }
    *pNextTime = earliestExecution;
    *pEventPtr = deploymentSchedule + earliestEvent;
}