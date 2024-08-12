/**
 * @file charlie_scheduler.hpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __CONSOLODATED_SCHEDULER__HPP__
#define __CONSOLODATED_SCHEDULER__HPP__
#include "product.hpp"
#if SCHEDULER_VERSION == CONSOLODATED_VERSION

#include <stddef.h>
#include <cstdint>

#include "deploymentSchedule.hpp"
#include "abstractScheduler.hpp"
#include "cli/conio.hpp"
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif // TEST_VERSION


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
    //! how many times ensemble was scheduled
    std::uint32_t measurementCount;

    //! store the next time the task should run
    std::uint32_t nextRunTime;
    
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
    private:
    uint32_t tableSize;
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
    SCH_error_e getNextTask(DeploymentSchedule_t** p_next_task,
                    uint32_t* p_next_runtime,
                    uint32_t current_time);

    bool willOverlap(uint32_t i, system_tick_t currentTime, 
                                                uint32_t nextStartTime);
        
};
#endif //SCHEDULER_VERSION
#endif //__CONSOLODATED_SCHEDULER__HPP__