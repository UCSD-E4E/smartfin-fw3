#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__
#include <stdint.h>
#include <stddef.h>
#include <Particle.h>
typedef struct DeploymentSchedule_ DeploymentSchedule_t;

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
 */
typedef void (*EnsembleFunction)(DeploymentSchedule_t* pDeployment); 

/**
 * @brief Ensemble initialization function.
 * 
 * This function is executed once when all of the 
 * 
 */
typedef void (*EnsembleInit)(DeploymentSchedule_t* pDeployment);

struct DeploymentSchedule_
{
    EnsembleFunction func;
    EnsembleInit init;
    size_t measurementsToAccumulate;
    uint32_t ensembleDelay;
    /**
     * @brief Interval between ensembles in ms
     * 
     * Set to UINT32_MAX to execute only once.
     * 
     */
    uint32_t ensembleInterval;
    /**
     * @brief Total number of measurements to execute
     * 
     * Set to UINT32_MAX to execute forever.
     * 
     */
    uint32_t nMeasurements;
    size_t lastExecuteTime;
    system_tick_t startTime;
    uint32_t measurementCount;

    void* pData;
};

void SCH_initializeSchedule(DeploymentSchedule_t* pDeployment, system_tick_t startTime);
void SCH_getNextEvent(DeploymentSchedule_t* deploymentSchedule, DeploymentSchedule_t ** pEventPtr, size_t* pNextTime);

#endif