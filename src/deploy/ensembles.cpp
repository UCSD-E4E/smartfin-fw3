/**
 * @file ensembles.cpp
 * @brief Contains definitions of ensembles (updated version of smartfin-fw2 ensembles)
 *
 * \defgroup ensemble_imp Ensemble Implementation
 * @{
 */
#include "ensembles.hpp"

#include "consts.hpp"
#include "deploy/ensembleTypes.hpp"
#include "imu/newIMU.hpp"
#include "product.hpp"
#include "scheduler.hpp"
#include "system.hpp"
#include "util.hpp"
#include "vers.hpp"

#include <cmath>
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

/**
 * @brief Ensemble 01 (Temperature)
 *
 * See https://github.com/UCSD-E4E/smartfin-fw3/wiki/Smartfin-Publish-Data-Format#temperature-0x01
 *
 * \defgroup ensemble_01 Temperature (0x01)
 * @{
 */

/**
 * @brief Ensemble 01 (Temperature) data structure
 *
 * This structure contains the temperature and water data accumulation fields.
 *
 */
typedef struct
{
    float temperature;
    int16_t water;
    uint32_t accumulateCount;
} Ensemble01_eventData_t;
/**
 * @brief Ensemble 01 (Temperature) data store
 *
 */
static Ensemble01_eventData_t ensemble01Data;

/**
 * @brief Ensemble 01 (Temperature) Initialization
 *
 * This function initializes all of the support structures for Ensemble 01.
 *
 * We only support external temperature sensor.
 *
 * @param pDeployment Deployment data
 */
void SS_Ensemble01_Init(DeploymentSchedule_t *pDeployment)
{
    memset(&ensemble01Data, 0x00, sizeof(Ensemble01_eventData_t));
    pDeployment->state.pData = &ensemble01Data;
}

/**
 * @brief Ensemble 01 (Temperature) Function
 *
 * This function accumulates and decimates data for Ensemble 01
 *
 * We only support external temperature sensor.
 *
 * @param pDeployment Deployment data
 */
void SS_Ensemble01_Func(DeploymentSchedule_t *pDeployment)
{
    float temp;
    int water;
    // Accumulate data
    Ensemble01_eventData_t *pData = (Ensemble01_eventData_t *)pDeployment->state.pData;

#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        Ensemble01_data_t data;
    } ensData;
#pragma pack(pop)

    temp = pSystemDesc->pTempSensor->getTemp();
    water = pSystemDesc->pWaterSensor->getLastReading();
    // FIXME: change error catch
    if (0 != temp)
    {
        pData->temperature += temp;
        pData->water += water;
        pData->accumulateCount += 1;
    }

    if (pData->accumulateCount == pDeployment->measurementsToAccumulate)
    {
        // We have accumulated enough to average, average, write, and reset
        water = pData->water / pDeployment->measurementsToAccumulate;
        temp = pData->temperature / pDeployment->measurementsToAccumulate;

        // populate ensemble
        ensData.header.ensembleType = ENS_TEMP;
        ensData.header.elapsedTime_ds = Ens_getStartTime();
        ensData.data.raw_temp = 128 * (temp - 100 * water);

        // Commit ensemble
        pSystemDesc->pRecorder->putBytes(&ensData, sizeof(ensData));

        // Reset data
        pData->temperature = 0;
        pData->water = 0;
        pData->accumulateCount = 0;
    }
}

/** @}*/

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
static Ensemble10_eventData_t ensemble10Data;

typedef struct Ensemble08_eventData_
{
    double temperature;
    int32_t water;

    uint32_t accumulateCount;
} Ensemble08_eventData_t;
static Ensemble08_eventData_t ensemble08Data;

typedef struct Ensemble07_eventData_
{
    uint16_t battVoltage;
    uint32_t accumulateCount;
} Ensemble07_eventData_t;
static Ensemble07_eventData_t ensemble07Data;

/**
 * @}
 */

/**
 * @brief Ensemble Functions
 *
 * \defgroup ens_fn Ensemble Functions
 * \addtogroup ens_fn
 * @{
 */

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
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    float temp = NAN;
    uint8_t water = UINT8_MAX;
    int32_t lat = INT32_MAX, lng = INT32_MAX;
    float accelData[3] = {NAN, NAN, NAN};
    float gyroData[3] = {NAN, NAN, NAN};
    float magData[3] = {NAN, NAN, NAN};
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
    // temp = pSystemDesc->pTempSensor->getTemp();
    // water = pSystemDesc->pWaterSensor->getCurrentReading();
    // getAccelerometer(accelData, accelData + 1, accelData + 2);
    // getGyroscope(gyroData, gyroData + 1, gyroData + 2);
    // getMagnetometer(magData, magData + 1, magData + 2);

    // // GPS
    // bool locked;
    // unsigned int satsInView;
    // ubloxGPS *ubloxGps_(nullptr);
    // locked = (ubloxGps_->getLock()) ? 1 : 0;
    // gps_sat_t sats_in_view_desc[NUM_SAT_DESC];
    // satsInView = ubloxGps_->getSatellitesDesc(sats_in_view_desc);
    // if (locked && satsInView > 4)
    // {
    //     hasGPS = true;
    //     lat = ubloxGps_->getLatitude();
    //     lng = ubloxGps_->getLongitude();
    // }
    // else
    // {
    //     hasGPS = false;
    //     lat = pData->location[0];
    //     lng = pData->location[1];
    // }

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

        ensData.header.elapsedTime_ds = Ens_getStartTime();
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
#endif
}

void SS_ensemble07Func(DeploymentSchedule_t *pDeployment)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
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
        ensData.header.elapsedTime_ds = Ens_getStartTime();
        ensData.header.ensembleType = ENS_BATT;
        ensData.data.batteryVoltage =
            N_TO_B_ENDIAN_2((pData->battVoltage / pData->accumulateCount) * 1000);

        pSystemDesc->pRecorder->putData(ensData);
        memset(pData, 0, sizeof(Ensemble07_eventData_t));
    }
#endif
}

void SS_ensemble08Func(DeploymentSchedule_t *pDeployment)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
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

        ens.header.elapsedTime_ds = Ens_getStartTime();
        ens.header.ensembleType = ENS_TEMP_TIME;
        ens.ensData.rawTemp = N_TO_B_ENDIAN_2(temp / 0.0078125);

        pSystemDesc->pRecorder->putData(ens);
        memset(pData, 0, sizeof(Ensemble08_eventData_t));
    }
#endif
}

/**
 * @brief Firmware Ensemble
 * \defgroup fw_ens Firmware Ensemble
 * \addtogroup fw_ens
 * @{
 */

/**
 * @brief Firmware Ensemble Initialization
 *
 * @param pDeployment Deployment schedule block
 */
void SS_fwVerInit(DeploymentSchedule_t *pDeployment)
{
    (void)pDeployment;
}
/**
 * @brief Firmware Ensemble execute
 *
 * @param pDeployment Deployment schedule block
 */
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

    ens.header.elapsedTime_ds = Ens_getStartTime();
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
/** @} */

/**
 * @brief High Data Rate IMU Ensemble (0x0C)
 * \addtogroup hdr_imu_ens
 * @{
 */

/**
 * @brief Initialization function for high data rate IMU
 *
 * @param pDeployment Deployment event
 */
void SS_HighRateIMU_x0C_Init(DeploymentSchedule_t *pDeployment)
{
    (void)pDeployment;
}

/**
 * @brief Measurement function for high data rate IMU
 *
 * @param pDeployment Deployment event
 */
void SS_HighRateIMU_x0C_Func(DeploymentSchedule_t *pDeployment)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE && defined(SF_HIGH_DATA_RATE)
#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        Ensemble12_data_t data;
    } ensData;
#pragma pack(pop)
    float values[9];

    pSystemDesc->pIMU->getDmpAccel_ms2(values[0], values[1], values[2]);
    pSystemDesc->pIMU->getDmpRotVel_dps(values[3], values[4], values[5]);
    pSystemDesc->pIMU->getDmpMag_uT(values[6], values[7], values[8]);

    ensData.data.acceleration_ms2_q14[0] = N_TO_B_ENDIAN_2((int16_t)(values[0] * Q14_SCALAR));
    ensData.data.acceleration_ms2_q14[1] = N_TO_B_ENDIAN_2((int16_t)(values[1] * Q14_SCALAR));
    ensData.data.acceleration_ms2_q14[2] = N_TO_B_ENDIAN_2((int16_t)(values[2] * Q14_SCALAR));
    ensData.data.angularVel_dps_q7[0] = N_TO_B_ENDIAN_2((int16_t)(values[3] * Q7_SCALAR));
    ensData.data.angularVel_dps_q7[1] = N_TO_B_ENDIAN_2((int16_t)(values[4] * Q7_SCALAR));
    ensData.data.angularVel_dps_q7[2] = N_TO_B_ENDIAN_2((int16_t)(values[5] * Q7_SCALAR));
    ensData.data.magIntensity_uT_q3[0] = N_TO_B_ENDIAN_2((int16_t)(values[6] * Q3_SCALAR));
    ensData.data.magIntensity_uT_q3[1] = N_TO_B_ENDIAN_2((int16_t)(values[7] * Q3_SCALAR));
    ensData.data.magIntensity_uT_q3[2] = N_TO_B_ENDIAN_2((int16_t)(values[8] * Q3_SCALAR));

    ensData.header.ensembleType = ENS_TEMP_HIGH_DATA_RATE_IMU;
    ensData.header.elapsedTime_ds = Ens_getStartTime();

    pSystemDesc->pRecorder->putBytes(&ensData,
                                     sizeof(EnsembleHeader_t) + sizeof(Ensemble12_data_t));

#endif
}
/** @} */

/** @} */
