#include "ensembleTypes.hpp"
#include "product.hpp"
#include "system.hpp"
#include "cli/flog.hpp"
#include "cli/conio.hpp"
#include "util.hpp"
#include "imu/imu.hpp"

#include "Particle.h"

/**
 * @brief Ensemble 10 state/accumulation structure
 */
typedef struct Ensemble10_eventData_
{
    /**
     * @brief Average temperature value of the water in Celsius provided
     * by the temperature sensor
     */
    double temperature;
    /**
     * @brief Truncated average of whether or not system in water over
     * accumulation period
     *
     */
    int32_t water;
    /**
     * @brief Array saving average of the accumulated accelerometer data on the
     * x, y, and z axis such that acc[i] = acceleration[i] * 16384, where 
     * `acceleration` is the acceleration in G's (multiple of Earth gravity), 
     * and acc is the stored representation, stored as `int32_t`.Thus, acc[i] =
     * 16384 = 1 G.
     *
     * G is approximately equal to 9.81 m/s^2
     */
    int32_t acc[3];
    /**
     * @brief Array of length 3 saving average of the accumulated gyroscope data
     * on the x, y, and z axis such that ang[i] = angular_velocity * 131.072,
     * where 'angular_velocity' is the angular velocity in degrees per second and
     * ang is the stored representation, stored as `int32_t`. Thus, ang[i] = 
     * 131.072 = 1 degree/second
     *
     */
    int32_t ang[3];
    /**
     * @brief Array of length 3 saving average of the accumulated magnetometer
     * data on the x, y, and z axis in mag[i] = magnetic_field/0.15,
     * where 'magnetic_field' is the magnetic_field in uT and
     * mag is the stored representation, stored as `int32_t`. Thus, mag[i] =
     * 1 = 0.15 uT
     *
     */
    int32_t mag[3];
    /**
     * @brief Array of length 2 saving latitude and longitude values, multiplied 
     * by 1e6, of the point at which data was collected
     */
    int32_t location[2];
    /**
     * @brief Indicates if GNSS point is locked and more than 4 point
     * satellites in view
     *
     * All possible values are [0,1]:
     * 0 - less than 4 point satellites are in view thus GNSS point is not locked
     * 1 - 4 or more point satellites are in view thus GNSS point is locked
     */
    uint8_t hasGPS;
    /**
     * @brief Number of times measurements gathered
     */
    uint32_t accumulateCount;
}Ensemble10_eventData_t;

/**
 * @brief Ensemble 10 state/accumulation data
 */
static Ensemble10_eventData_t ensemble10Data;

/**
 * @brief implementation of Ensemble 10
 */
void SS_ensemble10Func()
{
    int32_t lat, lng;
    float temp;
    uint8_t water;
    float accelData[3] = {0,0,0};
    float gyroData[3] = {0,0,0};
    float magData[3] = {0,0,0};

    LocationPoint point;
    LocationService& instance= LocationService::instance();

    
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

    getAccelerometer(accelData, accelData + 1, accelData + 2);
    getGyroscope(gyroData, gyroData + 1, gyroData + 2);
    getMagnetometer(magData, magData + 1, magData + 2);

    /**
     * Check pre-processer defines to switch between 
     * ICM and MAX temp sensor
     */
    #if USE_ICM_TEMP_SENSOR
        getTemperature(&temp);
    #else 
        temp = pSystemDesc->pTempSensor->getTemp();
    #endif
        
    instance.getLocation(point);

    if(point.locked == 1 && point.satsInView > 4)
    {
        hasGPS = true;
        lat = point.latitude * 1e7;
        lng = point.longitude * 1e7;
    }
    else
    {
        hasGPS = false;
        lat = pData->location[0];
        lng = pData->location[1];
    }

    water = pSystemDesc->pWaterSensor->getCurrentReading();

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
    ensData.data.ens11.location[0] = N_TO_B_ENDIAN_4(pData->location[0]/ 1e7)  ;
    ensData.data.ens11.location[1] = N_TO_B_ENDIAN_4(pData->location[1]/ 1e7)  ;


    ensData.header.ensembleType = ENS_TEMP_IMU;
    int x = pSystemDesc->pRecorder->putBytes(&ensData, sizeof(EnsembleHeader_t) + sizeof(Ensemble10_data_t));
    SF_OSAL_printf("Error code %d", x);
    
    memset(pData, 0, sizeof(Ensemble10_eventData_t));
    SF_OSAL_printf("Finished setting up data");
}