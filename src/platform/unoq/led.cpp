/**
 * @file led.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Antigravity
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL::LedStatus and
 *        SF_HAL::LedSystemTheme.
 * @date 2026-07-23
 *
 * Controls the on-board RGB user LEDs on the Uno Q (red:user, green:user,
 * blue:user via Linux sysfs) according to the active LedStatus color.
 * Maintains the empty-class-plus-address-pool pattern required by hal_types.hpp.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <mutex>

namespace SF_HAL
{

namespace
{

constexpr int POOL_SIZE = 8;

struct PoolEntry
{
    SF_HAL::LedStatus* owner = nullptr;
    uint32_t           color = 0;
    LedPattern         pattern = LedPattern::SOLID;
    uint16_t           period_ms = 0;
    LedPriority        priority = LedPriority::NORMAL;
    bool               active = false;
    uint64_t           seq = 0;
};

PoolEntry pool[POOL_SIZE];
std::mutex led_mutex;
static uint64_t global_led_seq = 0;

void write_sysfs_led(const char* name, int val)
{
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", name);
    FILE* f = fopen(path, "w");
    if (f)
    {
        fprintf(f, "%d\n", val ? 1 : 0);
        fclose(f);
    }
}

void update_hardware_led_locked()
{
    PoolEntry* best = nullptr;
    for (auto& e : pool)
    {
        if (e.owner != nullptr && e.active)
        {
            if (best == nullptr)
            {
                best = &e;
            }
            else if (static_cast<int>(e.priority) > static_cast<int>(best->priority))
            {
                best = &e;
            }
            else if (static_cast<int>(e.priority) == static_cast<int>(best->priority) && e.seq > best->seq)
            {
                best = &e;
            }
        }
    }

    uint32_t color = (best && best->active) ? best->color : 0;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    write_sysfs_led("red:user", r > 0 ? 1 : 0);
    write_sysfs_led("green:user", g > 0 ? 1 : 0);
    write_sysfs_led("blue:user", b > 0 ? 1 : 0);
}

PoolEntry& resolve_locked(SF_HAL::LedStatus* self)
{
    for (auto& e : pool)
    {
        if (e.owner == self)
        {
            return e;
        }
    }
    for (auto& e : pool)
    {
        if (e.owner == nullptr)
        {
            e.owner = self;
            e.color = 0;
            e.pattern = LedPattern::SOLID;
            e.period_ms = 0;
            e.priority = LedPriority::NORMAL;
            e.active = false;
            e.seq = 0;
            return e;
        }
    }
    // Fallback: overwrite first slot if pool exhausted
    pool[0].owner = self;
    return pool[0];
}

} // namespace

LedStatus::~LedStatus()
{
    std::lock_guard<std::mutex> lock(led_mutex);
    for (auto& e : pool)
    {
        if (e.owner == this)
        {
            e.owner = nullptr;
            e.active = false;
            break;
        }
    }
    update_hardware_led_locked();
}

void LedStatus::setColor(uint32_t color)
{
    std::lock_guard<std::mutex> lock(led_mutex);
    PoolEntry& e = resolve_locked(this);
    e.color = color;
    if (e.active)
    {
        e.seq = ++global_led_seq;
    }
    update_hardware_led_locked();
}

void LedStatus::setPattern(LedPattern pattern)
{
    std::lock_guard<std::mutex> lock(led_mutex);
    PoolEntry& e = resolve_locked(this);
    e.pattern = pattern;
}

void LedStatus::setPeriod(uint16_t period_ms)
{
    std::lock_guard<std::mutex> lock(led_mutex);
    PoolEntry& e = resolve_locked(this);
    e.period_ms = period_ms;
}

void LedStatus::setPriority(LedPriority priority)
{
    std::lock_guard<std::mutex> lock(led_mutex);
    PoolEntry& e = resolve_locked(this);
    e.priority = priority;
    update_hardware_led_locked();
}

void LedStatus::setActive(bool active)
{
    std::lock_guard<std::mutex> lock(led_mutex);
    PoolEntry& e = resolve_locked(this);
    e.active = active;
    if (active)
    {
        e.seq = ++global_led_seq;
    }
    update_hardware_led_locked();
}

LedSystemTheme::LedSystemTheme()
{
}

void LedSystemTheme::setSignal(LedSignal signal, uint32_t color)
{
    (void)signal;
    (void)color;
}

void LedSystemTheme::setSignal(LedSignal signal,
                               uint32_t color,
                               LedPattern pattern,
                               LedSpeed speed)
{
    (void)signal;
    (void)color;
    (void)pattern;
    (void)speed;
}

void LedSystemTheme::setSignal(LedSignal signal,
                               uint32_t color,
                               LedPattern pattern,
                               uint16_t period_ms)
{
    (void)signal;
    (void)color;
    (void)pattern;
    (void)period_ms;
}

void LedSystemTheme::apply()
{
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
