/**
 * @file rideTask.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2025-04-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "rideTask.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "deploy/ensembles.hpp"
#include "imu/imu.hpp"
#include "product.hpp"
#include "system.hpp"

/** @brief deployment schedule of ensembles to run
 * @see SCH_getNextEvent
 */
DeploymentSchedule_t deploymentSchedule[] = {
    {SS_fwVerFunc, SS_fwVerInit, 1, UINT32_MAX, 10, 0, "FW VER", {0}},
    {SS_ensemble10Func, SS_ensemble10Init, 1, 1000, 50, 0, "Temp + IMU + GPS", {0}},
    {nullptr, nullptr, 0, 0, 0, 0, nullptr, {0}}};

/**
 * @brief creates file name for log
 * @todo implement RIDE_setFileName
 * @param startTime
 */
static void RIDE_setFileName(system_tick_t startTime)
{
    return;
}
RideTask::RideTask() : scheduler(deploymentSchedule)
{
}

/**
 * @brief initialize ride task
 * Sets LEDs  and initializes schedule
 */
void RideTask::init()
{
    SF_OSAL_printf("Entering STATE_DEPLOYED" __NL__);
    pSystemDesc->pChargerCheck->stop();
    this->ledStatus.setColor(RIDE_RGB_LED_COLOR);
    this->ledStatus.setPattern(RIDE_RGB_LED_PATTERN_GPS);
    this->ledStatus.setPeriod(RIDE_RGB_LED_PERIOD_GPS);
    this->ledStatus.setPriority(RIDE_RGB_LED_PRIORITY);
    this->ledStatus.setActive();

    this->startTime = millis();

    this->scheduler.initializeScheduler();
    pSystemDesc->pRecorder->openSession();
    setupICM();
}

/**
 * @brief runs tasks given by scheduler
 */
STATES_e RideTask::run(void)
{

    DeploymentSchedule_t *pNextEvent = NULL;
    system_tick_t nextEventTime;

    SF_OSAL_printf(__NL__ "Deployment started at %" PRId32 __NL__, millis());

    while (1)
    {
        // Wait for positive in-water signal.  This is run by waterCheck
        if (pSystemDesc->pWaterSensor->getLastStatus())
        {
            break;
        }
        else if (millis() - this->startTime > SURF_SESSION_GET_INTO_WATER_TIMEOUT_MS)
        {
            return STATE_DEEP_SLEEP;
        }
        delay(1000);
    }

    while (1)
    {

        RIDE_setFileName(this->startTime);

        SCH_error_e retval =
            this->scheduler.getNextTask(&pNextEvent, (std::uint32_t *)&nextEventTime, millis());
        // Check if scheduler failed to find nextEvent
        if (TASK_SEARCH_FAIL == retval)
        {
            SF_OSAL_printf("getNextEvent is null! Exiting RideTask..." __NL__);
            FLOG_AddError(FLOG_SCHEDULER_FAILED, TASK_SEARCH_FAIL);
            return STATE_UPLOAD;
        }
        SF_OSAL_printf("Next task is %s at %d" __NL__, pNextEvent->taskName, nextEventTime);
        delay(nextEventTime - millis());
        SF_OSAL_printf("Starts at %" PRId32 __NL__, (std::uint32_t)millis());
        pNextEvent->measure(pNextEvent);
        SF_OSAL_printf("Ends at %" PRId32 __NL__, (std::uint32_t)millis());

        // pNextEvent->lastMeasurementTime = nextEventTime;

        if (pSystemDesc->pWaterSensor->getLastStatus() == WATER_SENSOR_LOW_STATE)
        {
            SF_OSAL_printf("Out of water!" __NL__);
            return STATE_UPLOAD;
        }

        if (pSystemDesc->flags->batteryLow)
        {
            SF_OSAL_printf("Low Battery!" __NL__);
            return STATE_DEEP_SLEEP;
        }
    }
    return STATE_UPLOAD;
}

/**
 * @brief exits ride state
 */
void RideTask::exit(void)
{
    SF_OSAL_printf("Closing session" __NL__);
    pSystemDesc->pRecorder->closeSession();
    pSystemDesc->pChargerCheck->start();
    // Deinitialize sensors
    // pSystemDesc->pTempSensor->stop();
    // pSystemDesc->pCompass->close();
    // pSystemDesc->pIMU->close();
    // pSystemDesc->pGPS->gpsModuleStop();
}
