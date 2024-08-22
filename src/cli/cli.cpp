/**
 * Project smartfin-fw3
 * Description: Command Line interface for development, manufacturing tests, and user interaction
 * Author: Emily Thorpe
 * Date: Jul 20 2023 
 * 
*/

#include "cli.hpp"

#include "cliDebug.hpp"

#include "conio.hpp"
#include "consts.hpp"
#include "menu.hpp"

#include "menuItems/systemCommands.hpp"
#include "menuItems/debugCommands.hpp"
#include "menuItems/gpsCommands.hpp"
#include "debug/recorder_debug.hpp"
#include "debug/session_debug.hpp"
#include "imu/imu.hpp"
#include "menuItems/debugCommands.hpp"

#include "states.hpp"
#include "util.hpp"
#include "vers.hpp"
#include "product.hpp"
#include "sleepTask.hpp"
#include "system.hpp"
#include "tracker_config.h"
#include "system.hpp"

#include <fstream>
#include <bits/stdc++.h>
#include "location_service.h"
#include "Particle.h"
#include <string>
#include <iostream>
#include <fstream>

void CLI_displayMenu(void);
void CLI_hexdump(void);

static LEDStatus CLI_ledStatus;


static void CLI_setState(void);
static void CLI_displaySystemState(void);
static void CLI_displayNVRAM(void);
static void CLI_sleepSetSleepBehavior(void);
static void CLI_sleepGetSleepBehavior(void);
static void CLI_displayResetReason(void);
static void CLI_monitorSensors(void);
static void CLI_testTime(void);

const Menu_t CLI_menu[] =
{
    {1, "display Menu", &CLI_displayMenu, MENU_CMD},
    {2, "disconnect particle", &CLI_disconnect, MENU_CMD},
    {3, "connect particle", &CLI_connect, MENU_CMD},
    {4, "show flog errors", &CLI_displayFLOG, MENU_CMD},
    {5, "test printf", &CLI_testPrintf, MENU_CMD},
    {6, "debug menu", {.pMenu=CLI_debugMenu}, MENU_SUBMENU},
    {7, "hexdump", &CLI_hexdump, MENU_CMD},
    {8, "gps", &CLI_GPS, MENU_CMD},
    {9, "sleep", &CLI_doSleep, MENU_CMD},
    {10, "Self Identify", &CLI_self_identify, MENU_CMD},
    {11, "check charge ports", &CLI_checkCharging, MENU_CMD},
    {12, "MFG Test", &CLI_doMfgTest, MENU_CMD},
    {13, "upload", &CLI_doUpload, MENU_CMD},
    {14, "Recorder Test Menu", {.pMenu=Recorder_debug_menu}, MENU_SUBMENU},
    {15, "Session Test Menu", {.pMenu=Session_debug_menu}, MENU_SUBMENU},
    {16, "Display all sensors", &CLI_monitorSensors, MENU_CMD},
    {17, "Test Sampling Time for Sensors", &CLI_testTime, MENU_CMD},
    {100, "Set State", &CLI_setState, MENU_CMD},
    {101, "Display System State", &CLI_displaySystemState, MENU_CMD},
    {102, "Display NVRAM", &CLI_displayNVRAM, MENU_CMD},
    {200, "Sleep - Set Sleep Behavior", &CLI_sleepSetSleepBehavior, MENU_CMD},
    {201, "Sleep - Get Sleep Behavior", &CLI_sleepGetSleepBehavior, MENU_CMD},
    {300, "Display Reset Reason", &CLI_displayResetReason, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}
};


STATES_e CLI_nextState;

void CLI::init(void)
{
    VERS_printBanner();

    CLI_nextState = STATE_CLI;

    pSystemDesc->pChargerCheck->start();

    CLI_ledStatus.setColor(CLI_RGB_LED_COLOR);
    CLI_ledStatus.setPattern(CLI_RGB_LED_PATTERN);
    CLI_ledStatus.setPeriod(CLI_RGB_LED_PERIOD);
    CLI_ledStatus.setPriority(CLI_RGB_LED_PRIORITY);
    CLI_ledStatus.setActive();

    // While there is an avaliable character typed, get it
    while (kbhit())
    {
        getch();
    }
}

STATES_e CLI::run(void)
{
    MNU_executeMenu(CLI_menu);
    return CLI_nextState;
}


void CLI::exit() 
{
    pSystemDesc->pChargerCheck->stop();
}
static void CLI_testTime(void) {
    //SF_OSAL_printf("DMP Data..." __NL__);
    getDMPData();
    //float gyroData[3] = {0,0,0};
    //float magData[3] = {0,0,0};
    //float accelData[3] = {0,0,0};
    // LocationPoint point;
    // LocationService& instance= LocationService::instance();
    
    //     getAccelerometer(accelData, accelData + 1, accelData + 2);
    //pSystemDesc->pTempSensor->getTemp();
    //instance.getLocation(point);
    //  SF_OSAL_printf("gps overnight sample: %lu", millis());
    //  instance.getLocation(point);
    //  SF_OSAL_printf("gps final 1 sample: %lu", millis());
    //std::ofstream outputFile("/home/m.annamalai/timing.txt");
//     SF_OSAL_printf("starting overnight gps sample timing runs of 1000..." __NL__);
//    //if (outputFile.is_open()) {
//     unsigned long it = 0;
//     unsigned long ft = 0;
//     unsigned long diff = 0;
//     for (int i = 0; i < 1000; i++) {
//         it = millis();
//         for (int i = 0; i < 1000; i++) {
//             instance.getLocation(point);
//             //point.latitude = ubloxGps_->getLatitude();
//         }
//         ft = millis();
//         diff = ft-it;
//         SF_OSAL_printf("%lu" __NL__, diff);
//         // outputFile << diff;
//         // outputFile << "\n";
//     }
//     outputFile.close();
//     //} else {
    //    SF_OSAL_printf("Error");
    //}
    // SF_OSAL_printf(".........................");
    // SF_OSAL_printf("starting gps 100 timing runs of 10000..." __NL__);
    // unsigned long it1 = 0;
    // unsigned long ft1 = 0;
    // unsigned long diff1 = 0;
    // for (int i = 0; i < 100; i++) {
    //     it1 = millis();
    //     for (int i = 0; i < 1000; i++) {
    //         instance.getLocation(point);
    //     }
    //     ft1 = millis();
    //     diff1 = ft1-it1;
    //     SF_OSAL_printf("%lu" __NL__, diff1);
    // }
    // SF_OSAL_printf(".........................");
    // SF_OSAL_printf("starting gps 1000 timing runs of 1000..." __NL__);
    // unsigned long it2 = 0;
    // unsigned long ft2 = 0;
    // unsigned long diff2 = 0;
    // for (int i = 0; i < 1000; i++) {
    //     it2 = millis();
    //     for (int i = 0; i < 1000; i++) {
    //         instance.getLocation(point);
    //     }
    //     ft2 = millis();
    //     diff2 = ft2-it2;
    //     SF_OSAL_printf("%lu" __NL__, diff2);
    // }
    
    

    

}
static void CLI_monitorSensors(void) {
    char ch;

    float accelData[3] = {0,0,0};
    float accelDMPData[4] = {0,0,0,0};
    float gyroData[3] = {0,0,0};
    float gyroDMPData[3] = {0,0,0};
    float magData[3] = {0,0,0};
    double quatData[5] = {0,0,0,0,0};
    uint8_t dmpRawData[14] = {0};
    float tmpData = 0;
    float wdCR = 0;
    float wdLS = 0;

    setupICM();
    SF_OSAL_printf(__NL__);
    
    bool a = false;
    bool g = false;
    bool m = false;
    bool t = false;
    bool w = false;
    bool p = false;
    
    SF_OSAL_printf("Enter delay time: ");
    char dt[SF_CLI_MAX_CMD_LEN];
    getline(dt, SF_CLI_MAX_CMD_LEN);
    int delayTime = atoi(dt);
    while (true) {
        SF_OSAL_printf("Enter which sensors you want to look at (a, g, m, t, w, p, r), d to quit: ");
        ch = getch();
        SF_OSAL_printf("%c", ch); 
        SF_OSAL_printf(__NL__);
        if (ch == 'd') {
            break;
        } else if (ch == 'a') { //accelerometer
            a = true;
        } else if (ch == 'g') { //gyroscope
            g = true;
        } else if (ch == 'm') { //Magenetometer
            m = true;
        } else if (ch == 't') { //temp
            t = true;
        }else if (ch == 'w') { //wet dry sensor
            w = true;
        } else if (ch == 'p') { //DMP values
            p = true;
        } else if (ch == 'r') { //raw values
            while(1) {
                if(kbhit()) 
                    {
                        ch = getch();

                        if('q' == ch) 
                        {
                            break;
                        } 
                    }
            // if (getDMPRaw(dmpRawData)) {
            //     SF_OSAL_printf("DMP Raw Data: ");
            //     for (size_t i = 0; i < sizeof(dmpRawData); ++i) {
            //         SF_OSAL_printf("%02X ", dmpRawData[i]);
            //     }
            //     SF_OSAL_printf(__NL__);
            // } 
        }
        } else {
             SF_OSAL_printf("invalid input" __NL__);
        }
    }
     SF_OSAL_printf(__NL__);
    std::vector<std::string> headers;
    if (a) {
        headers.push_back("ax");
        headers.push_back("ay");
        headers.push_back("az");
    }
    if (g) {
        headers.push_back("gx");
        headers.push_back("gy");
        headers.push_back("gz");
    }
    if (m) {
        headers.push_back("mx");
        headers.push_back("my");
        headers.push_back("mz");
    }
    if (t) {
        headers.push_back("temp");
    }
    if (w) {
        headers.push_back("wd lr");
        headers.push_back("wd ls");
    }
    if (p) {
        headers.push_back("dq1");
        headers.push_back("dq2");
        headers.push_back("dq3");
        headers.push_back("dq0");
        headers.push_back("dqacc");
        headers.push_back("dax");
        headers.push_back("day");
        headers.push_back("daz");
        headers.push_back("a acc");
        // headers.push_back("dcx");
        // headers.push_back("dcy");
        // headers.push_back("dcz");
    }

    
    int count = 0;
   
    for(int i = 0; i < 1; i++){
    // {
        if(kbhit()) 
        {
            ch = getch();

            // if('q' == ch) 
            // {
            //     break;
            // } 
        }
        if (a) {
        getAccelerometer(accelData, accelData + 1, accelData + 2);
        }
        if (g) {
        getGyroscope(gyroData, gyroData + 1, gyroData + 2);
        }
        if (m) {
        getMagnetometer(magData, magData + 1, magData + 2);
        }
        //getDMPAccelerometer(accelDMPData, accelDMPData + 1, accelDMPData + 2, accelDMPData + 3);
        //getDMPGyroscope(gyroDMPData, gyroDMPData + 1, gyroDMPData + 2);
        //getDMPQuaternion(quatData, quatData + 1, quatData + 2, quatData + 3, quatData + 4);
        //getDMPCompass();
        //this is from temp sensor not imu temp
        if (t) {
        tmpData = pSystemDesc->pTempSensor->getTemp();
        }
        if (w) {
        wdCR =  pSystemDesc->pWaterSensor->getCurrentReading();
        wdLS = pSystemDesc->pWaterSensor->getLastStatus();
        }
    

        std::map<std::string, float> sensorData = {
        {"ax", N_TO_B_ENDIAN_2(B_TO_N_ENDIAN_2(accelData[0]))},
        {"ay", N_TO_B_ENDIAN_2(B_TO_N_ENDIAN_2(accelData[1]))},
        {"az", N_TO_B_ENDIAN_2(B_TO_N_ENDIAN_2(accelData[2]))},
        {"gx", gyroData[0]},
        {"gy", gyroData[1]},
        {"gz", gyroData[2]},
        {"mx", magData[0]},
        {"my", magData[1]},
        {"mz", magData[2]},
        {"temp", tmpData},
        {"wd cr", wdCR},
        {"wd ls", wdLS},
        {"dax", accelDMPData[0]},
        {"day", accelDMPData[1]},
        {"daz", accelDMPData[2]},
        {"a acc", accelDMPData[3]},
        {"dgx", gyroDMPData[0]},
        {"dgy", gyroDMPData[1]},
        {"dgz", gyroDMPData[2]},
        {"dq1", quatData[0]},
        {"dq2",  quatData[1]},
        {"dq3",  quatData[2]},
        {"dq0",  quatData[3]},
        {"dqacc",  quatData[4]}


    };
        if (count % 10 == 0) {
            for (const auto& header : headers) {
            SF_OSAL_printf("|   %s    |\t", header.c_str());
            }
            SF_OSAL_printf(__NL__);
        }
        for (const auto& header : headers) {
        SF_OSAL_printf(" %8.4f\t", sensorData.at(header));
        }
        SF_OSAL_printf(__NL__);
        count++;
        delay(delayTime);
    }
}
// }
void CLI_displayMenu(void)
{
    MNU_displayMenu(CLI_menu);
}

void CLI_hexdump(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    const void* pBuffer;
    size_t buffer_length;
    SF_OSAL_printf("Starting address: 0x");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    pBuffer = (const void*)strtol(input_buffer, &pEndTok, 16);
    SF_OSAL_printf("Length: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    buffer_length = (size_t) strtol(input_buffer, &pEndTok, 10);
    hexDump(pBuffer, buffer_length);
}

static void CLI_setState(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    STATES_e nextState;
    
    for (int i = 1; i < STATE_N_STATES; i++)
    {
        SF_OSAL_printf("%3d: %s" __NL__, i, STATES_NAME_TAB[i]);
    }
    SF_OSAL_printf("Enter state to change to: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    nextState = (STATES_e) strtol(input_buffer, &pEndTok, 10);
    if (nextState == 0)
    {
        SF_OSAL_printf("Invalid state" __NL__);
        return;
    }
    CLI_nextState = nextState;
    SF_OSAL_printf("Switching to %s" __NL__, STATES_NAME_TAB[nextState]);
    return;
}

static void CLI_displaySystemState(void)
{
    SYS_displaySys();
}

static void CLI_displayNVRAM(void)
{
    NVRAM& instance = NVRAM::getInstance();
    instance.displayNVRAM();
}

static void CLI_sleepSetSleepBehavior(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    SleepTask::BOOT_BEHAVIOR_e boot_behavior;
    SF_OSAL_printf("Boot Behavior Code: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    boot_behavior = (SleepTask::BOOT_BEHAVIOR_e) strtol(input_buffer, &pEndTok, 10);
    SleepTask::setBootBehavior(boot_behavior);
}

static void CLI_sleepGetSleepBehavior(void)
{
    SleepTask::BOOT_BEHAVIOR_e boot_behavior =  SleepTask::getBootBehavior();
    SF_OSAL_printf("Boot Behavior: %s" __NL__, SleepTask::strBootBehavior(boot_behavior));
}

void CLI_displayResetReason(void)
{
    uint16_t reset_reason = System.resetReason();
    SF_OSAL_printf("Reset Reason: %hd", reset_reason);
    switch(reset_reason)
    {
        case RESET_REASON_PIN_RESET:
            SF_OSAL_printf("nRESET Assertion");
            break;
        case RESET_REASON_POWER_MANAGEMENT:
            SF_OSAL_printf("Low Power Management Reset");
            break;
        case RESET_REASON_POWER_DOWN:
            SF_OSAL_printf("Power-down Reset");
            break;
        case RESET_REASON_POWER_BROWNOUT:
            SF_OSAL_printf("Brownout Reset");
            break;
        case RESET_REASON_WATCHDOG:
            SF_OSAL_printf("Watchdog Reset");
            break;
        case RESET_REASON_UPDATE:
            SF_OSAL_printf("FW Update Success");
            break;
        case RESET_REASON_UPDATE_TIMEOUT:
            SF_OSAL_printf("FW Update Timeout");
            break;
        case RESET_REASON_FACTORY_RESET:
            SF_OSAL_printf("Factory Reset");
            break;
        case RESET_REASON_SAFE_MODE:
            SF_OSAL_printf("Safe Mode");
            break;
        case RESET_REASON_DFU_MODE:
            SF_OSAL_printf("DFU mode");
            break;
        case RESET_REASON_PANIC:
            SF_OSAL_printf("System Panic");
            break;
        case RESET_REASON_USER:
            SF_OSAL_printf("User Reset");
            break;
        case RESET_REASON_NONE:
            SF_OSAL_printf("No info available");
            break;
        case RESET_REASON_UNKNOWN:
        default:
            SF_OSAL_printf("Unknown Reset");
            break;
    }
    SF_OSAL_printf(__NL__);
}