#include "test_ensembles.cpp"
#include "../src/simulation.hpp"


int main(int argc, char* argv[])
{
   int numTasks=3;
   DeploymentSchedule_ tasks[3];
   DeploymentSchedule_ A;
   A.priority=1;
   A.measure=SS_ensembleAFunc;
   A.startDelay=0;
   A.ensembleInterval=33;
   A.maxDuration=1;
   A.taskName='A';
   tasks[0]=A;
   DeploymentSchedule_ B;
   B.priority=2;
   B.measure=SS_ensembleBFunc;
   B.startDelay=0;
   B.ensembleInterval=1000;
   B.maxDuration=1;
   B.taskName='B';
   tasks[1]=B;
   DeploymentSchedule_ C;
   C.measure=SS_ensembleCFunc;
   C.startDelay=0;
   C.ensembleInterval=1000;
   C.maxDuration=1;
   C.taskName='C';
   tasks[2]=C;
   Simulation ride=Simulation(tasks, numTasks);
   ride.run();
   ride.printRunTimes();

}

