#include "system.hpp"

#include "location_service.h"
#include "Particle.h"
#include "product.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "SPI.h"
#include <fcntl.h>

#include "consts.hpp"

char SYS_deviceID[32];

SystemDesc_t systemDesc, *pSystemDesc = &systemDesc;
SystemFlags_t systemFlags;


Recorder dataRecorder;


static void SYS_chargerTask(void);
static int SYS_initGPS(void);
static int SYS_initNVRAM(void);
static int SYS_initTasks(void);
static int SYS_initFS();

static Timer chargerTimer(SYS_CHARGER_REFRESH_MS, SYS_chargerTask, false);


static LocationServiceConfiguration create_location_service_config();

void SYS_initSys(void)
{
    memset(pSystemDesc, 0, sizeof(SystemDesc_t));
    systemDesc.deviceID = SYS_deviceID;
    systemDesc.flags = &systemFlags;

    memset(SYS_deviceID, 0, 32);
    strncpy(SYS_deviceID, System.deviceID().c_str(), 31);

    SYS_initTasks();
    SYS_initGPS();
    SYS_initNVRAM();
    SYS_initFS();
}

static int SYS_initTasks(void)
{
    pinMode(STAT_PIN, INPUT);
    pinMode(SF_USB_PWR_DETECT_PIN, INPUT);
    systemFlags.hasCharger = true;
    systemFlags.batteryLow = false;

    systemDesc.pChargerCheck = &chargerTimer;
    return 1;
}


void SYS_chargerTask(void)
{
    bool isCharging = ~digitalRead(STAT_PIN);
    systemFlags.hasCharger = digitalRead(SF_USB_PWR_DETECT_PIN);
    static int chargedTimestamp;
    static int chargingTimestamp;

    if(systemFlags.hasCharger)
    {
        // Check charging with hysteresis
        if(isCharging)
        {
            chargingTimestamp++;
            if(chargingTimestamp >= SYS_CHARGER_MIN_CHARGING_MS / SYS_CHARGER_REFRESH_MS)
            {
                chargingTimestamp = SYS_CHARGER_MIN_CHARGING_MS / SYS_CHARGER_REFRESH_MS;
            }
            chargedTimestamp = 0;
        }
        else
        {
            chargingTimestamp = 0;
            chargedTimestamp++;
            if(chargedTimestamp >= SYS_CHARGER_MIN_CHARGED_MS / SYS_CHARGER_REFRESH_MS)
            {
                chargedTimestamp = SYS_CHARGER_MIN_CHARGED_MS / SYS_CHARGER_REFRESH_MS;
            }
        }
        if(chargingTimestamp >= SYS_CHARGER_MIN_CHARGING_MS / SYS_CHARGER_REFRESH_MS)
        {
            systemDesc.pBatteryLED->setState(SFLed::SFLED_STATE_BLINK);
        }
        if(chargedTimestamp >= SYS_CHARGER_MIN_CHARGED_MS / SYS_CHARGER_REFRESH_MS)
        {
            systemDesc.pBatteryLED->setState(SFLed::SFLED_STATE_ON);
        }
    }
    else
    {
        chargedTimestamp = 0;
        chargedTimestamp = 0;
        systemDesc.pBatteryLED->setState(SFLed::SFLED_STATE_OFF);
        systemDesc.pChargerCheck->stopFromISR();
    }
}


/**
 * @brief Initialize file system
 * 
 * @return int 
 */
static int SYS_initFS(void)
{
    // DP_spiFlash.begin();
    // DP_fs.withPhysicalAddr(SF_FLASH_SIZE_MB * 1024 * 1024);
    // SF_OSAL_printf("Device ID : %s", DP_spiFlash.jedecIdRead());
    // FLOG_AddError(FLOG_SYS_MOUNT_FAIL, DP_fs.mountAndFormatIfNecessary());

    // systemDesc.pFileSystem = &DP_fs;

    dataRecorder.init();
    systemDesc.pRecorder = &dataRecorder;

    // EXAMPLE
    

    return 1;
}

/**
 * @brief Initialization function for GPS 
 * Ublox gps, handled by @file gps/location_service.cpp
 * @return int 1 on success, otherwise 0
 */
static int SYS_initGPS(void) 
{
    LocationServiceConfiguration config = create_location_service_config();
    LocationService::instance().setModuleType();
    if(LocationService::instance().begin(config) != 0)
    {
        SF_OSAL_printf("GPS Initialization Failed" __NL__);
        SF_OSAL_printf("Check pin map and reboot" __NL__);

        FLOG_AddError(FLOG_GPS_INIT_FAIL, 0);
        return 0;
    }

    if(LocationService::instance().start() != 0)
    {
        SF_OSAL_printf("GPS Start Failed" __NL__);
        SF_OSAL_printf("Please check GPS and reboot" __NL__);

        FLOG_AddError(FLOG_GPS_START_FAIL, 0);
        return 0;
    }
    LocationService::instance().setFastLock(true);

    systemDesc.pLocService = &LocationService::instance();
    
    return 1;
}

static int SYS_initNVRAM(void)
{
    NVRAM& nvram = NVRAM::getInstance();

    systemDesc.pNvram = &nvram;

    return 1;
}

/**
 * @brief Create a location service config object with defaults
 * 
 * @return LocationServiceConfiguration 
 */
static LocationServiceConfiguration create_location_service_config() {
    LocationServiceConfiguration config;
    config.enableFastLock(LOCATION_CONFIG_ENABLE_FAST_LOCK);
    config.enableUDR(LOCATION_CONFIG_ENABLE_UDR);
    config.udrModel(LOCATION_CONFIG_UDR_DYNAMIC_MODEL);
    config.imuYaw(LOCATION_CONFIG_IMU_ORIENTATION_YAW);
    config.imuPitch(LOCATION_CONFIG_IMU_ORIENTATION_PITCH);
    config.imuRoll(LOCATION_CONFIG_IMU_ORIENTATION_ROLL);
    config.enableIMUAutoAlignment(LOCATION_CONFIG_ENABLE_AUTO_IMU_ALIGNMENT);
    config.imuToVRPX(0);
    config.imuToVRPY(0);
    config.imuToVRPZ(0);
    config.enableHotStartOnWake(LOCATION_CONFIG_ENABLE_HOT_START_ON_WAKE);
    config.enableAssistNowAutonomous(LOCATION_CONFIG_ENABLE_ASSISTNOW_AUTONOMOUS);

    return config;
}
