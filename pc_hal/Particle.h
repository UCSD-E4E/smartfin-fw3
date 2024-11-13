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

typedef struct
{

} __SPISettings;

class os_queue_t
{
};

class Thread
{
};

enum
{
    PIN_INVALID
};
enum
{
    HIGH
};

class TwoWire
{
};
typedef std::size_t system_tick_t;

template <class T> class Vector
{
};

class RecursiveMutex
{
};

typedef struct
{

} LEDSystemTheme;

class FuelGauge
{
};

typedef struct
{

} LEDStatus;

#define SYSTEM_MODE(a) ;
#define SYSTEM_THREAD(a) ;

typedef enum
{
    FEATURE_RESET_INFO
} HAL_Feature;
class SystemClass
{
public:
    int enableFeature(HAL_Feature feature)
    {
        return 0;
    }
};
#define System __fetch_global_System()
SystemClass &__fetch_global_System();
#endif // __PC_HAL_PARTICLE_H__