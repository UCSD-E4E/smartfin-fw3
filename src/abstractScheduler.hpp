#ifndef __ABSTRACT_SCHEDULER_HPP__
#define __ABSTRACT_SCHEDULER_HPP__
#include <cstdint>
#include "deploymentSchedule.hpp"
#include <cstdint>


class AbstractScheduler {
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
    virtual SCH_error_e getNextTask(DeploymentSchedule_t** p_next_task,
                    std::uint32_t* p_next_runtime,
                    std::uint32_t current_time) = 0;
};

typedef enum error_
{
    SUCCESS,
    TASK_SEARCH_FAIL,
    FLOG_SCHEDULER_DELAY_EXCEEDED
}SCH_error_e;

#endif // __ABSTRACT_SCHEDULER_HPP__