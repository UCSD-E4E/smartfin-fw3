#ifndef __SCHEDULER__HPP_
#define __SCHEDULER__HPP_

#include <stdint.h>
#include "Particle.h"
#include "cli/conio.hpp"


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

/**
 * @brief Records and writes ensemble
 */
typedef void (*EnsembleProccess)(DeploymentSchedule_t* pDeployment);
struct DeploymentSchedule_{
    
    EnsembleFunction measure;               // measurement function
    EnsembleInit init;                      // initialization function
    //EnsembleProccess process;             // processing function

    uint32_t measurementsToAccumulate;      // measurements before processing
    uint32_t ensembleDelay;                 // delay after deployment start
    uint32_t ensembleInterval;              // time between ensembles

    // State information
    uint32_t nMeasurements;              // Total number of measurements to execute
    uint32_t lastMeasurementTime;     
    uint32_t deploymentStartTime;      
    uint32_t measurementCount;                
    void* pData;                        // Buffer to store measurements temporarily
    uint32_t meanDuration = 0;          //store mean running time of measurement
};

void SCH_initializeSchedule(DeploymentSchedule_t* ScheduleTable_t, system_tick_t startTime);
void SCH_getNextEvent(DeploymentSchedule_t* ScheduleTable_t, DeploymentSchedule_t** p_nextEvent, system_tick_t* p_nextTime);
#endif //__SCHEDULER__HPP_