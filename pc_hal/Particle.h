/**
 * @file Particle.h
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief GLIBC Particle Shim
 * @version 0.1
 * @date 2024-11-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __PC_HAL_PARTICLE_H__
#define __PC_HAL_PARTICLE_H__

#include <functional>
#include <string>
class EEPROMClass
{
    EEPROMClass()
    {
    }

    template <typename T> T &get(int idx, T &t)
    {
        return T();
    }

    template <typename T> const T &put(int idx, const T &t)
    {
        return t;
    }
};

#define EEPROM __fetch_global_EEPROM()
EEPROMClass &__fetch_global_EEPROM();

class Timer
{
};

class USARTSerial
{
};

class SPIClass
{
};

class String : public std::string
{
};

typedef enum
{

} LogLevel;

class Logger
{
};
#endif // __PC_HAL_PARTICLE_H__