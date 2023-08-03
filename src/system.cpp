#include "system.hpp"

#include "gps/location_service.h"
#include "product.hpp"
#include "temperature/tmpSensor.h"
#include "temperature/max31725.h"

#include "Particle.h"



char SYS_deviceID[32];

SystemDesc_t systemDesc, *pSystemDesc = &systemDesc;
SystemFlags_t systemFlags;


static int SYS_initNVRAM(void);
static int SYS_initTasks(void);
static void SYS_chargerTask(void);
static int SYS_initGPS();
static int SYS_initTempSensor(void);


static Timer chargerTimer(SYS_CHARGER_REFRESH_MS, SYS_chargerTask, false);

I2C i2cBus;
MAX31725 max31725(i2cBus, MAX31725_I2C_SLAVE_ADR_00);
tmpSensor tempSensor(max31725);


int SYS_initSys(void)
{
    memset(pSystemDesc, 0, sizeof(SystemDesc_t));
    systemDesc.deviceID = SYS_deviceID;
    systemDesc.flags = &systemFlags;

    memset(SYS_deviceID, 0, 32);
    strncpy(SYS_deviceID, System.deviceID(), 31);

    

    SYS_initTasks();
    SYS_initGPS();
    SYS_initTempSensor();
    SYS_initNVRAM();

    return 1;
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


static int SYS_initTempSensor(void)
{
    Wire.begin();
    systemDesc.pTempSensor = &tempSensor;

    return 1;
}

static void SYS_chargerTask(void)
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


static int SYS_initGPS() 
{
    LocationServiceConfiguration config = create_location_service_config();
	LocationService::instance().setModuleType();
    LocationService::instance().begin(config);
    LocationService::instance().start();
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