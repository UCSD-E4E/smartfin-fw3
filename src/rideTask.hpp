/**
 * @file rideTask.hpp
 * @brief Header file for deployment state
 * 
 */
#ifndef __RIDE_HPP__
#define __RIDE_HPP__
#include "task.hpp"

#include "Particle.h"
#include "product.hpp"

/**
 * @class RideTask
 * @brief Manages deployment of measurements and recording
 */
class RideTask : public Task
{
public:
    /**
     * @brief initialize ride task
     * Sets LEDs  and initializes schedule
    */
    void init(void);
    /**
     * @brief runs tasks given by scheduler
     * 
     */
    STATES_e run(void);
    /**
     * @brief exits ride state
     */
    void exit(void);
private:
    
    LEDStatus ledStatus; /**< manages led behavior*/
    bool gpsLocked; /** @todo add documentation*/
    system_tick_t startTime; /**< start time at initialization */

};

#endif