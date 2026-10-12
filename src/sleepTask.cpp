#include "sleepTask.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "platform/hal.hpp"
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

    if (SF_HAL::gpio_read(SF_HAL::PinId::UsbPwrDetect))
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
    SF_HAL::gpio_set_mode(SF_HAL::PinId::WaterStatusLed, SF_HAL::GpioMode::OUTPUT);
    SF_HAL::gpio_write(SF_HAL::PinId::WaterStatusLed, SF_HAL::GpioState::LOW);

    // Set WATER_EN LOW so that we can wake from it
    SF_HAL::gpio_write(SF_HAL::PinId::WaterDetectEn, SF_HAL::GpioState::LOW);

    FLOG_AddError(FLOG_SYS_SLEEP, SF_HAL::millis());
    FLOG_AddError(FLOG_SYS_SLEEP, behavior);

    switch(behavior)
    {
        case BOOT_BEHAVIOR_UPLOAD_REATTEMPT:

            SF_OSAL_printf("REUPLOAD" __NL__);
            if (SF_HAL::gpio_read(SF_HAL::PinId::Wkp))
            {
                SF_HAL::system_sleep(SF_HAL::SleepMode::SOFT_POWER_OFF);
                break;
            }
            else
            {
                SF_OSAL_printf("Waking up in %ld seconds...ZZZzzzzz" __NL__,
                               SF_UPLOAD_REATTEMPT_DELAY_SEC);
                SF_HAL::system_sleep(SF_HAL::SleepMode::SOFT_POWER_OFF,
                                     SF_UPLOAD_REATTEMPT_DELAY_SEC);
                break;
            }
        default:
            SF_HAL::gpio_write(SF_HAL::PinId::Wkp, SF_HAL::GpioState::LOW);
            SF_HAL::system_sleep_gpio_wake(SF_HAL::PinId::Wkp);
            break;
    }

    //safety
    SF_OSAL_printf("System going down!" __NL__);
    SF_HAL::system_reset();
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

    while (SF_HAL::gpio_read(SF_HAL::PinId::UsbPwrDetect))
    {
        SF_HAL::cloud_process();
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
        SF_HAL::delay_ms(10);
    }

    if (userChosenCharge)
    {
        return STATE_CHARGE;
    }

    if (!SF_HAL::gpio_read(SF_HAL::PinId::UsbPwrDetect))
    {
        SF_OSAL_printf(__NL__ "USB disconnected! Resetting state machine..." __NL__);
    }

    return SF_DEFAULT_STATE;
}

void SleepTask::exit(void)
{
    // Deactivate sleep LED pattern so it doesn't linger into subsequent states (e.g. CLI or Charge)
    this->ledStatus.setActive(false);
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
