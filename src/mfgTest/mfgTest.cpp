#include "mfgTest.hpp"

#include <Particle.h>
#include <errno.h>
#include <stdint.h>

#include "system.hpp"
#include "cli/conio.hpp"
#include "product.hpp"

typedef int (MfgTest::*mfg_run_function_t)(void);

typedef struct {
    mfg_run_function_t run_function_ptr;
} mfg_test_entry_t;

int(* (MfgTest::MFG_TEST_TABLE)[])(void) = {
    &MfgTest::wet_dry_sensor_test,
    NULL
};

void MfgTest::init(void){}

STATES_e MfgTest::run(void)
{
    String deviceID = System.deviceID();
    int retval = 0;

    SF_OSAL_printf("\r\nStarting Manufacturing Testing\r\n");
    SF_OSAL_printf("Testing Device %s\r\n", deviceID.c_str());

    for(uint8_t i = 0; MFG_TEST_TABLE[i]; i++)
    {
        retval = MFG_TEST_TABLE[i]();
        if(retval != 0)
        {
            break;
        }
    }

    if(retval)
    {
        SF_OSAL_printf("Manufacturing Tests FAILED.\r\nMark unit as scrap.\r\n\r\n");
    }
    else
    {
        SF_OSAL_printf("All tests passed.\r\n");
    }
    return STATE_CLI;
}

void MfgTest::exit(void){}

int MfgTest::wet_dry_sensor_test(void)
{
    int retval = 0;

    SF_OSAL_printf("Running the Wet/Dry Sensor\r\n");
    SF_OSAL_printf("Internally shorting the Wet/Dry Sensor\r\n");

    pSystemDesc->pWaterSensor->resetArray();
    // change window to small window (smaller moving average for quick detect)
    pSystemDesc->pWaterSensor->setWindowSize(WATER_DETECT_SURF_SESSION_INIT_WINDOW);
    // set the initial state to "not in water" (because hystersis)
    pSystemDesc->pWaterSensor->forceState(WATER_SENSOR_LOW_STATE);
    // set in-water
    digitalWrite(WATER_MFG_TEST_EN, HIGH);

    for (int i = 0; i < 100; i++)
    {
        pSystemDesc->pWaterSensor->takeReading();
    }
    if (!pSystemDesc->pWaterSensor->getCurrentStatus())
    {
        SF_OSAL_printf("Wet Sensor failed\r\n");
        retval = -EIO;
    }
    else
    {
        SF_OSAL_printf("Wet Sensor passed\r\n");
    }
    // set out-of-water
    digitalWrite(WATER_MFG_TEST_EN, LOW);


    //Take 100 readings, then query the status
    for (int i = 0; i < 100; i++)
    {
        pSystemDesc->pWaterSensor->takeReading();
    }
    if (pSystemDesc->pWaterSensor->getCurrentStatus())
    {
        SF_OSAL_printf("Dry Sensor failed\r\n");
        retval = -EIO;
    }
    else
    {
        SF_OSAL_printf("Dry Sensor passed\r\n");
    }

    digitalWrite(WATER_MFG_TEST_EN, LOW);

    return retval;
}


