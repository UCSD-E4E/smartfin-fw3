/**
 * @file NVRAM.cpp
 * @author 
 * @brief Manages onboard NonVolitaile RAM
 * @version 0.1
 * @date 2023-08-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "NVRAM.hpp"

#include "Particle.h"
#include "cli/conio.hpp"
#include "consts.hpp"

#include <cinttypes>
#include <cstddef>

NVRAM& NVRAM::getInstance(void)
{
    static NVRAM NVRAM_instance;
    return NVRAM_instance;
}

const NVRAM::nvramTableEntry_t* NVRAM::getTableEntry(NVRAM::DATA_ID_e dataID)
{
    int i;
    for(i = 0; i < NVRAM::NUM_DATA_IDs; i++)
    {
        if(this->LAYOUT[i].id == dataID)
        {
            return &this->LAYOUT[i];
        }
    }
    return NULL;
}


void NVRAM::displayNVRAM(void)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    this->get(NVRAM_VALID, u8);
    SF_OSAL_printf("NVRAM Valid: 0x%02hx" __NL__, u8);
    this->get(BOOT_BEHAVIOR, u8);
    SF_OSAL_printf("Boot Behavior: 0x%02hx" __NL__, u8);
    this->get(TMP116_CAL_VALUE, u16);
    SF_OSAL_printf("TMP116 Cal Value: 0x%04hx" __NL__, u16);
    this->get(TMP116_CAL_CURRENT_ATTEMPT, u8);
    SF_OSAL_printf("TMP116 Cal Current Attempt: 0x%02hx" __NL__, u8);
    this->get(TMP116_CAL_ATTEMPTS_TOTAL, u8);
    SF_OSAL_printf("TMP116 Cal Total Attempts: 0x%02hx" __NL__, u8);
    this->get(TMP116_CAL_DATA_COLLECTION_PERIOD_SEC, u32);
    SF_OSAL_printf("TMP116 Cal Data Collection Period (s): 0x%02x" __NL__, u32);
    this->get(TMP116_CAL_CYCLE_PERIOD_SEC, u32);
    SF_OSAL_printf("TMP116 Cal Cycle Period (s): 0x%02x" __NL__, u32);
    this->get(UPLOAD_REATTEMPTS, u8);
    SF_OSAL_printf("Upload Reattempts: 0x%02hx" __NL__, u8);
    this->get(NO_UPLOAD_FLAG, u8);
    SF_OSAL_printf("No Upload Flag: 0x%02hx" __NL__, u8);
    this->get(CLOUD_CONNECT_COUNTER, u16);
    SF_OSAL_printf("Cloud Connect Counter: 0x%04hx" __NL__, u16);
    this->get(WATER_DETECT_WINDOW_LEN, u8);
    SF_OSAL_printf("Water Detect Window Len: 0x%02hx" __NL__, u8);
    this->get(TMP_CAL_BURST_COUNT, u32);
    SF_OSAL_printf("Temp Cal Burst Count: %" PRIu32 __NL__, u32);
    this->get(TMP_CAL_MEAS_COUNT, u32);
    SF_OSAL_printf("Temp Cal Measurement Count: %" PRIu32 __NL__, u32);
    this->get(TMP_CAL_BURST_PERIOD, u32);
    SF_OSAL_printf("Temp Cal Burst Period: %" PRIu32 __NL__, u32);
    this->get(TMP_CAL_MEAS_PERIOD, u32);
    SF_OSAL_printf("Temp Cal Measurement Period: %" PRIu32 __NL__, u32);
}

void NVRAM::resetDefault(void)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

    u8 = true;
    this->put(NVRAM_VALID, u8);

    u8 = 0;
    this->put(BOOT_BEHAVIOR, u8);

    u16 = 0;
    this->put(TMP116_CAL_VALUE, u16);

    u8 = 0;
    this->put(TMP116_CAL_CURRENT_ATTEMPT, u8);

    u8 = 0;
    this->put(TMP116_CAL_ATTEMPTS_TOTAL, u8);

    u32 = 0;
    this->put(TMP116_CAL_DATA_COLLECTION_PERIOD_SEC, u32);

    u32 = 0;
    this->put(TMP116_CAL_CYCLE_PERIOD_SEC, u32);

    u8 = 0;
    this->put(UPLOAD_REATTEMPTS, u8);

    u8 = false;
    this->put(NO_UPLOAD_FLAG, u8);

    u16 = 0;
    this->put(CLOUD_CONNECT_COUNTER, u16);

    u8 = 40;
    this->put(WATER_DETECT_WINDOW_LEN, u8);

    u32 = 14;
    this->put(TMP_CAL_BURST_COUNT, u32);

    u32 = 5 * 60;
    this->put(TMP_CAL_MEAS_COUNT, u32);

    u32 = 60 * 60;
    this->put(TMP_CAL_BURST_PERIOD, u32);

    u32 = 1;
    this->put(TMP_CAL_MEAS_PERIOD, u32);
}