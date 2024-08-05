#include "product.hpp"
#ifndef __ANTARA_SCHEDULER__HPP__
#define __ANTARA_SCHEDULER__HPP__
#if SCHEDULER_VERSION == ANTARA_VERSION

#include "abstractScheduler.hpp"
#include "deploymentSchedule.hpp"
/**
 * @brief Ensemble function.
 *
 * This function executes once to update the ensemble state.  This should update
 * the accumulators with a new measurement.  If the accumulators have
 * accumulated the proper amount of data, this function should then record the
 * proper data.
 *
 * Essentially, this will be called every ensembleInterval ms after
 * ensembleDelay ms from the start of the deployment.
 * 
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleFunction)(DeploymentSchedule_t* pDeployment);
/**
 * @brief Ensemble initialization function.
 *
 * This function is executed once when all of the
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleInit)(DeploymentSchedule_t* pDeployment);

struct DeploymentSchedule_
{
    // Ensemble properties
    //int priority;
    EnsembleFunction measure;               //!< measurement function
    EnsembleInit init;                      //!< initialization function
    uint32_t startDelay;             //!< delay after deployment start
    uint32_t ensembleInterval;       //!< time between ensembles
    // void* pData;                  //!< Buffer to store measurements temporarily
    uint32_t maxDuration;            //!< store max running time of measurement
    const char* taskName;            //!< task name of ensemble
    uint32_t maxDelay;

    // State Information
    uint32_t nextRunTime;
    uint32_t measurementCount;
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
    void initializeScheduler();
};
#endif
#endif //__ANTARA_SCHEDULER__HPP__
