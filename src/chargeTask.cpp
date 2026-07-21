#include "chargeTask.hpp"

#include "cellular/sf_cloud.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "platform/hal.hpp"
#include "system.hpp"

static void byteshiftl(void* pData, size_t dataLen, size_t nPos, uint8_t fill);

void ChargeTask::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_CHARGING" __NL__);
    pSystemDesc->pChargerCheck->start();
    this->ledStatus.setColor(CHARGE_RGB_LED_COLOR);
    this->ledStatus.setPattern(CHARGE_RGB_LED_PATTERN);
    this->ledStatus.setPeriod(CHARGE_RGB_LED_PERIOD);
    this->ledStatus.setPriority(CHARGE_RGB_LED_PRIORITY);
    this->ledStatus.setActive();
    this->startTime = SF_HAL::millis();

    //referenced dataupload
    this->initSuccess = 1;
    //RGB connection test
    if (sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS))
    {
        this->initSuccess = 0;
        //this->ledStatus.setColor(RGB_COLOR_RED);
    }
    // else if(sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS) == 0){
    //     this->ledStatus.setColor(RGB_COLOR_BLUE);
    // }
    SF_HAL::cloud_sync_time();
}

STATES_e ChargeTask::run(void)
{
    while(1)
    {
        SF_HAL::cloud_process();
        if (SF_OSAL_kbhit())
        {
            this->inputBuffer[CLI_BUFFER_LEN - 1] = SF_OSAL_getch();
            byteshiftl(this->inputBuffer, CLI_BUFFER_LEN, 1, 0);
            if(strcmp(this->inputBuffer, CLI_INTERRUPT_PHRASE) == 0)
            {
                return STATE_CLI;
            }
        }

        //Check if currently charging using chargerCheck
        if (!pSystemDesc->flags->hasCharger)
        {
            SF_OSAL_printf("Going to sleep" __NL__);
            return STATE_DEEP_SLEEP;
        }

#if SF_CHARGE_ALLOW_DEPLOY == 1
        if (pSystemDesc->pWaterSensor->getLastReading())
        {
            return STATE_DEPLOYED;
        }
#endif

        SF_HAL::thread_yield();
    }
}

void ChargeTask::exit(void)
{
    if (sf::cloud::wait_disconnect(5000))
    {
        FLOG_AddError(FLOG_CELL_DISCONN_FAIL, 0);
    }

    pSystemDesc->pChargerCheck->stop();
    this->ledStatus.setActive(false);
}

static void byteshiftl(void* pData, size_t dataLen, size_t nPos, uint8_t fill)
{
    uint8_t* pByte = (uint8_t*) pData;
    size_t i;
    for(i = 0; i < dataLen - nPos; i++)
    {
        pByte[i] = pByte[i + nPos];
    }
    for(; i < dataLen; i++)
    {
        pByte[i] = fill;
    }
}
