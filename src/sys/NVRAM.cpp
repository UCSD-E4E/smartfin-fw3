#include "NVRAM.hpp"

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
