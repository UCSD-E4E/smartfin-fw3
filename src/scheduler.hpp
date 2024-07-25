/**
 * @file scheduler.hpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __SCHEDULER__HPP_
#define __SCHEDULER__HPP_
#define CHARLIE_VERSION 0
#define ANTARA_VERSION 1
#ifndef SCHEDULER_VERSION
#define SCHEDULER_VERSION  CHARLIE_VERSION
#endif
#include <stddef.h>
#include <cstdint>
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif

#if SCHEDULER_VERSION == ANTARA_VERSION


#else


typedef enum error_
{
    SUCCESS,
    TASK_SEARCH_FAIL,
}SCH_error_e;

/**
 * @brief defines type for DeploymentSchedule_ struct below.

 */
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
/**
* @brief contains stat information for each ensemble
*/

struct StateInformation
{
    //! Total number of measurements to execute
    uint32_t nMeasurements;    
    //! last time measurement was scheduled 
    uint32_t lastMeasurementTime;
    //! when schedule was initialized   
    uint32_t deploymentStartTime;
    //! how many times ensemble was scheduled
    uint32_t measurementCount; 
    //! Buffer to store measurements temporarily  
    void* pData;
    
    //! store the next time the task should run
    uint32_t nextRunTime;
    //! store the zeroth run time
    uint32_t firstRunTime;
    
};
/**
 * @brief Records and writes ensemble
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleProccess)(DeploymentSchedule_t* pDeployment);
struct DeploymentSchedule_
{

    EnsembleFunction measure;               //!< measurement function
    EnsembleInit init;                      //!< initialization function
    //EnsembleProccess process;             //!< processing function

    uint32_t measurementsToAccumulate;      //!< measurements before processing
    uint32_t ensembleDelay;                 //!< delay after deployment start
    uint32_t ensembleInterval;              //!< time between ensembles
    //! store max running time of measurement        
    uint32_t maxDuration;               
    //! maximum delay before throwing flag and resetting
    uint32_t maxDelay;
    //! task name of ensemble
    const char*  taskName;
    //! state information
    StateInformation state;
    
    
};
/**
 * @brief Initializes ensemble variables
 * 
 * @param scheduleTable  the schedule table
 * @param startTime the start time of the deployment
 */
void SCH_initializeSchedule(DeploymentSchedule_t* scheduleTable,
                                system_tick_t startTime);

/**
 * @brief sets next scheduled task and time
 * 
 * @param scheduleTable the schedule table
 * @param p_nextEvent the next event to run
 * @param p_nextTime time next event will run
 * 
 * Handles all the scheduler logic, focusing on maininting task intervals with 
 * respect to the initial measurement. See expected behaivor in 
 * @ref tests/fixed_google_tests.cpp
 * @return outcome enum from
 */
uint32_t SCH_getNextEvent(DeploymentSchedule_t* scheduleTable,
            DeploymentSchedule_t** p_nextEvent,
            system_tick_t* p_nextTime,
            system_tick_t currentTime);

/**
 * @brief Determines if a task will overlap with other scheduled tasks.
 * 
 * This function checks if the proposed start and end times of a task
 * overlap with any other tasks in the schedule table. It ensures that 
 * tasks are scheduled without conflicts.
 *
 * @param scheduleTable The table containing all scheduled tasks.
 * @param idx The index of the current task in the schedule table.
 * @param currentTime The current system time.
 * @param nextStartTime The proposed start time of the current task.
 * @return True if there is an overlap with another task; false otherwise.
 */
bool SCH_willOverlap(DeploymentSchedule_t* scheduleTable, int i,
                    system_tick_t currentTime, uint32_t nextStartTime);
#endif            

       
#endif //__SCHEDULER__HPP_

