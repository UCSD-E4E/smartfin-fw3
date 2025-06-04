/**
 * @file ensembles.hpp
 * @brief Header file to declare various ensembles
 */

#ifndef __ENSEMBLES_HPP__
#define __ENSEMBLES_HPP__
#include "scheduler.hpp"

void SS_ensemble10Func(DeploymentSchedule_t *pDeployment);
void SS_ensemble10Init(DeploymentSchedule_t *pDeployment);

void SS_ensemble07Func(DeploymentSchedule_t *pDeployment);
void SS_ensemble07Init(DeploymentSchedule_t *pDeployment);

void SS_ensemble08Func(DeploymentSchedule_t *pDeployment);
void SS_ensemble08Init(DeploymentSchedule_t *pDeployment);

void SS_fwVerInit(DeploymentSchedule_t *pDeployment);
void SS_fwVerFunc(DeploymentSchedule_t *pDeployment);

void SS_Ensemble01_Init(DeploymentSchedule_t *pDeployment);
void SS_Ensemble01_Func(DeploymentSchedule_t *pDeployment);

void SS_HighRateIMU_x0C_Init(DeploymentSchedule_t *pDeployment);
void SS_HighRateIMU_x0C_Func(DeploymentSchedule_t *pDeployment);

#endif //__ENSEMBLES_HPP__