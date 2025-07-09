/**
 * @file tempCal.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Temperature Calibration Task
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "tempCal.hpp"

#include "Particle.h"
#include "cellular/sf_cloud.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"
#include "deploy/ensembleTypes.hpp"
#include "product.hpp"
#include "system.hpp"
#include "util.hpp"

void TempCalTask::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_TMP_CAL" __NL__);
    this->ledStatus.setColor(SF_TCAL_RGB_LED_COLOR);
    this->ledStatus.setPattern(LED_PATTERN_SOLID);
    this->ledStatus.setPriority(SF_TCAL_RGB_LED_PRIORITY);
    this->ledStatus.setActive();

    // Wait to sync time
    if (!sf::cloud::is_connected())
    {
        if (sf::cloud::wait_connect(120000))
        {
            SF_OSAL_printf("Failed to connect!" __NL__);
        }
    }
    Particle.syncTime();
    system_tick_t start = millis();
    while (millis() < start + 120000 || !Particle.syncTimeDone())
    {
        delay(1);
    }
    if (!Particle.syncTimeDone())
    {
        SF_OSAL_printf("Failed to sync time!" __NL__);
    }
    this->ledStatus.setPattern(SF_TCAL_RGB_LED_PATTERN);
    this->ledStatus.setPeriod(SF_TCAL_RGB_LED_PERIOD / 10);
    this->ledStatus.setActive();

    // 5 minute delay to get it into the water
    delay(5 * 60 * 1000);
    this->ledStatus.setPeriod(SF_TCAL_RGB_LED_PERIOD);

    pSystemDesc->pNvram->get(NVRAM::TMP_CAL_BURST_COUNT, this->burst_limit);
    pSystemDesc->pNvram->get(NVRAM::TMP_CAL_MEAS_COUNT, this->measurement_limit);
    pSystemDesc->pNvram->get(NVRAM::TMP_CAL_BURST_PERIOD, this->burst_period_s);
    pSystemDesc->pNvram->get(NVRAM::TMP_CAL_MEAS_PERIOD, this->measurement_period_s);
    this->burst_idx = 0;
    this->measurement_idx = 0;
    if (!pSystemDesc->pRecorder->openSession())
    {
        SF_OSAL_printf("Failed to open session!" __NL__);
    }
    if (Time.isValid())
    {
        pSystemDesc->pRecorder->setSessionTime(Time.now());
    }
}

STATES_e TempCalTask::run(void)
{
    system_tick_t start = millis();
    system_tick_t nextMeasurement, nextBurst;
    for (this->burst_idx = 0; this->burst_idx < this->burst_limit; this->burst_idx++)
    {
        for (this->measurement_idx = 0; this->measurement_idx < this->measurement_limit;
             this->measurement_idx++)
        {
            SF_OSAL_printf("i0: %u, i1: %u" __NL__, this->burst_idx, this->measurement_idx);
            this->do_measure();
            if (this->measurement_idx + 1 < this->measurement_limit)
            {
                nextMeasurement = (this->measurement_idx + 1) * this->measurement_period_s * 1000 +
                                  this->burst_idx * this->burst_period_s * 1000 + start;
                this->sleep_until(nextMeasurement);
            }
        }
        if (this->burst_idx + 1 < this->burst_limit)
        {
            nextBurst = (this->burst_idx + 1) * this->burst_period_s * 1000 + start;
            this->sleep_until(nextBurst);
        }
    }
    return STATE_DEEP_SLEEP;
}

void TempCalTask::exit(void)
{
    pSystemDesc->pRecorder->closeSession();
    this->ledStatus.setActive(false);
}

void TempCalTask::sleep_until(system_tick_t timestamp)
{
    system_tick_t now = millis();
    if (now >= timestamp)
    {
        // timestamp is in the past
        return;
    }
    system_tick_t ms_to_sleep = timestamp - now;
    if (ms_to_sleep < 10 * 60 * 1000)
    {
        delay(ms_to_sleep);
    }
    else
    {
        this->ledStatus.setActive(false);
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        SystemSleepConfiguration config;
        config.mode(SystemSleepMode::STOP).duration(ms_to_sleep);
        System.sleep(config);
#else
        delay(ms_to_sleep);
#endif
        this->ledStatus.setActive(true);
    }
}

void TempCalTask::do_measure(void)
{
    std::uint32_t timestamp = Time.now();
    float battery_voltage = pSystemDesc->pBattery->getVCell();
    float temperature = pSystemDesc->pTempSensor->getTemp();
    std::uint8_t water = pSystemDesc->pWaterSensor->getCurrentReading();
    std::int16_t temp = ((water) ? temperature : (temperature - 100)) / 0.0078125;
#pragma pack(push, 1)
    struct
    {
        EnsembleHeader_t header;
        Ensemble07_data_t data;
    } ens07;
    struct {
        EnsembleHeader_t header;
        Ensemble08_data_t data;
    } ens08;
#pragma pack(pop)
    ens07.header.elapsedTime_ds = Ens_getStartTime();
    ens08.header.elapsedTime_ds = Ens_getStartTime();
    ens07.header.ensembleType = ENS_BATT;
    ens08.header.ensembleType = ENS_TEMP_TIME;
    ens07.data.batteryVoltage = N_TO_B_ENDIAN_2((std::uint16_t)(battery_voltage * 1000));
    ens08.data.rawTemp = N_TO_B_ENDIAN_2(temp);
    ens08.data.timestamp = N_TO_B_ENDIAN_4(timestamp);

    pSystemDesc->pRecorder->putData(ens07);
    pSystemDesc->pRecorder->putData(ens08);
}