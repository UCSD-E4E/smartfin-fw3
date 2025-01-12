/**
 * @file session_debug.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Deployment/Session Debug Application
 * @version 0.1
 * @date 2023-09-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "session_debug.hpp"

#include "cli/conio.hpp"
#include "cli/menu.hpp"
#include "cellular/deploy.hpp"
#include "cellular/recorder.hpp"
#include "system.hpp"
#include "util.hpp"

#include <errno.h>

void DEPD_testOpenRW(void);
void DEPD_testOpenRO(void);
void DEPD_testOpenWO(void);
void DEPD_testCloseFile(void);
void DEPD_testWrite(void);
void DEPD_testRead(void);
void DEPD_dumpErrno(void);
void DEPD_testSeek(void);
void DEPD_testGetLength(void);
void DEPD_testRemove(void);
void DEPD_testTruncate(void);

const Menu_t Session_debug_menu[] =
{
    {1, "Open File R/W", DEPD_testOpenRW, MENU_CMD},
    {2, "Open File WO", DEPD_testOpenWO, MENU_CMD},
    {3, "Open File RO", DEPD_testOpenRO, MENU_CMD},
    {4, "Close File", DEPD_testCloseFile, MENU_CMD},
    {5, "Write Data", DEPD_testWrite, MENU_CMD},
    {6, "Read Data", DEPD_testRead, MENU_CMD},
    {7, "Seek File", DEPD_testSeek, MENU_CMD},
    {8, "Get File Length", DEPD_testGetLength, MENU_CMD},
    {9, "Remove Session", DEPD_testRemove, MENU_CMD},
    {10, "Truncate File", DEPD_testTruncate, MENU_CMD},
    {100, "Dump Errno", DEPD_dumpErrno, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}
};

void DEPD_testOpenRW(void)
{
    Deployment& instance = Deployment::getInstance();
    char userInput[REC_SESSION_NAME_MAX_LEN];
    int retval;

    SF_OSAL_printf("Enter path: ");
    SF_OSAL_getline(userInput, REC_SESSION_NAME_MAX_LEN);

    retval = instance.open(userInput, Deployment::RDWR);
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void DEPD_testOpenRO(void)
{
    Deployment& instance = Deployment::getInstance();
    char userInput[REC_SESSION_NAME_MAX_LEN];
    int retval;

    SF_OSAL_printf("Enter path: ");
    SF_OSAL_getline(userInput, REC_SESSION_NAME_MAX_LEN);

    retval = instance.open(userInput, Deployment::READ);
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void DEPD_testOpenWO(void)
{
    Deployment& instance = Deployment::getInstance();
    char userInput[REC_SESSION_NAME_MAX_LEN];
    int retval;

    SF_OSAL_printf("Enter path: ");
    SF_OSAL_getline(userInput, REC_SESSION_NAME_MAX_LEN);

    retval = instance.open(userInput, Deployment::WRITE);
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void DEPD_testWrite(void)
{
    Deployment& instance = Deployment::getInstance();
    char userInput[SF_CLI_MAX_CMD_LEN];
    int retval;

    SF_OSAL_printf("Enter data: ");
    SF_OSAL_getline(userInput, SF_CLI_MAX_CMD_LEN);

    retval = instance.write(userInput, strlen(userInput));
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void DEPD_testCloseFile(void)
{
    Deployment& instance = Deployment::getInstance();
    int retval = instance.close();
    SF_OSAL_printf("Returned %d" __NL__, retval);
}

void DEPD_testRead(void)
{
    Deployment& instance = Deployment::getInstance();
    uint8_t data_buffer[REC_MEMORY_BUFFER_SIZE];
    char userInput[SF_CLI_MAX_CMD_LEN];
    int n_bytes;
    int retval;

    SF_OSAL_printf("Enter number of bytes to read: ");
    SF_OSAL_getline(userInput, SF_CLI_MAX_CMD_LEN);
    n_bytes = atoi(userInput);

    if (n_bytes < 1)
    {
        return;
    }
    retval = instance.read(data_buffer, n_bytes);

    hexDump(data_buffer, retval);
}

void DEPD_dumpErrno(void)
{
    SF_OSAL_printf("errno: %s" __NL__, strerror(errno));
}

void DEPD_testSeek(void)
{
    char userInput[SF_CLI_MAX_CMD_LEN];
    Deployment& instance = Deployment::getInstance();
    size_t n_bytes;
    int retval;

    SF_OSAL_printf("Enter location to seek to: ");
    SF_OSAL_getline(userInput, SF_CLI_MAX_CMD_LEN);
    n_bytes = atoi(userInput);

    retval = instance.seek(n_bytes);
    SF_OSAL_printf("Returned %d" __NL__, retval);

}

void DEPD_testGetLength(void)
{
    Deployment& instance = Deployment::getInstance();
    int length;

    length = instance.getLength();
    SF_OSAL_printf("File Length: %d" __NL__, length);
}

void DEPD_testRemove(void)
{
    Deployment& instance = Deployment::getInstance();
    int retval = instance.remove();
    SF_OSAL_printf("Returned: %d" __NL__, retval);
}

void DEPD_testTruncate(void)
{
    Deployment& instance = Deployment::getInstance();
    size_t n_bytes;
    char userInput[SF_CLI_MAX_CMD_LEN];
    int retval;

    SF_OSAL_printf("Enter size to truncate to: ");
    SF_OSAL_getline(userInput, SF_CLI_MAX_CMD_LEN);
    n_bytes = atoi(userInput);

    retval = instance.truncate(n_bytes);
    SF_OSAL_printf("Returned: %d" __NL__, retval);
}