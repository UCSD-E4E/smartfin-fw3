#include "mfgTest.hpp"

#include "cellular/sf_cloud.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"
#include "product.hpp"
#include "system.hpp"

#include <Particle.h>
#include <cmath>
#include <errno.h>
#include <stdint.h>

MfgTest::mfg_test_entry MfgTest::MFG_TEST_TABLE[] = {
    {&MfgTest::wet_dry_sensor_test, "Wet/Dry Sensor", MfgTest::PENDING},
    {&MfgTest::temperature_sensor_test, "Temperature Sensor", MfgTest::PENDING},
    {&MfgTest::imu_test, "IMU", MfgTest::PENDING},
    {&MfgTest::cellular_test, "Cellular", MfgTest::PENDING},
    {&MfgTest::gps_test, "GPS", MfgTest::PENDING},
    {nullptr, nullptr, MfgTest::PENDING}};

void MfgTest::run(void)
{
    mfg_test_entry *test_entry = nullptr;
    String deviceID = System.deviceID();
    int retval = 0;

    SF_OSAL_printf("Starting Manufacturing Testing" __NL__);
    SF_OSAL_printf("Testing Device %s" __NL__, deviceID.c_str());

    for (test_entry = MFG_TEST_TABLE; test_entry->fn; test_entry++)
    {
        test_entry->pass = (*test_entry->fn)();
    }

    SF_OSAL_printf("| %24s | %16s |" __NL__, "Sensor", "Pass (0)/Fail (1)");
    SF_OSAL_printf("|--------------------------|------------------|" __NL__);
    for (test_entry = MFG_TEST_TABLE; test_entry->fn; test_entry++)
    {
        SF_OSAL_printf("| %24s | %16d |" __NL__, test_entry->name, test_entry->pass);
        retval |= (int)(test_entry->pass);
    }

    if (retval)
    {
        SF_OSAL_printf("Manufacturing Tests FAILED" __NL__ "Mark unit as scrap." __NL__);
    }
    else
    {
        SF_OSAL_printf("All tests passed." __NL__);
    }
}

MfgTest::MFG_TEST_RESULT_t MfgTest::wet_dry_sensor_test(void)
{
    MFG_TEST_RESULT_t retval = MFG_TEST_RESULT_t::PASS;

    pSystemDesc->pWaterCheck->stop();

    SF_OSAL_printf("Running the Wet/Dry Sensor" __NL__);
    SF_OSAL_printf("Internally shorting the Wet/Dry Sensor" __NL__);

    pSystemDesc->pWaterSensor->resetArray();
    // change window to small window (smaller moving average for quick detect)
    pSystemDesc->pWaterSensor->setWindowSize(WATER_DETECT_SURF_SESSION_INIT_WINDOW);
    // set the initial state to "not in water" (because hystersis)
    pSystemDesc->pWaterSensor->forceState(WATER_SENSOR_LOW_STATE);
    // set in-water
    digitalWrite(WATER_MFG_TEST_EN, HIGH);

    SF_OSAL_printf("value: %d " , digitalRead(WATER_MFG_TEST_EN));

    for (int i = 0; i < 100; i++)
    {
        pSystemDesc->pWaterSensor->takeReading();
    }

    if (!pSystemDesc->pWaterSensor->getCurrentStatus())
    {
        SF_OSAL_printf("Wet Sensor failed" __NL__);
        retval = MFG_TEST_RESULT_t::FAIL;
    }
    else
    {
        SF_OSAL_printf("Wet Sensor passed" __NL__);
    }

    // set out-of-water
    digitalWrite(WATER_MFG_TEST_EN, LOW);
    SF_OSAL_printf("value: %d " , digitalRead(WATER_MFG_TEST_EN));



    //Take 100 readings, then query the status
    for (int i = 0; i < 100; i++)
    {
        pSystemDesc->pWaterSensor->takeReading();
    }

    if (pSystemDesc->pWaterSensor->getCurrentStatus())
    {
        SF_OSAL_printf("Dry Sensor failed" __NL__);
        retval = MFG_TEST_RESULT_t::FAIL;
    }
    else
    {
        SF_OSAL_printf("Dry Sensor passed" __NL__);
    }

    digitalWrite(WATER_MFG_TEST_EN, LOW);

    pSystemDesc->pWaterCheck->start();


    return retval;
}


MfgTest::MFG_TEST_RESULT_t MfgTest::temperature_sensor_test(void)
{
    float temp;
    MFG_TEST_RESULT_t retval = MFG_TEST_RESULT_t::PASS;

    SF_OSAL_printf("Running the Temp Test" __NL__);

    // if (!pSystemDesc->pTempSensor->init())
    // {
    //     SF_OSAL_printf("Failed to init temp sensor" __NL__);
    //     retval = MFG_TEST_RESULT_t::FAIL;
    // }

    temp = pSystemDesc->pTempSensor->getTemp();

    if ((temp >= MFG_MIN_VALID_TEMPERATURE) && (temp <= MFG_MAX_VALID_TEMPERATURE))
    {
        SF_OSAL_printf("Temp passed: Temp %f" __NL__, temp);
    }
    else
    {
        SF_OSAL_printf("Temp failed: Temp %f" __NL__);
        retval = MFG_TEST_RESULT_t::FAIL;
    }

    // pSystemDesc->pTempSensor->stop();

    return retval;
}

float _std_dev(float x, float x2, std::size_t n)
{
    return sqrtf((n * x2 - x * x) / (n * (n - 1)));
}

void _print_axis(const char *name, float mean, float std_dev)
{
    SF_OSAL_printf("| %16s | %16.2f | %16.2f |" __NL__, name, mean, std_dev);
}

MfgTest::MFG_TEST_RESULT_t MfgTest::imu_test(void)
{
    float accel[3];
    float rotvel[3];
    float mag[3];

    float accel_acc[3] = {0};
    float accel_acc2[3] = {0};
    float gyro_acc[3] = {0};
    float gyro_acc2[3] = {0};
    float mag_acc[3] = {0};
    float mag_acc2[3] = {0};
    bool fail_flag = false;
    const std::size_t nIterations = 100;
    SF_OSAL_printf("Running IMU Test" __NL__);

    for (std::size_t idx = 0; idx < nIterations; idx++)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        fail_flag |= pSystemDesc->pIMU->getDmpAccel_ms2(accel[0], accel[1], accel[2]);
        fail_flag |= pSystemDesc->pIMU->getDmpRotVel_dps(rotvel[0], rotvel[1], rotvel[2]);
        fail_flag |= pSystemDesc->pIMU->getDmpMag_uT(mag[0], mag[1], mag[2]);
#endif

        accel_acc[0] += accel[0];
        accel_acc[1] += accel[1];
        accel_acc[2] += accel[2];
        gyro_acc[0] += rotvel[0];
        gyro_acc[1] += rotvel[1];
        gyro_acc[2] += rotvel[2];
        mag_acc[0] += mag[0];
        mag_acc[1] += mag[1];
        mag_acc[2] += mag[2];

        accel_acc2[0] += accel[0] * accel[0];
        accel_acc2[1] += accel[1] * accel[1];
        accel_acc2[2] += accel[2] * accel[2];
        gyro_acc2[0] += rotvel[0] * rotvel[0];
        gyro_acc2[1] += rotvel[1] * rotvel[1];
        gyro_acc2[2] += rotvel[2] * rotvel[2];
        mag_acc2[0] += mag[0] * mag[0];
        mag_acc2[1] += mag[1] * mag[1];
        mag_acc2[2] += mag[2] * mag[2];
        delay(5);
    }
    if (fail_flag)
    {
        SF_OSAL_printf("IMU Test failed: Return flag" __NL__);
        return MfgTest::FAIL;
    }
    float accel_mean[3];
    float gyro_mean[3];
    float mag_mean[3];

    accel_mean[0] = accel_acc[0] / nIterations;
    accel_mean[1] = accel_acc[1] / nIterations;
    accel_mean[2] = accel_acc[2] / nIterations;
    gyro_mean[0] = gyro_acc[0] / nIterations;
    gyro_mean[1] = gyro_acc[1] / nIterations;
    gyro_mean[2] = gyro_acc[2] / nIterations;
    mag_mean[0] = mag_acc[0] / nIterations;
    mag_mean[1] = mag_acc[1] / nIterations;
    mag_mean[2] = mag_acc[2] / nIterations;

    float accel_std[3], gyro_std[3], mag_std[3];
    accel_std[0] = _std_dev(accel_acc[0], accel_acc2[0], nIterations);
    accel_std[1] = _std_dev(accel_acc[1], accel_acc2[1], nIterations);
    accel_std[2] = _std_dev(accel_acc[2], accel_acc2[2], nIterations);
    gyro_std[0] = _std_dev(gyro_acc[0], gyro_acc2[0], nIterations);
    gyro_std[1] = _std_dev(gyro_acc[1], gyro_acc2[1], nIterations);
    gyro_std[2] = _std_dev(gyro_acc[2], gyro_acc2[2], nIterations);
    mag_std[0] = _std_dev(mag_acc[0], mag_acc2[0], nIterations);
    mag_std[1] = _std_dev(mag_acc[1], mag_acc2[1], nIterations);
    mag_std[2] = _std_dev(mag_acc[2], mag_acc2[2], nIterations);

    SF_OSAL_printf("| %16s | %16s | %16s |" __NL__, "Channel", "Mean", "StdDev");
    SF_OSAL_printf("|------------------|------------------|------------------|" __NL__);
    _print_axis("Acc.X", accel_mean[0], accel_std[0]);
    _print_axis("Acc.Y", accel_mean[1], accel_std[1]);
    _print_axis("Acc.Z", accel_mean[2], accel_std[2]);
    _print_axis("Gyr.X", gyro_mean[0], gyro_std[0]);
    _print_axis("Gyr.Y", gyro_mean[1], gyro_std[1]);
    _print_axis("Gyr.Z", gyro_mean[2], gyro_std[2]);
    _print_axis("Mag.X", mag_mean[0], mag_std[0]);
    _print_axis("Mag.X", mag_mean[1], mag_std[1]);
    _print_axis("Mag.X", mag_mean[2], mag_std[2]);
    if (accel_mean[0] == 0 || accel_mean[1] == 0 || accel_mean[2] == 0)
    {
        return MfgTest::FAIL;
    }
    if (gyro_mean[0] == 0 || gyro_mean[1] == 0 || gyro_mean[2] == 0)
    {
        return MfgTest::FAIL;
    }
    if (mag_mean[0] == 0 || mag_mean[1] == 0 || mag_mean[2] == 0)
    {
        return MfgTest::FAIL;
    }
    return MfgTest::PASS;
}

MfgTest::MFG_TEST_RESULT_t MfgTest::cellular_test(void)
{
    SF_OSAL_printf("Running cellular test" __NL__);
    sf::cloud::initialize_counter();
    if (!sf::cloud::is_connected())
    {
        if (sf::cloud::wait_connect(MANUFACTURING_CELL_TIMEOUT_MS))
        {
            SF_OSAL_printf("Fail" __NL__);
            return MfgTest::FAIL;
        }
    }
    Particle.syncTime();
    system_tick_t start = millis();
    while (millis() < start + MANUFACTURING_CELL_TIMEOUT_MS || !Particle.syncTimeDone())
    {
        delay(1);
    }
    SF_OSAL_printf("Pass" __NL__);
    sf::cloud::wait_disconnect(MANUFACTURING_CELL_TIMEOUT_MS);
    return MfgTest::PASS;
}

MfgTest::MFG_TEST_RESULT_t MfgTest::gps_test(void)
{
    SF_OSAL_printf("Running GPS test" __NL__);
    if (!pSystemDesc->pLocService->isActive())
    {
        SF_OSAL_printf("Location service is not active!" __NL__);
        return MfgTest::FAIL;
    }
    SF_OSAL_printf("Location service pass" __NL__);
    return MfgTest::PASS;
}