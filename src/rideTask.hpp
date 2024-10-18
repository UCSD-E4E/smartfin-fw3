/**
 * @file rideTask.hpp
 * @brief Header file for deployment state
 * 
 */
#ifndef __RIDE_HPP__
#define __RIDE_HPP__
#include "task.hpp"
#include "product.hpp"
#include "scheduler.hpp"

#include <Particle.h>


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
    //! TODO: implement LEDStatus
    LEDStatus ledStatus; //!< manages led behavior
    
    system_tick_t startTime; /**< start time at initialization */

    Scheduler scheduler;

};

#endif