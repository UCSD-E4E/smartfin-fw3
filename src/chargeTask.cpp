#include "chargeTask.hpp"

#include "Particle.h"
#include "cli/conio.hpp"
#include "cli/flog.hpp"

#include "system.hpp"
#include "sleepTask.hpp"
#include "consts.hpp"

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
    this->startTime = millis();
}

STATES_e ChargeTask::run(void)
{
    while(1)
    {
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
            FLOG_AddError(FLOG_CHARGER_REMOVED, 0);
            SF_OSAL_printf("Going to sleep" __NL__);
            return STATE_DEEP_SLEEP;
        }

        os_thread_yield();
    }
}

void ChargeTask::exit(void)
{
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
