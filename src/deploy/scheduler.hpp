/**
 * @file deploymentSchedule.hpp
 * @author Charlie Kushlevsky (ckushelevsky@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2024-08-01
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __DEPLOYMENT_SCHDEULE_HPP__
#define __DEPLOYMENT_SCHDEULE_HPP__
#include <cstdint>
/**
 * @brief Scheduler Error Codes
 *
 * Encoding for AbstractScheduler::getNextTask return values
 */
typedef enum error_
{
    /**
     * @brief Scheduler successfully retrieved next task
     *
     */
    SCHEDULER_SUCCESS,
    /**
     * @brief Scheduler failed to find a new task.
     *
     * This is generally a fatal error, and cannot be solved at runtime
     *
     */
    TASK_SEARCH_FAIL,
    /**
     * @brief Scheduler retrieved the next task, but the task is delayed!
     *
     */
    SCHEDULER_DELAY_EXCEEDED
} SCH_error_e;

/**
 * @brief Deployment Schedule Type

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
    /**
     * @brief Measurement count
     *
     * This counter starts at 0 at the beginning of the deployment, and is
     * incremented once every time the measurement is run.  It is not reset
     * during the deployment
     */
    std::uint32_t measurementCount;
    //! store the next time the task should run
    /**
     * @brief Next time the task should run in ms since boot
     *
     * This is updated by the scheduler and should only be updated by
     * Scheduler::getNextTask once a task is selected to run.
     */
    std::uint32_t nextRunTime;
    /**
     * @brief Accumulated
     * 
     * 
     */
    std::uint64_t durationAccumulate;
    void *pData;
};

/**
 * @brief contains ensemble definitions and info for scheduler
 *
 */
struct DeploymentSchedule_
{
    //! measurement function
    EnsembleFunction measure;
    //! initialization function
    EnsembleInit init;

    //! measurements before processing
    std::uint32_t measurementsToAccumulate;

    //! time between ensembles
    std::uint32_t ensembleInterval;

    //! max running time of measurement
    std::uint32_t maxDuration;

    //! max delay before throwing flag and resetting
    std::uint32_t maxDelay;

    //! task name of ensemble
    const char *taskName;

    //! state information
    StateInformation state;
};

/**
 * @brief contains schedule table for scheduler function to initialize and
 * get next tasks
 *
 */
class Scheduler
{
private:
    /**
     * @brief Table size
     *
     */
    uint32_t tableSize;

public:
    /**
     * @brief Schedule table
     *
     */
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
};
#endif //__DEPLOYMENT_SCHDEULE_HPP__