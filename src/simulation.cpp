
#include "simulation.hpp"

Simulation::Simulation(DeploymentSchedule_ deploymentSchedule[], int num){
    int numTasks=num;
    tasks=deploymentSchedule;
    runtimes=nullptr;
    records=0;

}


void Simulation::run(void)
{
   uint32_t startTime=millis();
   Scheduler s=Scheduler(tasks, numTasks, startTime);
   while(millis()<15000){
        
        s.SCH_runSchedule();

   }
   //runtimes=&(s.runTimes); //what datatype should runtimes be 
   records=s.recordRuns;

}


void Simulation::printRunTimes(){

    /*if(runtimes!=nullptr){
        for(int i=0; i<numTasks; i++){
            print("Task:" + tasks[i].taskName);
            for(int j=0; j<records; j++){
                print("Runtime:" + runtimes[i][j]);
            }
        }

    }*/



}