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
#include "imu/newIMU.hpp"
#include "product.hpp"
#include "system.hpp"

/** @brief deployment schedule of ensembles to run
 * @see SCH_getNextEvent
 */
DeploymentSchedule_t deploymentSchedule[] = {
    // measure, init, accumulate, interval, duration, delay, name, state
    {SS_fwVerFunc, SS_fwVerInit, 1, UINT32_MAX, 10, 0, "FW VER", {0}},
    {SS_Ensemble01_Func, SS_Ensemble01_Init, 1, 1000, 10, 0, "Temp", {0}},
#if defined(SF_HIGH_DATA_RATE)
    {SS_HighRateIMU_x0C_Func, SS_HighRateIMU_x0C_Init, 1, 50, 5, 0, "HDR IMU", {0}},
#endif
    // {SS_ensemble10Func, SS_ensemble10Init, 1, 1000, 50, 0, "Temp + IMU + GPS", {0}},
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
    SF_OSAL_printf("Entering STATE_DEPLOYED at %" PRId32 __NL__, millis());
    pSystemDesc->pChargerCheck->stop();
    this->ledStatus.setColor(RIDE_RGB_LED_COLOR);
    this->ledStatus.setPattern(RIDE_RGB_LED_PATTERN_GPS);
    this->ledStatus.setPeriod(RIDE_RGB_LED_PERIOD_GPS);
    this->ledStatus.setPriority(RIDE_RGB_LED_PRIORITY);
    this->ledStatus.setActive();

    this->startTime = millis();

    this->scheduler.initializeScheduler();
    if (!pSystemDesc->pRecorder->openSession())
    {
        SF_OSAL_printf("Failed to open session!" __NL__);
    }
}

/**
 * @brief runs tasks given by scheduler
 */
STATES_e RideTask::run(void)
{

    DeploymentSchedule_t *pNextEvent = NULL;
    system_tick_t nextEventTime;

    unsigned long start, stop;

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
        Particle.process();
        delay(1000);
    }
    SF_OSAL_printf(__NL__ "Deployment started at %" PRId32 __NL__, millis());

    while (1)
    {

        RIDE_setFileName(this->startTime);

        SCH_error_e retval =
            this->scheduler.getNextTask(&pNextEvent, (std::uint32_t *)&nextEventTime, millis());
        // Check if scheduler failed to find nextEvent
        if (TASK_SEARCH_FAIL == retval)
        {
            SF_OSAL_printf("getNextEvent is null! Sleeping forever..." __NL__);
            FLOG_AddError(FLOG_SCHEDULER_FAILED, TASK_SEARCH_FAIL);
            nextEventTime = UINT32_MAX;
        }
        else
        {
            // SF_OSAL_printf("Next task is %s at %d" __NL__, pNextEvent->taskName, nextEventTime);
        }
        while (millis() < nextEventTime)
        {
            Particle.process();
            if (!pSystemDesc->pWaterSensor->getLastStatus())
            {
                SF_OSAL_printf("Out of water!" __NL__);
#if SF_CAN_UPLOAD
                return STATE_UPLOAD;
#else
                return STATE_DEEP_SLEEP;
#endif
            }
            if (pSystemDesc->flags->batteryLow)
            {
                SF_OSAL_printf("Low Battery!" __NL__);
                return STATE_DEEP_SLEEP;
            }
            delay(1);
        }
        // SF_OSAL_printf("Starts at %" PRId32 __NL__, (std::uint32_t)millis());
        start = micros();
        pNextEvent->measure(pNextEvent);
        stop = micros();
        pNextEvent->state.durationAccumulate += stop - start;
        // SF_OSAL_printf("Ends at %" PRId32 __NL__, (std::uint32_t)millis());

        // pNextEvent->lastMeasurementTime = nextEventTime;

        if (pSystemDesc->pWaterSensor->getLastStatus() == WATER_SENSOR_LOW_STATE)
        {
            SF_OSAL_printf("Out of water!" __NL__);
#if SF_CAN_UPLOAD
            return STATE_UPLOAD;
#else
            return STATE_DEEP_SLEEP;
#endif
        }

        if (pSystemDesc->flags->batteryLow)
        {
            SF_OSAL_printf("Low Battery!" __NL__);
            return STATE_DEEP_SLEEP;
        }
    }
#if SF_CAN_UPLOAD
    return STATE_UPLOAD;
#else
    return STATE_DEEP_SLEEP;
#endif
}

/**
 * @brief exits ride state
 */
void RideTask::exit(void)
{
    SF_OSAL_printf("Closing session" __NL__);
    pSystemDesc->pRecorder->closeSession();
    pSystemDesc->pChargerCheck->start();
    for (DeploymentSchedule_t *pEvent = deploymentSchedule; pEvent->measure; pEvent++)
    {
        SF_OSAL_printf("%s Average duration: %d us" __NL__,
                       pEvent->taskName,
                       pEvent->state.durationAccumulate / pEvent->state.measurementCount);
    }
    // Deinitialize sensors
    // pSystemDesc->pTempSensor->stop();
    // pSystemDesc->pCompass->close();
    // pSystemDesc->pIMU->close();
    // pSystemDesc->pGPS->gpsModuleStop();
}
