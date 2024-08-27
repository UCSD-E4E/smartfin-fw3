/**
 * @file charlie_scheduler.hpp
 * @author Antara Chugh (antarachugh@g.ucla.edu), Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__
#include "product.hpp"
#include "deploymentSchedule.hpp"
#include "abstractScheduler.hpp"
#include "cli/conio.hpp"

#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#include <stddef.h>
#include <cstdint>


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
 * @brief Records and writes ensemble
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleProccess)(DeploymentSchedule_t* pDeployment);

/**
* @brief contains state information for each ensemble
*/
struct StateInformation
{
    std::uint32_t measurementCount;
    //! store the next time the task should run
    std::uint32_t nextRunTime;
    
};

#ifndef TEST_VERSION
#define NON_TEST_CONST const
#else
#define NON_TEST_CONST
#endif

struct DeploymentSchedule_
{

    NON_TEST_CONST EnsembleFunction measure;               //!< measurement function
    NON_TEST_CONST EnsembleInit init;                      //!< initialization function

    //! measurements before processing
    NON_TEST_CONST std::uint32_t measurementsToAccumulate; 
    
    //! time between ensembles
    NON_TEST_CONST std::uint32_t ensembleInterval;              
        
    //! max running time of measurement 
    NON_TEST_CONST std::uint32_t maxDuration;                 
    
    //! max delay before throwing flag and resetting
    NON_TEST_CONST std::uint32_t maxDelay; 
    
    //! task name of ensemble
    const char* NON_TEST_CONST taskName;                 
    
    //! state information
    StateInformation state;                
    
};

class Scheduler : public AbstractScheduler {
    private:
    //! schedule table size
    uint32_t tableSize;
    public:
    //! the schedule table
    DeploymentSchedule_t* scheduleTable;
    
    
    
    /**
     * @brief constructor for scheduler
     * 
     * @param schedule the schedule table
    */
    Scheduler(DeploymentSchedule_t* scheduler);

    /**
     * @brief Initializes ensemble variables
     * 
     * @param scheduleTable  the schedule table
     * @param startTime the start time of the deployment
     */
    void initializeScheduler();
    
    /**
     * @brief Determines the next task to run 
     *
     * @param scheduleTable The table containing all scheduled tasks.
     * @param idx The index of the current task in the schedule table.
     * @param currentTime The current system time.
     * @param nextStartTime The proposed start time of the current task.
     * @return True if there is an overlap with another task; false otherwise.
     */
    SCH_error_e getNextTask(DeploymentSchedule_t** p_nextEvent,
                    std::uint32_t* p_nextTime,
                    std::uint32_t currentTime);

};

#endif //__SCHEDULER_HPP__