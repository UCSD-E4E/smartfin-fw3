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
#include "location_service.h"

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

Timer::Timer(int period, void (*fn)(void), bool one_shot)
    : period(period), cb(fn), one_shot(one_shot), active(false)
{
}

String::String(const char *val) : val(val)
{
}

TwoWire __global_i2c;
TwoWire &__fetch_global_I2C()
{
    return __global_i2c;
}

CellularClass __global_cellular;
CellularClass &__fetch_global_cellular()
{
    return __global_cellular;
}

void pinMode(int pin, __PC_HAL_PIN_CONFIG mode)
{
}

void digitalWrite(int pin, int state)
{
}

void digitalWriteFast(int pin, int state)
{
}

int digitalRead(int pin)
{
    return 0;
}
int pinReadFast(int pin)
{
    return 0;
}

void LocationService::setModuleType(void)
{
}
int LocationService::begin(const LocationServiceConfiguration &config)
{
    return 1;
}
int LocationService::start(bool restart)
{
    return 1;
}
void LocationService::setFastLock(bool enable)
{
}

LocationService::LocationService()
{
}
LocationService *LocationService::_instance = nullptr;
int LocationService::getLocation(LocationPoint &point)
{
    return 1;
}