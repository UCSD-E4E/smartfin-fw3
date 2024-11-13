/**
 * @file Particle_dummy.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Dummy Particle Implementation
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "Particle.h"

EEPROMClass __global_eeprom;
EEPROMClass &__fetch_global_EEPROM()
{
    return __global_eeprom;
}