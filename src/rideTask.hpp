/**
 * @file rideTask.hpp
 * @brief Header file for deployment state
 * 
 */
#ifndef __RIDE_HPP__
#define __RIDE_HPP__


#include "task.hpp"
#include "scheduler.hpp"
#include "product.hpp"
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "../tests/scheduler_test_system.hpp"
#endif


/**
 * @class RideTask
 * @brief Manages deployment of measurements and recording
 */
class RideTask : public Task
{
public:
    RideTask(DeploymentSchedule_ deploymentSchedule[], int num);
    /**
     * @brief initialize ride task
     * Sets LEDs  and initializes schedule
    */
    
    /**
     * @brief runs tasks given by scheduler
     * 
     * @return the next state to change to.
     */
    void init();
    STATES_e run(void);
    /**
     * @brief exits ride state
     */
    void exit(void);
    void printRunTimes();
private:
    
    //LEDStatus ledStatus; /**< manages led behavior*/
    system_tick_t startTime; /**< start time at initialization */
    int numTasks;
    DeploymentSchedule_ * tasks;
    Scheduler create();

};

#endif