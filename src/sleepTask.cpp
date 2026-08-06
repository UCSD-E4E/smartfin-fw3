#include "sleepTask.hpp"

#include "Particle.h"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "product.hpp"
#include "system.hpp"

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

    SleepTask::BOOT_BEHAVIOR_e behavior;
    if(!pSystemDesc->pNvram->get(NVRAM::BOOT_BEHAVIOR, behavior) || pSystemDesc->flags->batteryLow)
    {
        behavior = BOOT_BEHAVIOR_NORMAL;
    }

    // commit EEPROM before we bring down everything
    pSystemDesc->pNvram->put(NVRAM::BOOT_BEHAVIOR, behavior);
    pSystemDesc->pNvram->put(NVRAM::NVRAM_VALID, true);

    // bring down the system safely
    SYS_deinitSys();

    // Turn off wet/dry LED before sleeping
    pinMode(WATER_STATUS_LED, OUTPUT);
    digitalWrite(WATER_STATUS_LED, LOW);

    // Set WATER_EN LOW so that we can wake from it
    digitalWrite(WATER_DETECT_EN_PIN, LOW);

    FLOG_AddError(FLOG_SYS_SLEEP, millis());
    FLOG_AddError(FLOG_SYS_SLEEP, behavior);
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    switch(behavior)
    {
        case BOOT_BEHAVIOR_UPLOAD_REATTEMPT:

            SF_OSAL_printf("REUPLOAD" __NL__);
            if (digitalRead(WKP_PIN) == HIGH)
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
#endif
    //safety
    SF_OSAL_printf("System going down!" __NL__);
    System.reset();
}

STATES_e SleepTask::run(void)
{
    SF_OSAL_printf(__NL__ "USB connected while entering Deep Sleep." __NL__);
    SF_OSAL_printf("Choose USB sleep behavior:" __NL__);
    SF_OSAL_printf("  1: Remain in Deep Sleep (Black LED test mode)" __NL__);
    SF_OSAL_printf("  2: Go to Charge Mode (Solid Yellow LED)" __NL__);
    SF_OSAL_printf("  q / #CLI: Return to CLI Menu" __NL__);
    SF_OSAL_printf("Select option (defaulting to 1 - Deep Sleep): " __NL__);

    static char cliBuf[6] = {0};
    memset(cliBuf, 0, 6);

    bool userChosenCharge = false;

    while (digitalRead(SF_USB_PWR_DETECT_PIN))
    {
        Particle.process();
        if (SF_OSAL_kbhit())
        {
            int ch = SF_OSAL_getch();
            for (int idx = 0; idx < 4; idx++)
            {
                cliBuf[idx] = cliBuf[idx + 1];
            }
            cliBuf[4] = (char)ch;
            cliBuf[5] = '\0';

            if (ch == '1')
            {
                SF_OSAL_printf(__NL__ "Remaining in Deep Sleep test mode (Black LED active)." __NL__);
                SF_OSAL_printf("Type '#CLI' or 'q' at any time to return to CLI menu." __NL__);
            }
            else if (ch == '2' || ch == 'c')
            {
                SF_OSAL_printf(__NL__ "Switching to Charge Mode..." __NL__);
                userChosenCharge = true;
                break;
            }
            else if (strstr(cliBuf, "#CLI") != NULL || ch == 'q')
            {
                SF_OSAL_printf(__NL__ "CLI interrupt in SleepTask!" __NL__);
                return STATE_CLI;
            }
        }
        if (pSystemDesc->pWaterSensor->getLastStatus() && !pSystemDesc->flags->batteryLow)
        {
            SF_OSAL_printf(__NL__ "In water!" __NL__);
            return STATE_DEPLOYED;
        }
        delay(10);
    }

    if (userChosenCharge)
    {
        return STATE_CHARGE;
    }

    if (!digitalRead(SF_USB_PWR_DETECT_PIN))
    {
        SF_OSAL_printf(__NL__ "USB disconnected! Resetting state machine..." __NL__);
    }

    return SF_DEFAULT_STATE;
}

void SleepTask::exit(void)
{
    return;
}

void SleepTask::loadBootBehavior(void)
{
    uint8_t bootValid;
    if(!pSystemDesc->pNvram->get(NVRAM::NVRAM_VALID, bootValid))
    {
        bootValid = 0;
    }

    if(bootValid)
    {
        bootValid = 0;
        if(!pSystemDesc->pNvram->put(NVRAM::NVRAM_VALID, bootValid))
        {

            SF_OSAL_printf("Failed to clear boot flag" __NL__);
            return;
        }
    }
    else
    {
        pSystemDesc->pNvram->put(NVRAM::BOOT_BEHAVIOR, SleepTask::BOOT_BEHAVIOR_NORMAL);
    }
}

SleepTask::BOOT_BEHAVIOR_e SleepTask::getBootBehavior(void)
{
    SleepTask::BOOT_BEHAVIOR_e behavior;
    if(!pSystemDesc->pNvram->get(NVRAM::BOOT_BEHAVIOR, behavior))
    {
        behavior = BOOT_BEHAVIOR_NORMAL;
    }
    return behavior;
}

void SleepTask::setBootBehavior(SleepTask::BOOT_BEHAVIOR_e behavior)
{
    pSystemDesc->pNvram->put(NVRAM::BOOT_BEHAVIOR, behavior);
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
