/**
 * Project smartfin-fw3
 * Description: Debug CLI commands
 * @file debugCommands.cpp
 * @author @emilybthorpe
 * @date Jul 20 2023 
 * 
*/
#include "debugCommands.hpp"

#include "Particle.h"
#include "cellular/recorder.hpp"
#include "cellular/sf_cloud.hpp"
#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "fileCLI/fileCLI.hpp"
#include "imu/imu.hpp"
#include "product.hpp"
#include "states.hpp"
#include "system.hpp"

#include <dirent.h>
#include <fcntl.h>

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
    bool hasData = pSystemDesc->pRecorder->hasData();
    SF_OSAL_printf("Has data: %d", hasData);
}

void CLI_testGetNumFiles(void)
{
    int numFiles = pSystemDesc->pRecorder->getNumFiles();
    SF_OSAL_printf("Number of Files: %d", numFiles);
}

void CLI_createTestFile(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    int fd = open("/testfile.txt", O_RDWR | O_CREAT | O_TRUNC);
    SF_OSAL_printf("Error: %d" __NL__, errno);
    SF_OSAL_printf("fd sucsess %d" __NL__, fd);
    
    if (fd != -1) {
        for(int ii = 0; ii < 100; ii++) {
            String msg = String::format("testing %d\n", ii);
            SF_OSAL_printf("Creating file with msg %s" __NL__, msg.c_str());

            int i = write(fd, msg.c_str(), msg.length());
            SF_OSAL_printf("Sucsess: %d" __NL__, i);
        }
        close(fd);
    }
#endif
}

void CLI_wipeFileSystem(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    DIR* directory = opendir("");
    if (directory == 0)
    {
        closedir(directory);
    } 

    while (readdir(directory) != NULL)
    {
        unlink(readdir(directory)->d_name);
    }
#endif
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

void CLI_monitorTempSensor(void) 
{
    float temp;
    char ch;

    SF_OSAL_printf("starting temp sensor");

    if(pSystemDesc->pTempSensor->init() != 0)
    {
        SF_OSAL_printf("Temp Fail" __NL__);
        pSystemDesc->pTempSensor->stop();
        return;
    }

    SF_OSAL_printf("Started" __NL__);

    while(1) 
    {
        if (SF_OSAL_kbhit())
        {
            ch = SF_OSAL_getch();
            if('q' == ch) 
            {
                break;
            }
        }
        
        temp = pSystemDesc->pTempSensor->getTemp();
        SF_OSAL_printf("Temperature Reading: %f" __NL__, temp);
    }

    SF_OSAL_printf(__NL__);
    pSystemDesc->pTempSensor->stop();
}

void CLI_monitorIMU(void)
{
    char ch;

    float accelData[3] = {0,0,0};
    float gyroData[3] = {0,0,0};
    float magData[3] = {0,0,0};
    float tmpData = 0;

    // setupICM();
    // while(1)
    // {
    //     if (SF_OSAL_kbhit())
    //     {
    //         ch = SF_OSAL_getch();

    //         if('q' == ch)
    //         {
    //             break;
    //         }
    //     }

    //     getAccelerometer(accelData, accelData + 1, accelData + 2);
    //     getGyroscope(gyroData, gyroData + 1, gyroData + 2);
    //     getMagnetometer(magData, magData + 1, magData + 2);
    //     getTemperature(&tmpData);

    //     SF_OSAL_printf("Acceleromter: %8.4f\t%8.4f\t%8.4f" __NL__, accelData[0], accelData[1],
    //     accelData[2]); SF_OSAL_printf("Gyroscope: %8.4f\t%8.4f\t%8.4f" __NL__, gyroData[0],
    //     gyroData[1], gyroData[2]); SF_OSAL_printf("Magnetometor:  %8.4f\t%8.4f\t%8.4f" __NL__,
    //     magData[0], magData[1], magData[2]); SF_OSAL_printf("Temperature: %8.4f" __NL__,
    //     tmpData); delay(500);
    // }
}

void CLI_doMfgTest(void)
{
    CLI_nextState = STATE_MFG_TEST;
}
  

void CLI_monitorWetDry(void)
{
    uint8_t waterDetect;
    uint8_t water_status;
    char ch;

    SF_OSAL_printf("Reading    Status" __NL__);
    while(1)
    {
        if (SF_OSAL_kbhit())
        {
            ch = SF_OSAL_getch();

            if('q' == ch) 
            {
                break;
            }
        }
    
        waterDetect = pSystemDesc->pWaterSensor->getLastReading();
        water_status = pSystemDesc->pWaterSensor->getLastStatus();

        SF_OSAL_printf("%10d %6d\r", waterDetect, water_status);

        delay(500);
    }
    SF_OSAL_printf(__NL__);
}

void CLI_fileCLI(void)
{
    FileCLI cli;
    cli.execute();
}

void CLI_initCloudCounters(void)
{
    sf::cloud::initialize_counter();
}

void CLI_dumpIMURegs(void)
{
    setupICM();
    IMU_dumpRegs();
}