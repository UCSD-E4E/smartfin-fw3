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

#include <memory>
EEPROMClass::EEPROMClass()
{
    this->blob_size = 2048;
    this->blob = new std::uint8_t[this->blob_size];
    std::memset(this->blob, 0xFF, this->blob_size);
}

EEPROMClass __global_eeprom;
EEPROMClass &__fetch_global_EEPROM()
{
    return __global_eeprom;
}

SystemClass __global_system;
SystemClass &__fetch_global_System()
{
    return __global_system;
}

USARTSerial __default_serial;
USARTSerial &__get_default_serial()
{
    return __default_serial;
}

TimeClass __global_time;
TimeClass &__fetch_global_time()
{
    return __global_time;
}

ParticleClass __global_particle;
ParticleClass &__fetch_global_particle()
{
    return __global_particle;
}