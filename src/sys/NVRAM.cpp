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


#include "cli/conio.hpp"
#include "consts.hpp"

#include <cstddef>

#include "Particle.h"

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
}
