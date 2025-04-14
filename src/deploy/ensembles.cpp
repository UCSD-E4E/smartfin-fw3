/**
 * @file ensembles.cpp
 * @brief Contains definitions of ensembles (updated version of smartfin-fw2 ensembles)
 */
#include "ensembles.hpp"

#include "cellular/ensembleTypes.hpp"
#include "imu/imu.hpp"
#include "product.hpp"
#include "scheduler.hpp"
#include "system.hpp"
#include "util.hpp"
#include "vers.hpp"
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

// define ensemble structs
typedef struct Ensemble10_eventData_
{
    int16_t temperature;
    int16_t water;
    int16_t acc[3];
    int16_t ang[3];
    int16_t mag[3];
    int32_t location[2];
    uint8_t hasGPS;
    uint32_t accumulateCount;
} Ensemble10_eventData_t;

typedef struct Ensemble08_eventData_
{
    double temperature;
    int32_t water;

    uint32_t accumulateCount;
} Ensemble08_eventData_t;

typedef struct Ensemble07_eventData_
{
    uint16_t battVoltage;
    uint32_t accumulateCount;
} Ensemble07_eventData_t;

static Ensemble10_eventData_t ensemble10Data;
static Ensemble07_eventData_t ensemble07Data;
static Ensemble08_eventData_t ensemble08Data;

void SS_ensemble10Init(DeploymentSchedule_t *pDeployment)
{
    memset(&ensemble10Data, 0, sizeof(Ensemble10_eventData_t));
    pDeployment->state.pData = &ensemble10Data;
}

void SS_ensemble07Init(DeploymentSchedule_t *pDeployment)
{
    memset(&ensemble07Data, 0, sizeof(Ensemble07_eventData_t));
    pDeployment->state.pData = &ensemble07Data;
}

void SS_ensemble08Init(DeploymentSchedule_t *pDeployment)
{
    memset(&ensemble08Data, 0, sizeof(Ensemble08_eventData_t));
    pDeployment->state.pData = &ensemble08Data;
}

void SS_ensemble10Func(DeploymentSchedule_t *pDeployment)
{
    float temp;
    uint8_t water;
    int32_t lat, lng;
    float accelData[3];
    float gyroData[3];
    float magData[3];
    bool hasGPS = false;
    Ensemble10_eventData_t *pData = (Ensemble10_eventData_t *)pDeployment->state.pData;

#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        union
        {
            Ensemble10_data_t ens10;
            Ensemble11_data_t ens11;
        } data;
    } ensData;
#pragma pack(pop)

    // Obtain measurements
    temp = pSystemDesc->pTempSensor->getTemp();
    water = pSystemDesc->pWaterSensor->getCurrentReading();
    getAccelerometer(accelData, accelData + 1, accelData + 2);
    getGyroscope(gyroData, gyroData + 1, gyroData + 2);
    getMagnetometer(magData, magData + 1, magData + 2);

    // GPS
    bool locked;
    unsigned int satsInView;
    ubloxGPS *ubloxGps_(nullptr);
    locked = (ubloxGps_->getLock()) ? 1 : 0;
    gps_sat_t sats_in_view_desc[NUM_SAT_DESC];
    satsInView = ubloxGps_->getSatellitesDesc(sats_in_view_desc);
    if (locked && satsInView > 4)
    {
        hasGPS = true;
        lat = ubloxGps_->getLatitude();
        lng = ubloxGps_->getLongitude();
    }
    else
    {
        hasGPS = false;
        lat = pData->location[0];
        lng = pData->location[1];
    }

    // Accumulate measurements
    pData->temperature += temp;
    pData->water += water;
    pData->acc[0] += B_TO_N_ENDIAN_2(accelData[0]);
    pData->acc[1] += B_TO_N_ENDIAN_2(accelData[1]);
    pData->acc[2] += B_TO_N_ENDIAN_2(accelData[2]);
    pData->ang[0] += B_TO_N_ENDIAN_2(gyroData[0]);
    pData->ang[1] += B_TO_N_ENDIAN_2(gyroData[1]);
    pData->ang[2] += B_TO_N_ENDIAN_2(gyroData[2]);
    pData->mag[0] += B_TO_N_ENDIAN_2(magData[0]);
    pData->mag[1] += B_TO_N_ENDIAN_2(magData[1]);
    pData->mag[2] += B_TO_N_ENDIAN_2(magData[2]);
    pData->location[0] += lat;
    pData->location[1] += lng;
    pData->hasGPS += hasGPS ? 1 : 0;
    pData->accumulateCount++;

    // Report accumulated measurements
    if (pData->accumulateCount == pDeployment->measurementsToAccumulate)
    {
        water = pData->water / pDeployment->measurementsToAccumulate;
        temp = pData->temperature / pDeployment->measurementsToAccumulate;
        if (water == false)
        {
            temp -= 100;
        }

        ensData.header.elapsedTime_ds = Ens_getStartTime(
            pDeployment->state.nextRunTime); // does nextruntime work for start time
        SF_OSAL_printf("Ensemble timestamp: %d\n", ensData.header.elapsedTime_ds);
        ensData.data.ens10.rawTemp = N_TO_B_ENDIAN_2(temp / 0.0078125);
        ensData.data.ens10.rawAcceleration[0] =
            N_TO_B_ENDIAN_2(pData->acc[0] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawAcceleration[1] =
            N_TO_B_ENDIAN_2(pData->acc[1] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawAcceleration[2] =
            N_TO_B_ENDIAN_2(pData->acc[2] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawAngularVel[0] =
            N_TO_B_ENDIAN_2(pData->ang[0] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawAngularVel[1] =
            N_TO_B_ENDIAN_2(pData->ang[1] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawAngularVel[2] =
            N_TO_B_ENDIAN_2(pData->ang[2] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawMagField[0] =
            N_TO_B_ENDIAN_2(pData->mag[0] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawMagField[1] =
            N_TO_B_ENDIAN_2(pData->mag[1] / pDeployment->measurementsToAccumulate);
        ensData.data.ens10.rawMagField[2] =
            N_TO_B_ENDIAN_2(pData->mag[2] / pDeployment->measurementsToAccumulate);
        ensData.data.ens11.location[0] =
            N_TO_B_ENDIAN_4(pData->location[0] / pDeployment->measurementsToAccumulate);
        ensData.data.ens11.location[1] =
            N_TO_B_ENDIAN_4(pData->location[1] / pDeployment->measurementsToAccumulate);

        if (pData->hasGPS / pDeployment->measurementsToAccumulate)
        {
            ensData.header.ensembleType = ENS_TEMP_IMU_GPS;
            pSystemDesc->pRecorder->putBytes(&ensData,
                                             sizeof(EnsembleHeader_t) + sizeof(Ensemble11_data_t));
        }
        else
        {
            ensData.header.ensembleType = ENS_TEMP_IMU;
            pSystemDesc->pRecorder->putBytes(&ensData,
                                             sizeof(EnsembleHeader_t) + sizeof(Ensemble10_data_t));
        }

        memset(pData, 0, sizeof(Ensemble10_eventData_t));
    }
}

void SS_ensemble07Func(DeploymentSchedule_t *pDeployment)
{
    float battVoltage;
    Ensemble07_eventData_t *pData = (Ensemble07_eventData_t *)pDeployment->state.pData;
#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        Ensemble07_data_t data;
    } ensData;
#pragma pack(pop)

    // obtain measurements
    battVoltage = pSystemDesc->pBattery->getVCell();

    // accumulate measurements
    pData->battVoltage += battVoltage;
    pData->accumulateCount++;

    // Report accumulated measurements
    if (pData->accumulateCount == pDeployment->measurementsToAccumulate)
    {
        ensData.header.elapsedTime_ds = Ens_getStartTime(pDeployment->state.nextRunTime);
        ensData.header.ensembleType = ENS_BATT;
        ensData.data.batteryVoltage =
            N_TO_B_ENDIAN_2((pData->battVoltage / pData->accumulateCount) * 1000);

        pSystemDesc->pRecorder->putData(ensData);
        memset(pData, 0, sizeof(Ensemble07_eventData_t));
    }
}

void SS_ensemble08Func(DeploymentSchedule_t *pDeployment)
{
    float temp;
    uint8_t water;

    Ensemble08_eventData_t *pData = (Ensemble08_eventData_t *)pDeployment->state.pData;
#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        Ensemble08_data_t ensData;
    } ens;
#pragma pack(pop)

    // obtain measurements
    temp = pSystemDesc->pTempSensor->getTemp();
    water = pSystemDesc->pWaterSensor->getCurrentReading();

    // accumulate measurements
    pData->temperature += temp;
    pData->water += water;
    pData->accumulateCount++;

    // Report accumulated measurements
    if (pData->accumulateCount == pDeployment->measurementsToAccumulate)
    {
        water = pData->water / pDeployment->measurementsToAccumulate;
        temp = pData->temperature / pDeployment->measurementsToAccumulate;
        if (water == false)
        {
            temp -= 100;
        }

        ens.header.elapsedTime_ds = Ens_getStartTime(pDeployment->state.nextRunTime);
        ens.header.ensembleType = ENS_TEMP_TIME;
        ens.ensData.rawTemp = N_TO_B_ENDIAN_2(temp / 0.0078125);

        pSystemDesc->pRecorder->putData(ens);
        memset(pData, 0, sizeof(Ensemble08_eventData_t));
    }
}

void SS_fwVerInit(DeploymentSchedule_t *pDeployment)
{
    (void)pDeployment;
}
void SS_fwVerFunc(DeploymentSchedule_t *pDeployment)
{
#pragma pack(push, 1)
    struct textEns
    {
        EnsembleHeader_t header;
        uint8_t nChars;
        char verBuf[32];
    } ens;
#pragma pack(pop)

    ens.header.elapsedTime_ds = Ens_getStartTime(pDeployment->state.nextRunTime);
    ens.header.ensembleType = ENS_TEXT;

    ens.nChars = snprintf(ens.verBuf,
                          32,
                          "FW%d.%d.%d%s",
                          FW_MAJOR_VERSION,
                          FW_MINOR_VERSION,
                          FW_BUILD_NUM,
                          FW_BRANCH);
    pSystemDesc->pRecorder->putBytes(&ens, sizeof(EnsembleHeader_t) + sizeof(uint8_t) + ens.nChars);
}
