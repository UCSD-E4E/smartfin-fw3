/**
 * @file ensembles.cpp
 * @brief function definitions for test_ensembles.hpp
 * 
 */
#include "test_ensembles.hpp"
#ifndef TEST_VERSION
#include "Particle.h"
#else
#include "scheduler_test_system.hpp"
#endif


/**
 * @brief dummy ensemble init function
 * @param pDeployment schedule table
 */
void SS_ensembleAInit()
{
    return;
}
/**
 * @brief dummy measurement function that delays for 400 ms
 * @param pDeployment schedule table
 */
void SS_ensembleAFunc()
{
    delay(1);
}

/**
 * @brief dummy ensemble init function
 * @param pDeployment schedule table
 */
void SS_ensembleBInit()
{
    return;
}
/**
 * @brief dummy measurement function that delays for 200 ms
 * @param pDeployment schedule table
 */
void SS_ensembleBFunc()
{
    delay(1);

}

/**
 * @brief dummy ensemble init function
 * 
 */
void SS_ensembleCInit()
{
    return;
}
/**
 * @brief dummy measurement function that delays for 600 ms

 */
void SS_ensembleCFunc()
{
    delay(1);
}