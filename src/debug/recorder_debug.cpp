/**
 * @file recorder_debug.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Recorder Debug Application
 * @version 0.1
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "recorder_debug.hpp"

#include "cli/conio.hpp"
#include "cli/menu.hpp"
#include "cellular/recorder.hpp"
#include "system.hpp"

void REC_testHasData(void);

const Menu_t Recorder_debug_menu[] = 
{
    {1, "Check Has Data", &REC_testHasData, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}
};

void REC_testHasData(void)
{
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int has_data = pRecorder->hasData();
    SF_OSAL_printf("hasData: %d" __NL__, has_data);
    return;
}
