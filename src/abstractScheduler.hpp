#ifndef __ABSTRACT_SCHEDULER_HPP__
#define __ABSTRACT_SCHEDULER_HPP__
#include "a_deploymentSchedule.hpp"
#include <cstdint>

typedef struct DeploymentSchedule_ DeploymentSchedule_t;

class AbstractScheduler {
    AbstractScheduler(DeploymentSchedule_t schedule[]){};
    /**
     * Creates and initializes the schedule
     */
    void initializeScheduler(void);



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
    virtual int getNextTask(const DeploymentSchedule_t* p_next_task,
                    std::uint32_t* p_next_runtime,
                    std::uint32_t current_time) = 0;
};
#endif // __ABSTRACT_SCHEDULER_HPP__