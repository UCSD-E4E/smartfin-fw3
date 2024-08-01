/**
 * @file charlie_scheduler.hpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __SCHEDULER__HPP_
#define __SCHEDULER__HPP_

#include <stddef.h>
#include <cstdint>
#include "product.hpp"
#include "deploymentSchedule.hpp"
#include "abstractScheduler.hpp"
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif


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
class Scheduler : public AbstractScheduler {
    public:
    DeploymentSchedule_t* scheduleTable;
    

    Scheduler(DeploymentSchedule_t* scheduler);

    /**
     * @brief Initializes ensemble variables
     * 
     * @param scheduleTable  the schedule table
     * @param startTime the start time of the deployment
     */
    void initializeScheduler();
    
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
    int getNextTask(const DeploymentSchedule_t* p_next_task,
                    uint32_t* p_next_runtime,
                    uint32_t current_time);

    bool willOverlap(uint32_t i, system_tick_t currentTime, 
                                                uint32_t nextStartTime);
        
};

typedef enum error_
{
    SUCCESS,
    TASK_SEARCH_FAIL,
}SCH_error_e;








         

       
#endif //__SCHEDULER__HPP_

