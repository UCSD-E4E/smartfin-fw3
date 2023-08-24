#include "sleepTask.hpp"

#include "Particle.h"
#include "product.hpp"
#include "system.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"

#include "consts.hpp"

SleepTask::BOOT_BEHAVIOR_e SleepTask::bootBehavior;

void SleepTask::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_DEEP_SLEEP" __NL__);
    this->ledStatus.setColor(SLEEP_RGB_LED_COLOR);
    this->ledStatus.setPattern(SLEEP_RGB_LED_PATTERN);
    this->ledStatus.setPeriod(SLEEP_RGB_LED_PERIOD);
    this->ledStatus.setPriority(SLEEP_RGB_LED_PRIORITY);
    this->ledStatus.setActive();
    if(digitalRead(SF_USB_PWR_DETECT_PIN))
    {
        SF_OSAL_printf("USB detected, returning!" __NL__);
        return;
    }

    if(pSystemDesc->flags->batteryLow)
    {
        SleepTask::bootBehavior = BOOT_BEHAVIOR_NORMAL;
    }

    // commit EEPROM before we bring down everything
    pSystemDesc->pNvram->put(NVRAM::BOOT_BEHAVIOR, SleepTask::bootBehavior);
    pSystemDesc->pNvram->put(NVRAM::NVRAM_VALID, true);


    // bring down the system safely
    // SYS_deinitSys(); TODO

    switch(SleepTask::bootBehavior)
    {
        case BOOT_BEHAVIOR_UPLOAD_REATTEMPT:
            SF_OSAL_printf("REUPLOAD" __NL__);
            if(digitalRead(WKP_PIN) == HIGH)
            {
                System.sleep(SLEEP_MODE_SOFTPOWEROFF);
                break;
            }
            else
            {
                SF_OSAL_printf("Waking up in %ld seconds...ZZZzzzzz" __NL__, SF_UPLOAD_REATTEMPT_DELAY_SEC);
                System.sleep(SLEEP_MODE_SOFTPOWEROFF, SF_UPLOAD_REATTEMPT_DELAY_SEC);
                break;
            }
        default:
            digitalWrite(WKP, LOW);
            SystemSleepConfiguration config;
            config.mode(SystemSleepMode::HIBERNATE).gpio(WKP, RISING);
            System.sleep(config);
            break;
    }
    //safety
    SF_OSAL_printf("System going down!" __NL__);
    System.reset();
}

STATES_e SleepTask::run(void)
{
    SF_OSAL_printf("We're supposed to be asleep! Resetting state machine..." __NL__);
    return SF_DEFAULT_STATE;
}

void SleepTask::exit(void)
{
    return;
}

void SleepTask::loadBootBehavior(void)
{
    uint8_t bootValid = 0;
    if(!pSystemDesc->pNvram->get(NVRAM::NVRAM_VALID, bootValid))
    {
        SleepTask::bootBehavior = SleepTask::BOOT_BEHAVIOR_NORMAL;
        return;
    }

    if(bootValid)
    {
        if(!pSystemDesc->pNvram->get(NVRAM::BOOT_BEHAVIOR, SleepTask::bootBehavior))
        {
            SleepTask::bootBehavior = SleepTask::BOOT_BEHAVIOR_NORMAL;
            return;
        }
        bootValid = 0;
        if(!pSystemDesc->pNvram->put(NVRAM::NVRAM_VALID, bootValid))
        {
            SF_OSAL_printf("Failed to clear boot flag" __NL__);
            return;
        }
    }
    else
    {
        SleepTask::bootBehavior = SleepTask::BOOT_BEHAVIOR_NORMAL;
        return;
    }
}

SleepTask::BOOT_BEHAVIOR_e SleepTask::getBootBehavior(void)
{
    return SleepTask::bootBehavior;
}

void SleepTask::setBootBehavior(SleepTask::BOOT_BEHAVIOR_e behavior)
{
    SleepTask::bootBehavior = behavior;
    pSystemDesc->pNvram->put(NVRAM::BOOT_BEHAVIOR, SleepTask::bootBehavior);
    pSystemDesc->pNvram->put(NVRAM::NVRAM_VALID, true);
}

const char* SleepTask::strBootBehavior(BOOT_BEHAVIOR_e behavior)
{
    switch (behavior)
    {
        case BOOT_BEHAVIOR_NORMAL:
            return "Normal";
        case BOOT_BEHAVIOR_TMP_CAL_START:
            return "Temp Cal Start";
        case BOOT_BEHAVIOR_TMP_CAL_CONTINUE:
            return "Temp Cal Continue";
        case BOOT_BEHAVIOR_TMP_CAL_END:
            return "Temp cal End";
        case BOOT_BEHAVIOR_UPLOAD_REATTEMPT:
            return "Upload Reattempt";
    }
    return nullptr;
}