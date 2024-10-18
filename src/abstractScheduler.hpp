/**
 * @file abstractScheduler.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Abstract Scheduler definitions
 * @version 0.1
 * @date 2024-07-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __ABSTRACT_SCHEDULER_HPP__
#define __ABSTRACT_SCHEDULER_HPP__
#include "deploymentSchedule.hpp"

#include <cli/flog.hpp>
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
 * @brief Abstract Scheduler base class
 *
 * This base class provides the minimum required behavior for task schedulers
 */
class AbstractScheduler
{
public:
    /**
     * Creates and initializes the schedule
     */
    virtual void initializeScheduler(void) = 0;

    /**
     * Computes the time to the next task.
     *
     * @param p_next_task Pointer to a variable which will be populated with the
     * address to the next task to run
     * @param p_next_runtime Pointer to a variable which will be populated with
     * the time in milliseconds since boot at which the next task MUST run
     * @param current_time The current time in milliseconds since boot
     * @return 0 if successful, otherwise error code to be defined by
     * implementation
     */
    virtual SCH_error_e getNextTask(DeploymentSchedule_t **p_next_task,
                                    std::uint32_t *p_next_runtime,
                                    std::uint32_t current_time) = 0;
};

#endif // __ABSTRACT_SCHEDULER_HPP__