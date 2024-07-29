#include "abstractScheduler.hpp"

typedef struct DeploymentSchedule_ DeploymentSchedule_t;
struct  DeploymentSchedule_t
 {
    //Ensemble properties
    int priority;
    void (*measure)();               //!< measurement function
    void (*init)();                       //!< initialization function
    //EnsembleProccess process;             //!< processing function
    uint32_t startDelay;                 //!< delay after deployment start
    uint32_t ensembleInterval;              //!< time between ensembles
    //void* pData;               //!< Buffer to store measurements temporarily
    uint32_t maxDuration;       //!< store max running time of measurement
    char taskName;  //!< task name of ensemble
    uint32_t maxDelay;
   
    //State Information
    uint32_t nextRunTime;
};



class Scheduler : public AbstractScheduler{

   DeploymentSchedule_ * tasks;
   int numTasks;
   public:
    Scheduler(DeploymentSchedule_ schedule[], int length);
    int getNextTask(const DeploymentSchedule_t* p_next_task,
                    std::uint32_t* p_next_runtime,
                    std::uint32_t current_time);






};