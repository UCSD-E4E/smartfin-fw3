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

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#define retained
class EEPROMClass
{
private:
    uint8_t *blob;
    std::size_t blob_size;

public:
    EEPROMClass();

    template <typename T> T &get(int idx, T &t)
    {
        std::memcpy(&t, this->blob + idx, sizeof(T));
        return t;
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
public:
    void begin(int baud)
    {
    }

    std::size_t available()
    {
        return 0;
    }

    int read()
    {
        return 0;
    }

    void print(char ch)
    {
    }
};
#define Serial __get_default_serial()
USARTSerial &__get_default_serial();

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
typedef enum
{
    RESET_REASON_NONE
} __RESET_REASON_t;
class SystemClass
{
public:
    int enableFeature(HAL_Feature feature)
    {
        return 0;
    }
    __RESET_REASON_t resetReason()
    {
        return RESET_REASON_NONE;
    }
};
#define System __fetch_global_System()
SystemClass &__fetch_global_System();

typedef std::uint32_t time32_t;

class TimeClass
{
public:
    time32_t now()
    {
        return 0;
    }

    void delay(std::uint32_t ms)
    {
    }
};
#define Time __fetch_global_time()
TimeClass &__fetch_global_time();
#endif // __PC_HAL_PARTICLE_H__