/**
 * @file Particle.h
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief GLIBC Particle Shim
 * @version 0.1
 * @date 2024-11-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __PC_HAL_PARTICLE_H__
#define __PC_HAL_PARTICLE_H__

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#define retained
class EEPROMClass
{
private:
    uint8_t *blob;
    std::size_t blob_size;

public:
    EEPROMClass();

    template <typename T> T &get(int idx, T &t)
    {
        std::memcpy(&t, this->blob + idx, sizeof(T));
        return t;
    }

    template <typename T> const T &put(int idx, const T &t)
    {
        return t;
    }
};

#define EEPROM __fetch_global_EEPROM()
EEPROMClass &__fetch_global_EEPROM();

class Timer
{
    int period;
    void (*cb)(void);
    bool one_shot;

public:
    Timer(int period, void (*fn)(void), bool one_shot = false);
    void start(void)
    {
    }
    void stop(void)
    {
    }
};

class USARTSerial
{
public:
    void begin(int baud)
    {
    }

    std::size_t available()
    {
        return 0;
    }

    int read()
    {
        return 0;
    }

    void print(char ch)
    {
    }
    void write(const char *str)
    {
    }
};
#define Serial __get_default_serial()
USARTSerial &__get_default_serial();

class SPIClass
{
};

class String
{
    std::string val;

public:
    String(const char *val);
    const char *c_str() const
    {
        return val.c_str();
    }
};

typedef enum
{

} LogLevel;

class Logger
{
};

typedef struct
{

} __SPISettings;

class os_queue_t
{
};

class Thread
{
};
#define os_thread_yield() ;

enum
{
    PIN_INVALID
};

class TwoWire
{
public:
    void begin(void)
    {
    }
};
#define Wire __fetch_global_I2C()
TwoWire &__fetch_global_I2C();
typedef std::size_t system_tick_t;

template <class T> class Vector
{
};

class RecursiveMutex
{
};

typedef enum
{
    LED_SIGNAL_NETWORK_OFF,
    LED_SIGNAL_NETWORK_ON,
    LED_SIGNAL_NETWORK_CONNECTING,
    LED_SIGNAL_NETWORK_DHCP,
    LED_SIGNAL_NETWORK_CONNECTED,
    LED_SIGNAL_CLOUD_CONNECTING,
    LED_SIGNAL_CLOUD_CONNECTED,
    LED_SIGNAL_CLOUD_HANDSHAKE
} LEDSignal;
typedef enum
{
    LED_PATTERN_SOLID,
    LED_PATTERN_BLINK
} LEDPattern;
typedef enum
{
    LED_SPEED_NORMAL
} LEDSpeed;

enum
{
    RGB_COLOR_BLUE = 0x000000ff,
    RGB_COLOR_RED = 0x00ff0000,
    RGB_COLOR_YELLOW = 0x0000ffff,
};

class LEDSystemTheme
{
public:
    LEDSystemTheme(void)
    {
    }
    void setSignal(LEDSignal signal, uint32_t color)
    {
    }
    void setSignal(LEDSignal signal,
                   uint32_t color,
                   LEDPattern pattern,
                   LEDSpeed speed = LED_SPEED_NORMAL)
    {
    }
    void setSignal(LEDSignal signal, uint32_t color, LEDPattern pattern, uint16_t period)
    {
    }
};

class FuelGauge
{
public:
    float getVCell(void)
    {
        return 0;
    }
};

typedef enum
{
    LED_PRIORITY_IMPORTANT
} LEDPriority;
class LEDStatus
{
public:
    void setColor(uint32_t color)
    {
    }
    void setPattern(LEDPattern pattern)
    {
    }
    void setPeriod(std::uint16_t period)
    {
    }
    void setPriority(LEDPriority priority)
    {
    }
    void setActive(bool active = true)
    {
    }
};

#define SYSTEM_MODE(a) ;
#define SYSTEM_THREAD(a) ;

typedef enum
{
    FEATURE_RESET_INFO
} HAL_Feature;
typedef enum
{
    RESET_REASON_NONE,
    RESET_REASON_PIN_RESET,
    RESET_REASON_POWER_MANAGEMENT,
    RESET_REASON_POWER_DOWN,
    RESET_REASON_POWER_BROWNOUT,
    RESET_REASON_WATCHDOG,
    RESET_REASON_UPDATE,
    RESET_REASON_UPDATE_TIMEOUT,
    RESET_REASON_FACTORY_RESET,
    RESET_REASON_SAFE_MODE,
    RESET_REASON_DFU_MODE,
    RESET_REASON_PANIC,
    RESET_REASON_USER,
    RESET_REASON_UNKNOWN
} __RESET_REASON_t;

typedef enum
{
    BATTERY_STATE_UNKNOWN = 0,
    BATTERY_STATE_NOT_CHARGING = 1,
    BATTERY_STATE_CHARGING = 2,
    BATTERY_STATE_CHARGED = 3,
    BATTERY_STATE_DISCHARGING = 4,
    BATTERY_STATE_FAULT = 5,
    BATTERY_STATE_DISCONNECTED = 6
} battery_state_t;

enum
{
    SLEEP_MODE_SOFTPOWEROFF
};
class SystemClass
{
    String device_id = String("");
    String version_str = String("");

public:
    int enableFeature(HAL_Feature feature)
    {
        return 0;
    }
    __RESET_REASON_t resetReason()
    {
        return RESET_REASON_NONE;
    }

    String &deviceID(void)
    {
        return device_id;
    }

    int batteryState() const
    {
        return BATTERY_STATE_UNKNOWN;
    }

    void sleep(uint32_t mode);
    void sleep(uint32_t mode, uint32_t time);
    void reset(void)
    {
        exit(0);
    }

    String &version(void)
    {
        return version_str;
    }
};
#define System __fetch_global_System()
SystemClass &__fetch_global_System();

typedef std::uint32_t time32_t;

class TimeClass
{
public:
    time32_t now()
    {
        return 0;
    }

    void delay(std::uint32_t ms)
    {
    }
    void delayMicroseconds(std::size_t us)
    {
    }
    String format(uint32_t timestamp, const char *fmt)
    {
        return String("");
    }
};
#define Time __fetch_global_time()
TimeClass &__fetch_global_time();

#define millis Time.now
#define delay Time.delay
#define delayMicroseconds Time.delayMicroseconds

class ParticleClass
{
public:
    void process()
    {
    }

    bool connected()
    {
        return false;
    }
    bool disconnected()
    {
        return !this->connected();
    }
    void connect(void)
    {
    }
    void disconnect(void)
    {
    }
    bool publish(const char *name, const char *data)
    {
        return false;
    }
};
#define Particle __fetch_global_particle()
ParticleClass &__fetch_global_particle();

class CellularClass
{
public:
    bool isOn()
    {
        return false;
    }
    bool ready()
    {
        return false;
    }
};
#define Cellular __fetch_global_cellular()
CellularClass &__fetch_global_cellular();

/**
 * @brief PC HAL Pin Definitions
 *
 */
typedef enum
{
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    WKP
} __PC_HAL_PIN_DEFs;

typedef enum
{
    INPUT,
    INPUT_PULLDOWN,
    OUTPUT
} __PC_HAL_PIN_CONFIG;

typedef enum
{
    LOW,
    HIGH
} __PC_HAL_PIN_STATE;

void pinMode(int pin, __PC_HAL_PIN_CONFIG mode);

void digitalWrite(int pin, int state);
void digitalWriteFast(int pin, int state);

int digitalRead(int pin);
int pinReadFast(int pin);
namespace particle
{
    namespace protocol
    {
        const std::size_t MAX_EVENT_DATA_LENGTH = 1024;
        const std::size_t MAX_EVENT_NAME_LENGTH = 64;
    } // namespace protocol
} // namespace particle
#endif // __PC_HAL_PARTICLE_H__