#include "system.hpp"

#include "product.hpp"

#include "temperature/tmpSensor.h"
#include "temperature/max31725.h"
#include "location_service.h"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "SPI.h"
#include <fcntl.h>

#include "consts.hpp"
#include "states.hpp"
#include "product.hpp"

#include "sys/led.hpp"


#include "Particle.h"



char SYS_deviceID[32];

SystemDesc_t systemDesc = {0}, *pSystemDesc = &systemDesc;
SystemFlags_t systemFlags;

static LEDSystemTheme ledTheme;

Recorder dataRecorder;



static void SYS_chargerTask(void);
static int SYS_initGPS(void);
static int SYS_initNVRAM(void);
static int SYS_initTasks(void);
static void SYS_waterTask(void);
static int SYS_initWaterSensor(void);
static int SYS_initLEDs(void);
static int SYS_initFS();
static int SYS_initTempSensor(void);
static int SYS_initIMU(void);

I2C i2cBus;
MAX31725 max31725(i2cBus, MAX31725_I2C_SLAVE_ADR_00);
tmpSensor tempSensor(max31725);

static SFLed batteryLED(STAT_LED_PIN, SFLed::SFLED_STATE_OFF);
static SFLed waterLED(WATER_STATUS_LED, SFLed::SFLED_STATE_OFF);

static Timer chargerTimer(SYS_CHARGER_REFRESH_MS, SYS_chargerTask, false);
static Timer waterTimer(SYS_WATER_REFRESH_MS, SYS_waterTask, false);
static Timer ledTimer(SF_LED_BLINK_MS, SFLed::doLEDs, false);

static WaterSensor waterSensor(WATER_DETECT_EN_PIN, WATER_DETECT_PIN);

static LocationServiceConfiguration create_location_service_config();

static FuelGauge battery_desc;

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
static IMU icm_20948(Wire, false);
#endif

void SYS_initSys(void)
{
    memset(pSystemDesc, 0, sizeof(SystemDesc_t));
    systemDesc.deviceID = SYS_deviceID;
    systemDesc.flags = &systemFlags;

    memset(SYS_deviceID, 0, 32);
    strncpy(SYS_deviceID, System.deviceID().c_str(), 31);

    SYS_initTasks();
    SYS_initGPS();
    SYS_initTempSensor();
    SYS_initNVRAM();
    SYS_initFS();
    SYS_initWaterSensor();
    SYS_initLEDs();

    pinMode(WKP, INPUT);

    systemDesc.pBattery = &battery_desc;    
}

/**
 * @brief Delayed system initialization
 *
 * This to run after threads start
 *
 */
void SYS_delayedInitSys(void)
{
    SYS_initIMU();
}

void SYS_deinitSys(void)
{
    systemDesc.pIMU->end();
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

    return 1;
}

 /**
 * @brief Initialize system tasks (charging and sleep)
 * 
 * @return int 
 */
static int SYS_initTasks(void)
{
    pinMode(SF_USB_PWR_DETECT_PIN, INPUT_PULLDOWN);
    systemFlags.hasCharger = true;
    systemFlags.batteryLow = false;

    systemDesc.pChargerCheck = &chargerTimer;
    systemDesc.pWaterCheck = &waterTimer;
    systemDesc.pWaterCheck->start();

    return 1;
}


static int SYS_initTempSensor(void)
{
    Wire.begin();
    systemDesc.pTempSensor = &tempSensor;

    return 1;
}


static int SYS_initWaterSensor(void)
{
    uint8_t water_sensor_window = WATER_DETECT_SURF_SESSION_INIT_WINDOW;
    pinMode(WATER_DETECT_EN_PIN, OUTPUT);
    digitalWrite(WATER_DETECT_EN_PIN, HIGH);
    pinMode(WATER_DETECT_PIN, INPUT);
    pinMode(WATER_MFG_TEST_EN, OUTPUT);
    digitalWrite(WATER_MFG_TEST_EN, LOW);
    systemDesc.pWaterSensor = &waterSensor;
    waterSensor.begin();
    ledTimer.start();

    // TODO: remove this once NVRAM defaults exist
    systemDesc.pNvram->put(NVRAM::DATA_ID_e::WATER_DETECT_WINDOW_LEN,
                           WATER_DETECT_SURF_SESSION_INIT_WINDOW);
    if (!systemDesc.pNvram->get(NVRAM::DATA_ID_e::WATER_DETECT_WINDOW_LEN, water_sensor_window))
    {
        water_sensor_window = WATER_DETECT_SURF_SESSION_INIT_WINDOW;
        // We don't really care about the return value
        systemDesc.pNvram->put(NVRAM::DATA_ID_e::WATER_DETECT_WINDOW_LEN,
                               WATER_DETECT_SURF_SESSION_INIT_WINDOW);
    }

    waterSensor.setWindowSize(water_sensor_window);
    // Take an initial reading to ensure that subsequent initialization operations
    // work properly
    waterSensor.update();

    return 1;
}

static int SYS_initLEDs(void)
{
    batteryLED.init();
    systemDesc.pBatteryLED = &batteryLED;
    waterLED.init();
    systemDesc.pWaterLED = &waterLED;

    ledTheme.setSignal(LED_SIGNAL_NETWORK_OFF, 0x000000, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_NETWORK_ON, SF_DUP_RGB_LED_COLOR, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_NETWORK_CONNECTING, SF_DUP_RGB_LED_COLOR, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_NETWORK_DHCP, SF_DUP_RGB_LED_COLOR, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_NETWORK_CONNECTED, SF_DUP_RGB_LED_COLOR, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_CLOUD_CONNECTING, SF_DUP_RGB_LED_COLOR, LED_PATTERN_SOLID);
    ledTheme.setSignal(LED_SIGNAL_CLOUD_CONNECTED, SF_DUP_RGB_LED_COLOR, LED_PATTERN_BLINK, SF_DUP_RGB_LED_PERIOD);
    ledTheme.setSignal(LED_SIGNAL_CLOUD_HANDSHAKE, SF_DUP_RGB_LED_COLOR, LED_PATTERN_BLINK, SF_DUP_RGB_LED_PERIOD);

    systemDesc.systemTheme = &ledTheme;
    return 1;
}


/**
 * @brief Charging task
 * 
 */
void SYS_chargerTask(void)
{
    bool previous_state = systemFlags.hasCharger;
    bool isCharging = System.batteryState() == BATTERY_STATE_CHARGING;
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
        if (previous_state)
        {
            FLOG_AddError(FLOG_CHARGER_REMOVED, 0);
            chargedTimestamp = 0;
            systemDesc.pBatteryLED->setState(SFLed::SFLED_STATE_OFF);
        }
    }
}

static void SYS_waterTask(void)
{
    systemDesc.water_check_count++;
    systemDesc.pWaterSensor->update();
    if(systemDesc.pWaterSensor->getLastReading())
    {
        systemDesc.pWaterLED->setState(SFLed::SFLED_STATE_ON);
    }
    else
    {
        systemDesc.pWaterLED->setState(SFLed::SFLED_STATE_OFF);
    }
}

/**
 * @brief Initialization function for GPS
 * Ublox gps
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


/**
 * @brief Initializes NVRAM 
 * 
 * @return int sucsess
 */
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

void SYS_displaySys(void)
{
    SF_OSAL_printf("Device ID: %s" __NL__, pSystemDesc->deviceID);
    SF_OSAL_printf("Location Service: 0x%08x" __NL__, pSystemDesc->pLocService);
    SF_OSAL_printf("Charger Check Timer: 0x%08x" __NL__, pSystemDesc->pChargerCheck);
    SF_OSAL_printf("NVRAM: 0x%08x" __NL__, pSystemDesc->pNvram);
    SF_OSAL_printf("Battery LED: 0x%08x" __NL__, pSystemDesc->pBatteryLED);
    SF_OSAL_printf("Battery Low Flag: %d" __NL__, pSystemDesc->flags->batteryLow);
    SF_OSAL_printf("Has Charger Flag: %d" __NL__, pSystemDesc->flags->hasCharger);

    SF_OSAL_printf(__NL__);
    SF_OSAL_printf("Particle Connected: %d" __NL__, Particle.connected());
    SF_OSAL_printf("Cellular On: %d" __NL__, Cellular.isOn());
    SF_OSAL_printf("Cellular Ready: %d" __NL__, Cellular.ready());
}

void SYS_dumpSys(int indent)
{
    char indent_str[33];
    if (indent > 32)
    {
        indent = 32;
    }
    if (indent < 0)
    {
        indent = 0;
    }
    memset(indent_str, ' ', 32);
    indent_str[indent] = 0;

    {
        SF_OSAL_printf("%sDevice ID: %s" __NL__, indent_str, pSystemDesc->deviceID);
    }
    {
        SF_OSAL_printf("%sLocation Service: 0x%08x" __NL__, indent_str, pSystemDesc->pLocService);
        SF_OSAL_printf("%s  Location Service Status: %d" __NL__,
                       indent_str,
                       pSystemDesc->pLocService->isActive());
    }
    {
        SF_OSAL_printf("%sRecorder: 0x%08x" __NL__, indent_str, pSystemDesc->pRecorder);
    }
    {
        SF_OSAL_printf("%sCharger Check: 0x%08x" __NL__, indent_str, pSystemDesc->pChargerCheck);
        SF_OSAL_printf("%s  Charger Check Active: %d" __NL__,
                       indent_str,
                       pSystemDesc->pChargerCheck->isActive());
    }
    {
        SF_OSAL_printf("%sWater Check: 0x%08x" __NL__, indent_str, pSystemDesc->pWaterCheck);
        SF_OSAL_printf(
            "%s  Water Check Active: %d" __NL__, indent_str, pSystemDesc->pWaterCheck->isActive());
        SF_OSAL_printf(
            "%s  Water Check Exec Count: %d" __NL__, indent_str, systemDesc.water_check_count);
    }
    {
        SF_OSAL_printf("%sNVRAM: 0x%08x" __NL__, indent_str, pSystemDesc->pNvram);
    }
    {
        SF_OSAL_printf("%sWater Sensor: 0x%08x" __NL__, indent_str, pSystemDesc->pWaterSensor);
        SF_OSAL_printf(
            "%s  Window Size: %hu" __NL__, indent_str, pSystemDesc->pWaterSensor->getWindowSize());
    }
    {
        SF_OSAL_printf("%sBattery LED: 0x%08x" __NL__, indent_str, pSystemDesc->pBatteryLED);
    }
    {
        SF_OSAL_printf("%sWater LED: 0x%08x" __NL__, indent_str, pSystemDesc->pWaterLED);
        SF_OSAL_printf(
            "%s  Water LED State: %d" __NL__, indent_str, pSystemDesc->pWaterLED->getState());
    }
    {
        SF_OSAL_printf("%sTemp Sensor: 0x%08x" __NL__, indent_str, pSystemDesc->pTempSensor);
    }
    {
        SF_OSAL_printf("%sSystem Theme: 0x%08x" __NL__, indent_str, pSystemDesc->systemTheme);
    }
    {
        SF_OSAL_printf("%sBattery: 0x%08x" __NL__, indent_str, pSystemDesc->pBattery);
    }
    {
        SF_OSAL_printf("%sSystem Flags: 0x%08x" __NL__, indent_str, pSystemDesc->flags);
    }
}

/**
 * @brief Initializes the IMU
 *
 * @return int 1 on success, otherwise 0
 */
int SYS_initIMU(void)
{
    bool fail = false;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    pSystemDesc->pIMU = &icm_20948;

    fail = icm_20948.begin();
    if (fail)
    {
        SF_OSAL_printf("IMU init failed!" __NL__);
        return 0;
    }
#endif
    return 1;
}
