#include "ensembleTypes.hpp"
#include "product.hpp"
#include "system.hpp"
#include "cli/flog.hpp"
#include "cli/conio.hpp"
#include "util.hpp"


#include "Particle.h"

typedef struct Ensemble10_eventData_
{
    double temperature;
    int32_t water;
    int32_t acc[3];
    int32_t ang[3];
    int32_t mag[3];
    int32_t location[2];
    uint8_t hasGPS;
    uint32_t accumulateCount;
}Ensemble10_eventData_t;

static Ensemble10_eventData_t ensemble10Data;


void SS_ensemble10Func()
{
    int32_t lat, lng;
    float temp;
    uint8_t water;
    
    bool hasGPS = false;
    Ensemble10_eventData_t* pData = &ensemble10Data;

    #pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        union{
            Ensemble10_data_t ens10;
            Ensemble11_data_t ens11;
        }data;
    }ensData;
    #pragma pack(pop)

    // TODO: after merging of sensors, get data from here instead of dummy data
    // Obtain measurements    
    // temp = pSystemDesc->pTempSensor->getTemp();
    // water = pSystemDesc->pWaterSensor->getCurrentReading();


    // pSystemDesc->pIMU->get_accelerometer(accelData, accelData + 1, accelData + 2);
    // pSystemDesc->pIMU->get_accel_raw_data((uint8_t*) accelRawData);

    // pSystemDesc->pIMU->get_gyroscope(gyroData, gyroData + 1, gyroData + 2);
    // pSystemDesc->pIMU->get_gyro_raw_data((uint8_t*) gyroRawData);
    
    // pSystemDesc->pCompass->read(magData, magData + 1, magData + 2);
    // pSystemDesc->pCompass->read((uint8_t*) magRawData);

    // if(pSystemDesc->pGPS->location.isValid() && pSystemDesc->pGPS->location.isUpdated() && (pSystemDesc->pGPS->location.age() < GPS_AGE_VALID_MS))
    // {
    //     hasGPS = true;
    //     lat = pSystemDesc->pGPS->location.lat_int32();
    //     lng = pSystemDesc->pGPS->location.lng_int32();
    // }
    // else
    // {
    //     hasGPS = false;
    //     lat = pData->location[0];
    //     lng = pData->location[1];
    // }

    // Test values:
    temp = 23.0;
    water = 1;
    float accelData[3] = {1, 1, 1};
    float gyroData[3]  = {0,0,0};
    int16_t magData[3] = {-1,-1,-1};

    lat = 13;
    lng = 23;
    hasGPS = true;

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

    // ensData.header.elapsedTime_ds = Ens_getStartTime(pDeployment->startTime);
    // SF_OSAL_printf("Ensemble timestamp: %d\n", ensData.header.elapsedTime_ds);
    ensData.data.ens10.rawTemp = N_TO_B_ENDIAN_2(temp / 0.0078125);
    ensData.data.ens10.rawAcceleration[0] = N_TO_B_ENDIAN_2(pData->acc[0])  ;
    ensData.data.ens10.rawAcceleration[1] = N_TO_B_ENDIAN_2(pData->acc[1])  ;
    ensData.data.ens10.rawAcceleration[2] = N_TO_B_ENDIAN_2(pData->acc[2])  ;
    ensData.data.ens10.rawAngularVel[0] = N_TO_B_ENDIAN_2(pData->ang[0])  ;
    ensData.data.ens10.rawAngularVel[1] = N_TO_B_ENDIAN_2(pData->ang[1])  ;
    ensData.data.ens10.rawAngularVel[2] = N_TO_B_ENDIAN_2(pData->ang[2])  ;
    ensData.data.ens10.rawMagField[0] = N_TO_B_ENDIAN_2(pData->mag[0])  ;
    ensData.data.ens10.rawMagField[1] = N_TO_B_ENDIAN_2(pData->mag[1])  ;
    ensData.data.ens10.rawMagField[2] = N_TO_B_ENDIAN_2(pData->mag[2])  ;
    ensData.data.ens11.location[0] = N_TO_B_ENDIAN_4(pData->location[0])  ;
    ensData.data.ens11.location[1] = N_TO_B_ENDIAN_4(pData->location[1])  ;


    ensData.header.ensembleType = ENS_TEMP_IMU;
    int x = pSystemDesc->pRecorder->putBytes(&ensData, sizeof(EnsembleHeader_t) + sizeof(Ensemble10_data_t));
    SF_OSAL_printf("Sucsess %d", x);
    
    memset(pData, 0, sizeof(Ensemble10_eventData_t));
    SF_OSAL_printf("Finished setting up data");
}