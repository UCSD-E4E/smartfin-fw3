/**
 * @file hal_pc.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief PC/GLibC stub implementations for the SF_HAL platform interface.
 * @date 2026-05-26
 */

#include "Particle.h"
#include "platform/hal.hpp"
#include "platform/hal_types.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <string>

namespace SF_HAL
{

// Timing

tick_t millis()
{
    return static_cast<tick_t>(__fetch_global_time().now());
}

uint32_t micros()
{
    using namespace std::chrono;
    return static_cast<uint32_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}

void delay_ms(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delay_us(uint32_t us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

uint32_t time_now()
{
    return __fetch_global_time().now();
}

bool time_is_valid()
{
    return __fetch_global_time().isValid();
}

// GPIO

void gpio_set_mode(PinId /*pin*/, GpioMode /*mode*/)
{
}

void gpio_write(PinId /*pin*/, GpioState /*state*/)
{
}

bool gpio_read(PinId /*pin*/)
{
    return false;
}

void gpio_write_fast(PinId /*pin*/, GpioState /*state*/)
{
}

bool gpio_read_fast(PinId /*pin*/)
{
    return false;
}

// Serial

void serial_begin(uint32_t /*baud*/)
{
}

int serial_available()
{
    return 0;
}

int serial_read()
{
    return -1;
}

void serial_print_char(char /*ch*/)
{
}

void serial_write(const uint8_t * /*buf*/, std::size_t /*len*/)
{
}

// I2C

static TwoWire g_wire;

void i2c_begin()
{
    g_wire.begin();
}

bool i2c_is_enabled()
{
    return false;
}

int i2c_read(uint8_t /*address*/, char * /*data*/, int /*length*/, bool /*repeated*/)
{
    return -1;
}

int i2c_write(uint8_t /*address*/,
              const char * /*data*/,
              int /*length*/,
              bool /*repeated*/)
{
    return -1;
}

TwoWire &i2c_get_wire()
{
    return g_wire;
}

void i2c_lock()
{
}

void i2c_unlock()
{
}

// NVM

static uint8_t g_nvm[4096];

void nvm_read(uint32_t addr, void *out, std::size_t len)
{
    if (addr + len <= sizeof(g_nvm))
    {
        std::memcpy(out, g_nvm + addr, len);
    }
}

void nvm_write(uint32_t addr, const void *in, std::size_t len)
{
    if (addr + len <= sizeof(g_nvm))
    {
        std::memcpy(g_nvm + addr, in, len);
    }
}

// Cloud

void cloud_start_connect()
{
}

void cloud_start_disconnect()
{
}

bool cloud_connected()
{
    return false;
}

bool cloud_publish(const char * /*event*/, const char * /*data*/)
{
    return false;
}

void cloud_sync_time()
{
}

bool cloud_sync_time_done()
{
    return true;
}

bool cellular_is_on()
{
    return false;
}

bool cellular_is_ready()
{
    return false;
}

void cloud_process()
{
}

// BLE

void ble_set_callbacks(const ble::Callbacks & /*callbacks*/)
{
}

bool ble_init(const char * /*device_name*/,
              const char * /*service_uuid*/,
              const char * /*telemetry_char_uuid*/,
              const char * /*control_char_uuid*/)
{
    return false;
}

bool ble_advertise(const char * /*service_uuid*/, const char * /*local_name*/)
{
    return false;
}

void ble_stop_advertising()
{
}

bool ble_notify(const uint8_t * /*data*/, std::size_t /*len*/)
{
    return false;
}

// System identity and control

const char *system_device_id()
{
    return "pc-stub-device-id";
}

const char *system_os_version()
{
    return "pc-stub";
}

void system_reset()
{
    std::exit(0);
}

void system_sleep(SleepMode /*mode*/, uint32_t /*duration_sec*/)
{
}

void system_sleep_gpio_wake(PinId /*wake_pin*/)
{
}

// Battery

float battery_voltage()
{
    return 0.0f;
}

BatteryState battery_state()
{
    return BatteryState::UNKNOWN;
}

// Reset diagnostics

void reset_info_enable()
{
}

ResetReason reset_reason()
{
    return ResetReason::NONE;
}

uint32_t reset_reason_data()
{
    return 0;
}

// Threading

void *thread_create(const char *name,
                    ThreadFn function,
                    void *param,
                    ThreadPriority /*priority*/,
                    std::size_t /*stack_size*/)
{
    auto *t = new std::thread(function, param);
    (void)name;
    t->detach();
    return static_cast<void *>(t);
}

void thread_yield()
{
    std::this_thread::yield();
}

// Mutex

Mutex::Mutex()
{
    _handle = new std::mutex();
}

Mutex::~Mutex()
{
    delete static_cast<std::mutex *>(_handle);
}

void Mutex::lock()
{
    static_cast<std::mutex *>(_handle)->lock();
}

void Mutex::unlock()
{
    static_cast<std::mutex *>(_handle)->unlock();
}

// Timer

struct TimerHandle
{
    uint32_t period_ms;
    void (*callback)(void);
    bool one_shot;
    bool active;
};

Timer::Timer(uint32_t period_ms, void (*callback)(void), bool one_shot)
{
    auto *h = new TimerHandle{period_ms, callback, one_shot, false};
    _handle = h;
}

Timer::~Timer()
{
    delete static_cast<TimerHandle *>(_handle);
}

void Timer::start()
{
    static_cast<TimerHandle *>(_handle)->active = true;
}

void Timer::stop()
{
    static_cast<TimerHandle *>(_handle)->active = false;
}

bool Timer::isActive()
{
    return static_cast<TimerHandle *>(_handle)->active;
}

// LED

LedStatus::~LedStatus()
{
}

void LedStatus::setColor(uint32_t /*color*/)
{
}

void LedStatus::setPattern(LedPattern /*pattern*/)
{
}

void LedStatus::setPeriod(uint16_t /*period_ms*/)
{
}

void LedStatus::setPriority(LedPriority /*priority*/)
{
}

void LedStatus::setActive(bool /*active*/)
{
}

LedSystemTheme::LedSystemTheme()
{
}

void LedSystemTheme::setSignal(LedSignal /*signal*/, uint32_t /*color*/)
{
}

void LedSystemTheme::setSignal(LedSignal /*signal*/,
                               uint32_t /*color*/,
                               LedPattern /*pattern*/,
                               LedSpeed /*speed*/)
{
}

void LedSystemTheme::setSignal(LedSignal /*signal*/,
                               uint32_t /*color*/,
                               LedPattern /*pattern*/,
                               uint16_t /*period_ms*/)
{
}

void LedSystemTheme::apply()
{
}

// Misc

int random(int min, int max)
{
    return min + (std::rand() % (max - min));
}

std::vector<std::string> board_version()
{
    return {"pc-stub"};
}

} // namespace SF_HAL
