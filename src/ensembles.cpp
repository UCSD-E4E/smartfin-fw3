#include "ensembles.hpp"

#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "../tests/scheduler_test_system.hpp"
#endif


void SS_ensembleAInit(DeploymentSchedule_t* pDeployment)
{
    return;
}
void SS_ensembleAFunc(DeploymentSchedule_t* pDeployment)
{
    delay(400);
}


void SS_ensembleBInit(DeploymentSchedule_t* pDeployment)
{
    return;
}
void SS_ensembleBFunc(DeploymentSchedule_t* pDeployment)
{
    delay(200);

}

void SS_ensembleCInit(DeploymentSchedule_t* pDeployment)
{
    return;
}
void SS_ensembleCFunc(DeploymentSchedule_t* pDeployment)
{
    delay(600);
}