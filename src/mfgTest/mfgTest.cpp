#include "mfgTest.hpp"

#include <Particle.h>
#include <errno.h>
#include <stdint.h>

#include "system.hpp"
#include "cli/conio.hpp"
#include "product.hpp"
#include "consts.hpp"

const MfgTest::mfg_test_fn MfgTest::MFG_TEST_TABLE[] = {
    &MfgTest::wet_dry_sensor_test,
    &MfgTest::temperature_sensor_test,
    nullptr
};

void MfgTest::init(void)
{
}

STATES_e MfgTest::run(void)
{
    const mfg_test_fn* test_fn = nullptr;
    String deviceID = System.deviceID();
    int retval = 0;

    SF_OSAL_printf("Starting Manufacturing Testing" __NL__);
    SF_OSAL_printf("Testing Device %s" __NL__, deviceID.c_str());

    for(test_fn = MFG_TEST_TABLE; *test_fn; test_fn++)
    {
        retval |= (int)(*test_fn)();
    }

    if(retval)
    {
        SF_OSAL_printf("Manufacturing Tests FAILED" __NL__ "Mark unit as scrap." __NL__);
    }
    else
    {
        SF_OSAL_printf("All tests passed." __NL__);
    }
    return STATE_CLI;
}

void MfgTest::exit(void)
{
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

    pSystemDesc->pTempSensor->init();

    temp = pSystemDesc->pTempSensor->getTemp();

    if ((temp >= MFG_MIN_VALID_TEMPERATURE) && (temp <= MFG_MAX_VALID_TEMPERATURE))
    {
        SF_OSAL_printf("Temp passed: Temp %f" __NL__, temp);
    }
    else
    {
        SF_OSAL_printf("Temp failed" __NL__);
        retval = MFG_TEST_RESULT_t::FAIL;
    }

    pSystemDesc->pTempSensor->stop();

    return retval;
}
