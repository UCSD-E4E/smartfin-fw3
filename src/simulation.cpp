
#include "simulation.hpp"

Simulation::Simulation(DeploymentSchedule_ deploymentSchedule[], int num){
    int numTasks=num;
    tasks=deploymentSchedule;

}


void Simulation::run(void)
{
   uint32_t startTime=millis();
   Scheduler s=Scheduler(tasks, numTasks, startTime);
   while(millis()<15000){
        
        s.SCH_runSchedule();

   }
}


void Simulation::printRunTimes(){

    



}