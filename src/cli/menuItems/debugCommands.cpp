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
#include "product.hpp"

#include "Particle.h"

#include "imu/imu.hpp"
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

void CLI_checkCharging(void) 
{
    SF_OSAL_printf("Charging? %d" __NL__, System.batteryState() == BATTERY_STATE_CHARGING);
    SF_OSAL_printf("Powered? %d" __NL__, digitalRead(SF_USB_PWR_DETECT_PIN));
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
    char ch;

    float accelData[3] = {0,0,0};
    float gyroData[3] = {0,0,0};
    float magData[3] = {0,0,0};
    float tmpData = 0;

    setupICM();
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

        getAccelerometer(accelData, accelData + 1, accelData + 2);
        getGyroscope(gyroData, gyroData + 1, gyroData + 2);
        getMagnetometer(magData, magData + 1, magData + 2);
        getTemperature(&tmpData);

        SF_OSAL_printf("Acceleromter: %8.4f\t%8.4f\t%8.4f" __NL__, accelData[0], accelData[1], accelData[2]);
        SF_OSAL_printf("Gyroscope: %8.4f\t%8.4f\t%8.4f" __NL__, gyroData[0], gyroData[1], gyroData[2]);
        SF_OSAL_printf("Magnetometor:  %8.4f\t%8.4f\t%8.4f" __NL__, magData[0], magData[1], magData[2]);
        SF_OSAL_printf("Temperature: %8.4f" __NL__, tmpData);
        delay(500);
    }

}