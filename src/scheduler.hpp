/**
 * @file scheduler.hpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __SCHEDULER__HPP_
#define __SCHEDULER__HPP_

#include <stddef.h>
#include <cstdint>
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif

/** @todo Add documentation*/
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
struct DeploymentSchedule_
{

    EnsembleFunction measure;               // measurement function
    EnsembleInit init;                      // initialization function
    //EnsembleProccess process;             // processing function

    uint32_t measurementsToAccumulate;      // measurements before processing
    uint32_t ensembleDelay;                 // delay after deployment start
    uint32_t ensembleInterval;              // time between ensembles

    // State information
    uint32_t nMeasurements;         // Total number of measurements to execute
    uint32_t lastMeasurementTime;
    uint32_t deploymentStartTime;
    uint32_t measurementCount;
    void* pData;                    // Buffer to store measurements temporarily
    uint32_t maxDuration;       //store max running time of measurement
    char taskName;
};
/**
 * @brief Initializes ensemble variables within the DeploymentSchedule_t
 * 
 * @param ScheduleTable_t 
 * @param startTime 
 */
void SCH_initializeSchedule(DeploymentSchedule_t* ScheduleTable_t,
                                system_tick_t startTime);

/**
 * @brief sets p_nextEvent and p_nextTime to next scheduled task and time
 * 
 * @param ScheduleTable_t 
 * @param p_nextEvent 
 * @param p_nextTime 
 * @see 
 * @details
 * Handles all the scheduler logic, focusing on maininting task intervals with 
 * respect to the initial measurement. See expected behaivor in tests/gtests.cpp
 */
void SCH_getNextEvent(DeploymentSchedule_t* ScheduleTable_t,
            DeploymentSchedule_t** p_nextEvent,
            system_tick_t* p_nextTime);
bool SCH_willOverlap(DeploymentSchedule_t* scheduleTable, int i,
                    system_tick_t currentTime, uint32_t proposedEndTime,
                    uint32_t nextStartTime);
#endif //__SCHEDULER__HPP_

