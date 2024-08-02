/**
 * @file ensembles.hpp
 * @brief Header file to declare various dummy ensembles
 */

#ifndef __TEST_ENSEMBLES_HPP__
#define __TEST_ENSEMBLES_HPP__
#include "product.hpp"
#if SCHEDULER_VERSION == CHARLIE_VERSION
#include "charlie_scheduler.hpp"
#else
#include "antara_scheduler.hpp"
#endif
/**
 * @brief dummy ensemble init function
 * @param pDeployment schedule table
 */
void SS_ensembleAInit(DeploymentSchedule_t* pDeployment);

/**
 * @brief dummy ensemble function
 * @param pDeployment schedule table
 */
void SS_ensembleAFunc(DeploymentSchedule_t* pDeployment);

/**
 * @brief dummy ensemble init function
 * @param pDeployment schedule table
 */
void SS_ensembleBInit(DeploymentSchedule_t* pDeployment);

/**
 * @brief dummy ensemble function
 * @param pDeployment schedule table
 */
void SS_ensembleBFunc(DeploymentSchedule_t* pDeployment);

/**
 * @brief dummy ensemble init function
 * @param pDeployment schedule table
 */
void SS_ensembleCInit(DeploymentSchedule_t* pDeployment);

/**
 * @brief dummy ensemble function
 * @param pDeployment schedule table
 */
void SS_ensembleCFunc(DeploymentSchedule_t* pDeployment);


#endif //__TEST_ENSEMBLES_HPP__