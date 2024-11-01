/**
 * @file charlie_scheduler.hpp
 * @author Antara Chugh (antarachugh@g.ucla.edu), Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Header file for scheduler defined in @ref scheduler.cpp
 * @version 1
 */
#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__
#include "abstractScheduler.hpp"
#include "cli/conio.hpp"
#include "deploymentSchedule.hpp"
#include "product.hpp"

#include <cstdint>
#include <queue>
#include <stddef.h>

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#elif SF_PLATFORM == SF_PLATFORM_GCC
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
typedef void (*EnsembleFunction)(DeploymentSchedule_t *pDeployment);

/**
 * @brief Ensemble initialization function.
 *
 * This function is executed once when all of the
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleInit)(DeploymentSchedule_t *pDeployment);

/**
 * @brief Records and writes ensemble
 * @param pDeployment the schedule table
 */
typedef void (*EnsembleProccess)(DeploymentSchedule_t *pDeployment);

/**
 * @brief contains state information for each ensemble
 */
struct StateInformation
{
    std::uint32_t measurementCount;
    //! store the next time the task should run
    std::uint32_t nextRunTime;
    void *pData;
};

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#define TESTABLE_CONST const
#else
#define TESTABLE_CONST
#endif

/**
 * @brief contains ensemble definitions and info for scheduler
 *
 */
struct DeploymentSchedule_
{
    //! measurement function
    TESTABLE_CONST EnsembleFunction measure;
    //! initialization function
    TESTABLE_CONST EnsembleInit init;

    //! measurements before processing
    TESTABLE_CONST std::uint32_t measurementsToAccumulate;

    //! time between ensembles
    TESTABLE_CONST std::uint32_t ensembleInterval;

    //! max running time of measurement
    TESTABLE_CONST std::uint32_t maxDuration;

    //! max delay before throwing flag and resetting
    TESTABLE_CONST std::uint32_t maxDelay;

    //! task name of ensemble
    const char *TESTABLE_CONST taskName;

    //! state information
    StateInformation state;
};

/**
 * @brief contains schedule table for scheduler function to initialize and
 * get next tasks
 *
 */
class Scheduler : public AbstractScheduler
{
private:
    //! schedule table size
    uint32_t tableSize;

public:
    //! the schedule table
    DeploymentSchedule_t *scheduleTable;

    /**
     * @brief constructor for scheduler
     *
     * @param schedule the schedule table
     */
    Scheduler(DeploymentSchedule_t *scheduler);

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
    SCH_error_e getNextTask(DeploymentSchedule_t **p_nextEvent,
                            std::uint32_t *p_nextTime,
                            std::uint32_t currentTime);
#if SF_PLATFORM == SF_PLATFORM_GCC
    std::queue<std::tuple<const char *, std::uint32_t>> log;
#endif
};

#endif //__SCHEDULER_HPP__