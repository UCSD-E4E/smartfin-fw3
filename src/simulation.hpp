

#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "../tests/scheduler_test_system.hpp"
#endif
#include "scheduler.hpp"



class Simulation
{
public:
    Simulation(DeploymentSchedule_ deploymentSchedule[], int num);
    
    
    
    void run(void);
   
   
    void printRunTimes();
private:
    
    //LEDStatus ledStatus; /**< manages led behavior*/
    system_tick_t startTime; /**< start time at initialization */
    int numTasks;
    DeploymentSchedule_ * tasks;
    int ** runtimes; 
    int records;
    

};