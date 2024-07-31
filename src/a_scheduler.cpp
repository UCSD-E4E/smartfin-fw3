#include "a_scheduler.hpp"


Scheduler:: Scheduler(DeploymentSchedule_t schedule[]){
    tasks=schedule;
    
    int i=0;
   for(; tasks[i].measure; i++){
        if(i=0){
        tasks[i].startDelay=0;
        }else {
        if(i!=0){
            tasks[i].startDelay=(tasks[i-1].startDelay)+(tasks[i-1].maxDuration);
        }
        }

    }
    int numTasks=i-1;

}


int Scheduler:: getNextTask(const DeploymentSchedule_t* p_next_task, std::uint32_t* p_next_runtime, std::uint32_t current_time){

        //check in reverse order, return if possible
        int i=numTasks;

        while(i>0){
            bool canSet=true;
            int runTime=tasks[i].nextRunTime;
            int delay=current_time-tasks[i].nextRunTime;
            if(delay>0){
                runTime=current_time;
            }else{
                delay=0;
            }
            if(delay==tasks[i].maxDelay){
                p_next_task=&(tasks[i]);
                tasks[i].nextRunTime+=delay+tasks[i].ensembleInterval;
                return runTime;
            }
            int completion=runTime+tasks[i].ensembleInterval;
            int j=0;
            while(j<i && canSet){
                if(tasks[j].nextRunTime<completion){
                    canSet=false;
                }
                j++;

            }
            if(canSet){
                p_next_task=&(tasks[i]);
                tasks[i].nextRunTime+=delay+tasks[i].ensembleInterval;
                return runTime;

            }


            i--;

        }

       p_next_task=&(tasks[0]);
       int runTime=tasks[0].nextRunTime;
       int delay=current_time-(tasks[0].nextRunTime);
       if(delay>0){
        runTime=current_time;
       }else{
        delay=0;
       }
       tasks[0].nextRunTime+=delay+tasks[0].ensembleInterval;
       
       return runTime;


}

