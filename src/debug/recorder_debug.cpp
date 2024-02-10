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
#include "product.hpp"
#include "util.hpp"

#include <stdint.h>
#include <stdio.h>

void REC_testHasData(void);
void REC_testNumFiles(void);
void REC_testGetLastPacket(void);
void REC_testOpen(void);
void REC_testClose(void);
void REC_testPutBytes(void);
void REC_testSetTime(void);
void REC_testCreateBigSession(void);
void REC_testPopLastPacket(void);

const Menu_t Recorder_debug_menu[] =
{
    {1, "Check Has Data", &REC_testHasData, MENU_CMD},
    {2, "Get Num Files", &REC_testNumFiles, MENU_CMD},
    {3, "Open Session", &REC_testOpen, MENU_CMD},
    {4, "Close Session", &REC_testClose, MENU_CMD},
    {5, "Put Bytes", &REC_testPutBytes, MENU_CMD},
    {6, "Set Session Time", &REC_testSetTime, MENU_CMD},
    {7, "Get Last Packet", &REC_testGetLastPacket, MENU_CMD},
    {8, "Create big session", &REC_testCreateBigSession, MENU_CMD},
    {9, "Pop Last Packet", &REC_testPopLastPacket, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}
};

void REC_testHasData(void)
{
    Recorder* pRecorder = pSystemDesc->pRecorder;
    bool has_data = pRecorder->hasData();
    SF_OSAL_printf("hasData: %d" __NL__, has_data);
    return;
}

void REC_testNumFiles(void)
{
    int n_files = pSystemDesc->pRecorder->getNumFiles();
    SF_OSAL_printf("numFIles: %d" __NL__, n_files);
    return;
}

void REC_testGetLastPacket(void)
{
    uint8_t data_buffer[REC_MEMORY_BUFFER_SIZE];
    char name_buffer[REC_SESSION_NAME_MAX_LEN];
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int retval;

    retval = pRecorder->getLastPacket(data_buffer,
                                      REC_MEMORY_BUFFER_SIZE,
                                      name_buffer,
                                      REC_SESSION_NAME_MAX_LEN);

    if (-1 == retval)
    {
        SF_OSAL_printf("Returned -1" __NL__);
        return;
    }
    hexDump(data_buffer, retval);
    SF_OSAL_printf("Name: %s" __NL__, name_buffer);
    return;
}

void REC_testOpen(void)
{
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int retval;

    if (1 != (retval = pRecorder->openSession()))
    {
        SF_OSAL_printf("Returned %d!" __NL__, retval);
    }
}

void REC_testClose(void)
{
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int retval;

    if (1 != (retval = pRecorder->closeSession()))
    {
        SF_OSAL_printf("Returned %d!" __NL__, retval);
    }
}

void REC_testPutBytes(void)
{
    uint8_t user_input[REC_MEMORY_BUFFER_SIZE];
    char hex_buffer[3];
    size_t hex_idx;
    size_t input_length;
    size_t byte_idx = 0;
    Recorder* pRecorder = pSystemDesc->pRecorder;

    SF_OSAL_printf("Hex Input: ");
    getline((char*) user_input, REC_MEMORY_BUFFER_SIZE);

    memset(hex_buffer, 0, 3);
    input_length = strlen((char*) user_input);
    for (hex_idx = 0; hex_idx < input_length; hex_idx += 2)
    {
        memcpy(hex_buffer, user_input + hex_idx, 2);
        user_input[byte_idx] = (uint8_t) strtoul(hex_buffer, nullptr, 16);
        byte_idx++;
    }

    switch (pRecorder->putBytes(user_input, byte_idx))
    {
    case 0:
        break;
    case 1:
        SF_OSAL_printf("Session not opened!" __NL__);
        break;
    }
}

void REC_testCreateBigSession(void)
{
    char user_input[REC_MEMORY_BUFFER_SIZE];
    uint8_t rand_byte;
    size_t hex_idx;
    size_t input_length;
    Recorder* pRecorder = pSystemDesc->pRecorder;

    SF_OSAL_printf("Size of session to create: ");
    getline((char*) user_input, REC_MEMORY_BUFFER_SIZE);

    input_length = atoi(user_input);
    for (hex_idx = 0; hex_idx < input_length; hex_idx++)
    {
        rand_byte = random(256);
        switch (pRecorder->putBytes(&rand_byte, 1))
        {
        case 0:
            break;
        case 1:
            SF_OSAL_printf("Session not opened!" __NL__);
            break;
        }
    }

}

void REC_testSetTime(void)
{
    char user_input[REC_MEMORY_BUFFER_SIZE];
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int timestamp;
    int retval;

    SF_OSAL_printf("Enter start time: ");
    getline(user_input, REC_MEMORY_BUFFER_SIZE);
    timestamp = atoi(user_input);
    SF_OSAL_printf("Setting time to %d (0x%04x)" __NL__, timestamp, timestamp);
    retval = pRecorder->setSessionTime(timestamp);
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void REC_testPopLastPacket(void)
{
    Recorder* pRecorder = pSystemDesc->pRecorder;
    int retval;
    
    retval = pRecorder->popLastPacket(REC_MAX_PACKET_SIZE);

    SF_OSAL_printf("Returned %d" __NL__, retval);
}