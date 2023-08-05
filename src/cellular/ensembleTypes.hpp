#ifndef __ENSEMBLE_TYPES_HPP__
#define __ENSEMBLE_TYPES_HPP__
#include "Particle.h"
#include <cstdint>
typedef enum EnsembleID_
{
    ENS_TEMP = 0x01,
    ENS_ACC,
    ENS_GPS,
    ENS_TEMP_ACC,
    ENS_TEMP_GPS,
    ENS_TEMP_ACC_GPS,
    ENS_BATT,
    ENS_TEMP_TIME,
    ENS_IMU,
    ENS_TEMP_IMU,
    ENS_TEMP_IMU_GPS,
    ENS_TEXT = 0x0F,
    ENS_NUM_ENSEMBLES
}EnsembleID_e;

#pragma pack(push, 1)
typedef struct EnsembleHeader_
{
    unsigned int ensembleType : 4;
    unsigned int elapsedTime_ds : 20;
}EnsembleHeader_t;

/**
 * @brief Ensemble 07 - Battery
 * 
 */
typedef struct Ensemble07_data_
{
    /**
     * @brief Battery voltage (mV)
     * 
     */
    uint16_t batteryVoltage;
}Ensemble07_data_t;

typedef struct Ensemble08_data_
{
    int16_t rawTemp;
    uint32_t timestamp;
}Ensemble08_data_t;

typedef struct Ensemble10_data_
{
    int16_t rawTemp;
    int16_t rawAcceleration[3];
    int16_t rawAngularVel[3];
    int16_t rawMagField[3];
}Ensemble10_data_t;

typedef struct Ensemble11_data_
{
    int16_t rawTemp;
    int16_t rawAcceleration[3];
    int16_t rawAngularVel[3];
    int16_t rawMagField[3];
    int32_t location[2];
}Ensemble11_data_t;
#pragma pack(pop)

unsigned int Ens_getStartTime(system_tick_t sessionStart);
#endif