#include "abstractScheduler.hpp"

class Scheduler : public AbstractScheduler{

   DeploymentSchedule_t * tasks;
   int numTasks;
   public:
    Scheduler(DeploymentSchedule_t schedule[]);
    int getNextTask(const DeploymentSchedule_t* p_next_task,
                    std::uint32_t* p_next_runtime,
                    std::uint32_t current_time);

};