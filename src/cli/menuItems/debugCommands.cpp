/**
 * Project smartfin-fw3
 * Description: Debug CLI commands
 * @file debugCommands.cpp
 * @author @emilybthorpe
 * @date Jul 20 2023 
 * 
*/
#include "debugCommands.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include <fcntl.h>

#include "Particle.h"
#include "consts.hpp"
#include "cellular/recorder.hpp"
#include "system.hpp"


void CLI_restart(void)
{
    System.reset();
}

void CLI_displayFLOG(void)
{
    FLOG_DisplayLog();
}

void CLI_clearFLOG(void)
{
    FLOG_ClearLog();
}

void CLI_testHasData(void)
{
    int hasData = pSystemDesc->pRecorder->hasData();
    SF_OSAL_printf("Has data: %d", hasData);
}

void CLI_testGetNumFiles(void)
{
    int numFiles = pSystemDesc->pRecorder->getNumFiles();
    SF_OSAL_printf("Number of Files: %d", numFiles);
}

void CLI_createTestFile(void)
{
    int fd = open("/testfile.txt", O_RDWR | O_CREAT | O_TRUNC);
    SF_OSAL_printf("Error: %d", errno);
    SF_OSAL_printf("fd sucsess %d", fd);
    if (fd != -1) {
        for(int ii = 0; ii < 100; ii++) {
            String msg = String::format("testing %d\n", ii);
            SF_OSAL_printf("Creating file with msg %s", msg);

            int i = write(fd, msg.c_str(), msg.length());
            SF_OSAL_printf("Sucsess: %d", i);
        }
        close(fd);
    }
}

void CLI_testPrintf(void)
{
    char str[32];
    memcpy(str, "Copied string", strlen("Copied string"));
    SF_OSAL_printf("Starting printf tests" __NL__);
    SF_OSAL_printf("Percent: %%" __NL__);
    SF_OSAL_printf("Single Char: %c" __NL__, 'a');
    SF_OSAL_printf("Const Character String: %s" __NL__, "this is a string literal");
    SF_OSAL_printf("Stack Character String: %s" __NL__, str);
    SF_OSAL_printf("Precision Character String: %32s" __NL__, str);
    SF_OSAL_printf("Int: %i" __NL__, 123);
    SF_OSAL_printf("Precision Int: %4i" __NL__, 123);
    SF_OSAL_printf("Zero Prepend Precision Int: %04i" __NL__, 123);
    SF_OSAL_printf("Digit: %d" __NL__, 123);
    SF_OSAL_printf("Precision Digit: %4d" __NL__, 123);
    SF_OSAL_printf("Zero Prepend Precision Digit: %04d" __NL__, 123);
    SF_OSAL_printf("Octal: %o" __NL__, 123);
    SF_OSAL_printf("Precision Octal: %4o" __NL__, 123);
    SF_OSAL_printf("Zero Prepend Precision Octal: %04o" __NL__, 123);
    SF_OSAL_printf("hex: %x" __NL__, 123);
    SF_OSAL_printf("Precision hex: %4x" __NL__, 123);
    SF_OSAL_printf("Zero Prepend Precision hex: %04x" __NL__, 123);
    SF_OSAL_printf("HEX: %X" __NL__, 123);
    SF_OSAL_printf("Precision HEX: %4X" __NL__, 123);
    SF_OSAL_printf("Zero Prepend Precision HEX: %04X" __NL__, 123);

}