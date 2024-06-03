#ifndef SCHEDULER_FUNCTIONS_HPP
#define SCHEDULER_FUNCTIONS_HPP
#include "../src/scheduler.hpp"
#include "../tests/scheduler_test_system.hpp"

void SS_ensembleAInit(DeploymentSchedule_t* pDeployment);
void SS_ensembleAFunc(DeploymentSchedule_t* pDeployment);
void SS_ensembleBInit(DeploymentSchedule_t* pDeployment);
void SS_ensembleBFunc(DeploymentSchedule_t* pDeployment);
void SS_ensembleCInit(DeploymentSchedule_t* pDeployment);
void SS_ensembleCFunc(DeploymentSchedule_t* pDeployment);

#endif // SCHEDULER_FUNCTIONS_HPP