#include "product.hpp"
#ifndef __ANTARA_SCHEDULER__HPP__
#define __ANTARA_SCHEDULER__HPP__
#if SCHEDULER_VERSION == ANTARA_VERSION

#include "abstractScheduler.hpp"
#include "deploymentSchedule.hpp"
struct DeploymentSchedule_
{
    // Ensemble properties
    int priority;
    void (*measure)();               //!< measurement function
    void (*init)();                  //!< initialization function
    // EnsembleProccess process;      //!< processing function
    uint32_t startDelay;             //!< delay after deployment start
    uint32_t ensembleInterval;       //!< time between ensembles
    // void* pData;                  //!< Buffer to store measurements temporarily
    uint32_t maxDuration;            //!< store max running time of measurement
    const char* taskName;            //!< task name of ensemble
    uint32_t maxDelay;

    // State Information
    uint32_t nextRunTime;
};

class Scheduler : public AbstractScheduler
{
    DeploymentSchedule_t *tasks;
    int numTasks;

public:
    Scheduler(DeploymentSchedule_t schedule[], int numTasks);
    int getNextTask(DeploymentSchedule_t **p_next_task,
                    std::uint32_t *p_next_runtime,
                    std::uint32_t current_time);
};
#endif
#endif //__ANTARA_SCHEDULER__HPP__
