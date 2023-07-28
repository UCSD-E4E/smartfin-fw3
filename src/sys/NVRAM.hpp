#ifndef __NVRAM_HPP__
#define __NVRAM_HPP__

#include "Particle.h"

#include <cstdint>
class NVRAM
{
    public:
    typedef enum DATA_ID_
    {
        NVRAM_VALID,
        BOOT_BEHAVIOR,
        TMP116_CAL_VALUE,
        TMP116_CAL_CURRENT_ATTEMPT,
        TMP116_CAL_ATTEMPTS_TOTAL,
        TMP116_CAL_DATA_COLLECTION_PERIOD_SEC,
        TMP116_CAL_CYCLE_PERIOD_SEC,
        UPLOAD_REATTEMPTS,
        NO_UPLOAD_FLAG,
        NUM_DATA_IDs
    }DATA_ID_e;

    typedef struct nvramTableEntry_
    {
        DATA_ID_e id;

        uint32_t addr;
        uint32_t len;
    }nvramTableEntry_t;

    const nvramTableEntry_t LAYOUT[NUM_DATA_IDs] = 
    {
        {NVRAM_VALID, 0x0000, sizeof(uint8_t)},
        {BOOT_BEHAVIOR, 0x0001, sizeof(uint8_t)},
        {TMP116_CAL_VALUE, 0x0002, sizeof(uint16_t)},
        {TMP116_CAL_CURRENT_ATTEMPT, 0x0004, sizeof(uint8_t)},
        {TMP116_CAL_ATTEMPTS_TOTAL, 0x0005, sizeof(uint8_t)},
        {TMP116_CAL_DATA_COLLECTION_PERIOD_SEC, 0x0008, sizeof(uint32_t)},
        {TMP116_CAL_CYCLE_PERIOD_SEC, 0x000C, sizeof(uint32_t)},
        {UPLOAD_REATTEMPTS, 0x0014, sizeof(uint8_t)},
        {NO_UPLOAD_FLAG, 0x0015, sizeof(uint8_t)}

    };
    static NVRAM& getInstance(void);

    template <typename T> int get(DATA_ID_e dataID, T& pData)
    {
        uint32_t addr;
        const NVRAM::nvramTableEntry_t* tableEntry;

        tableEntry = this->getTableEntry(dataID);
        if(!tableEntry)
        {
            return 0;
        }

        if(sizeof(pData) != tableEntry->len)
        {
            return 0;
        }

        addr = tableEntry->addr;
        EEPROM.get(addr, pData);
        return 1;
    }
    template <typename T> int put(DATA_ID_e dataID, const T& pData)
    {
        uint32_t addr;
        const NVRAM::nvramTableEntry_t* tableEntry;

        tableEntry = this->getTableEntry(dataID);
        if(!tableEntry)
        {
            return 0;
        }

        if(sizeof(pData) != tableEntry->len)
        {
            return 0;
        }

        addr = tableEntry->addr;
        EEPROM.put(addr, pData);
        return 1;
    }


    private:
    const nvramTableEntry_t* getTableEntry(DATA_ID_e);
    NVRAM(){}
    NVRAM(NVRAM const&);
    void operator=(NVRAM const&);
};
#endif