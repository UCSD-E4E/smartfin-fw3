#ifndef __RIDE_HPP__
#define __RIDE_HPP__
#include "task.hpp"

#include "Particle.h"
#include "product.hpp"

class RideTask : public Task
{
public:
    void init(void);
    STATES_e run(void);
    void exit(void);
private:
    LEDStatus ledStatus;
    bool gpsLocked;
    system_tick_t startTime;

};

#endif