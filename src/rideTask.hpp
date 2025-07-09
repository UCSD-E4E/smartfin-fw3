/**
 * @file rideTask.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2025-04-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __RIDE_TASK_HPP__
#define __RIDE_TASK_HPP__
#include "Particle.h"
#include "deploy/scheduler.hpp"
#include "product.hpp"
#include "task.hpp"

/**
 * @class RideTask
 * @brief Manages deployment of measurements and recording
 */
class RideTask : public Task
{
public:
    RideTask();
    /**
     * @brief initialize ride task
     * Sets LEDs  and initializes schedule
     */
    void init(void);
    /**
     * @brief runs tasks given by scheduler
     *
     * @return the next state to change to.
     */
    STATES_e run(void);
    /**
     * @brief exits ride state
     */
    void exit(void);

private:
    /**
     * @brief LEDStatus class to manage LED behavior
     *
     * This class manages the LED behavior for the ride task.
     * It handles the initialization and updates of the LEDs based on the task state.
     */
    LEDStatus ledStatus;

    /**
     * @brief Start time at initialization
     *
     */
    system_tick_t startTime;
    /**
     * @brief Deploy Time in ms since boot
     *
     */
    system_tick_t deployTime;

    /**
     * @brief Flag if session time has been set to UTC time
     *
     */
    bool sessionTimeSet;

    Scheduler scheduler;
};
extern DeploymentSchedule_t deploymentSchedule[];
#endif //__RIDE_TASK_HPP__