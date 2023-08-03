/**
 * Project smartfin-fw3
 * Description: Debug CLI commands
 * @file debugCommands.cpp
 * @author @emilybthorpe
 * @date Jul 20 2023 
 * 
*/
#include "debugCommands.hpp"

#include "system.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"

#include "Particle.h"
#include "consts.hpp"


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


void CLI_monitorIMU(void)
{
    float accelData[3];
    float gyroData[3];

    char ch;

    if(!pSystemDesc->pIMU->open())
    {
        SF_OSAL_printf("IMU Fail\n");
    }

    while(1)
    {
		if(kbhit()) 
		{
			ch = getch();

			if('q' == ch) 
			{
                break;
			}
		}

        pSystemDesc->pIMU->get_accelerometer(accelData, accelData + 1, accelData + 2);

        pSystemDesc->pIMU->get_gyroscope(gyroData, gyroData + 1, gyroData + 2);

        SF_OSAL_printf("Acceleromter: %8.4f\t%8.4f\t%8.4f" __NL__, accelData[0], accelData[1], accelData[2]);
        SF_OSAL_printf("Gyroscope %8.4f\t%8.4f\t%8.4f" __NL__, gyroData[0], gyroData[1], gyroData[2]);
        delay(500);
	}

    SF_OSAL_printf(__NL__);

    pSystemDesc->pIMU->close();
}