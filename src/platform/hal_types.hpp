/**
 * @file hal_types.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Platform-agnostic type definitions for the Smartfin HAL.
 * @date 2026-04-27
 *
 * This header is the single source of truth for all types, enums, and
 * constants that application code uses in place of Particle SDK types.
 * No Particle.h or platform-specific header should be included here.
 *
 * Platform implementations must:
 *   - Provide definitions for all declared (but undefined) class methods.
 *   - Define PLATFORM_RETAINED before including this header when the target
 *     platform uses a keyword or attribute to mark retention across resets
 *     (e.g. `retained` on Particle, a linker-section attribute on bare-metal).
 */
#ifndef __SF_HAL_TYPES_HPP__
#define __SF_HAL_TYPES_HPP__

#include <cstddef>
#include <cstdint>

/**
 * @def PLATFORM_RETAINED
 * @brief Marks a variable as surviving sleep and warm-reset cycles.
 *
 * On Particle this expands to the `retained` storage-class modifier, which
 * places the variable in a dedicated SRAM region that the boot ROM does not
 * zero on wake.  On bare-metal targets, define this to the appropriate
 * linker-section or compiler attribute before including hal_types.hpp
 * (e.g. `__attribute__((section(".noinit")))`).
 *
 * Defaults to empty (no retention guarantee) so PC/host builds compile
 * without modification.
 */
#ifndef PLATFORM_RETAINED
#ifdef PARTICLE
// On Particle Device OS (nRF52840), `retained` expands to this attribute.
// Defined directly here so no Particle SDK header needs to be included in
// this platform-agnostic file.
#define PLATFORM_RETAINED __attribute__((section(".retained_user"), zero_init))
#else
#define PLATFORM_RETAINED
#endif
#endif

namespace SF_HAL
{

// Timing

/**
 * @brief Millisecond tick counter type.
 *
 * Replaces Particle's @c system_tick_t.  Rolls over after ~49 days; callers
 * that measure elapsed time must handle wrap-around with subtraction rather
 * than comparison (i.e. @c (millis() - start) >= timeout).
 */
using tick_t = uint32_t;

// GPIO

/**
 * @brief Pin direction and pull configuration.
 *
 * Replaces the bare integer constants passed to Particle's @c pinMode().
 */
enum class GpioMode : uint8_t
{
    INPUT = 0,          ///< Floating input, no internal pull resistor.
    INPUT_PULLDOWN = 1, ///< Input with internal pull-down resistor enabled.
    OUTPUT = 2,         ///< Push-pull output.
};

/**
 * @brief Logic level for GPIO read/write operations.
 *
 * Replaces the @c LOW / @c HIGH macros used with @c digitalWrite() and
 * @c digitalRead().
 */
enum class GpioState : uint8_t
{
    LOW = 0,  ///< Logic low (0 V reference).
    HIGH = 1, ///< Logic high (VDD reference).
};

// LED

/**
 * @brief LED animation pattern.
 *
 * Mirrors Particle's @c LEDPattern enum values so that existing numeric
 * constants can be mapped 1:1 during migration.
 */
enum class LedPattern : uint8_t
{
    INVALID = 0, ///< Sentinel; should not be set intentionally.
    SOLID = 1,   ///< Constant on at the configured colour.
    BLINK = 2,   ///< On/off square-wave at the configured period.
    FADE = 3,    ///< Sinusoidal brightness fade at the configured period.
};

/**
 * @brief LED display priority relative to other active statuses.
 *
 * Higher-priority statuses override lower-priority ones.  Values match
 * Particle's @c LEDPriority enum exactly so that @c static_cast is safe
 * in the Particle HAL implementation.
 */
enum class LedPriority : uint8_t
{
    BACKGROUND = 10, ///< Below all normal UI signals.
    NORMAL     = 20, ///< Default application priority.
    IMPORTANT  = 30, ///< Overrides normal system LED signals.
    CRITICAL   = 40, ///< Overrides everything, including system signals.
};

/**
 * @brief Preset animation speed hint for LED signals.
 *
 * Used when a pattern period is not specified explicitly.  Mirrors Particle's
 * @c LEDSpeed enum.
 */
enum class LedSpeed : uint8_t
{
    NORMAL = 0, ///< Platform-defined default animation rate.
};

/**
 * @brief System-level LED signal identifiers.
 *
 * These signals correspond to well-known device states (network, cloud) whose
 * visual representation is controlled by @c LedSystemTheme.  Mirrors
 * Particle's @c LEDSignal enum.
 */
enum class LedSignal : uint8_t
{
    NETWORK_OFF,        ///< Radio is powered off.
    NETWORK_ON,         ///< Radio is on but not yet connecting.
    NETWORK_CONNECTING, ///< Actively attempting to join the network.
    NETWORK_DHCP,       ///< Network joined; waiting for IP address.
    NETWORK_CONNECTED,  ///< IP address obtained.
    CLOUD_CONNECTING,   ///< TCP/TLS handshake with cloud in progress.
    CLOUD_CONNECTED,    ///< Cloud session established.
    CLOUD_HANDSHAKE,    ///< Particle protocol handshake in progress.
};

/**
 * @brief Packed RGB colour constants (0x00RRGGBB).
 *
 * Values match the Particle @c RGB_COLOR_* macros so callers can be migrated
 * with a mechanical find-and-replace.
 */
namespace color
{
    constexpr uint32_t BLUE = 0x000000FFu;   ///< Pure blue.
    constexpr uint32_t ORANGE = 0x00FFA500u; ///< Orange.
    constexpr uint32_t RED = 0x00FF0000u;    ///< Pure red.
    constexpr uint32_t YELLOW = 0x00FFFF00u; ///< Red + green = yellow.
    constexpr uint32_t WHITE = 0x00FFFFFFu;  ///< Full RGB = white.
} // namespace color

/**
 * @brief Per-task LED status controller.
 *
 * Application code holds @c LedStatus by value inside task classes.  The
 * platform HAL implementation provides the full class definition (including
 * any private state) and the method bodies; this declaration establishes the
 * interface contract only.
 *
 * @note Because @c LedStatus is held by value, all platforms must agree on
 * its memory layout.  The required implementation contract is the
 * empty-class-plus-address-pool pattern: @c LedStatus carries no data
 * members (@c sizeof == 1), and each platform implementation maps instance
 * addresses to platform-specific LED objects using a fixed-size internal
 * pool.  A future platform that cannot follow this pattern must switch to
 * pImpl (add a @c void* handle member here) and update all existing
 * implementations to match.
 *
 * Replaces Particle's @c LEDStatus class.
 */
class LedStatus
{
public:
    /**
     * @brief Release the platform LED object back to the HAL pool.
     */
    ~LedStatus();

    /**
     * @brief Set the solid or base colour for this status.
     * @param color Packed RGB value (0x00RRGGBB).
     */
    void setColor(uint32_t color);

    /**
     * @brief Set the animation pattern (solid, blink, fade).
     * @param pattern Desired @c LedPattern.
     */
    void setPattern(LedPattern pattern);

    /**
     * @brief Set the animation cycle period.
     * @param period_ms Full cycle duration in milliseconds.
     */
    void setPeriod(uint16_t period_ms);

    /**
     * @brief Set the display priority relative to other active statuses.
     * @param priority Desired @c LedPriority.
     */
    void setPriority(LedPriority priority);

    /**
     * @brief Activate or deactivate this status.
     * @param active @c true to make this status visible (default), @c false
     *               to suppress it without destroying the configuration.
     */
    void setActive(bool active = true);
};

/**
 * @brief System-wide LED theme that maps network/cloud signals to colours.
 *
 * One instance is typically created during system initialisation to configure
 * the visual meaning of each @c LedSignal.  Replaces Particle's
 * @c LEDSystemTheme class.
 */
class LedSystemTheme
{
public:
    LedSystemTheme();

    /**
     * @brief Assign a solid colour to a system signal.
     * @param signal The system state to configure.
     * @param color  Packed RGB value (0x00RRGGBB).
     */
    void setSignal(LedSignal signal, uint32_t color);

    /**
     * @brief Assign a colour and animation pattern to a system signal.
     * @param signal  The system state to configure.
     * @param color   Packed RGB value (0x00RRGGBB).
     * @param pattern Animation pattern to apply.
     * @param speed   Preset speed hint; defaults to @c LedSpeed::NORMAL.
     */
    void setSignal(LedSignal signal,
                   uint32_t color,
                   LedPattern pattern,
                   LedSpeed speed = LedSpeed::NORMAL);

    /**
     * @brief Assign a colour, animation pattern, and explicit period to a system signal.
     * @param signal    The system state to configure.
     * @param color     Packed RGB value (0x00RRGGBB).
     * @param pattern   Animation pattern to apply.
     * @param period_ms Full animation cycle duration in milliseconds.
     */
    void setSignal(LedSignal signal,
                   uint32_t color,
                   LedPattern pattern,
                   uint16_t period_ms);

    /**
     * @brief Activate this theme as the system-wide LED theme.
     *
     * Must be called once after all @c setSignal() calls are complete.
     * On Particle this calls @c LEDSystemTheme::apply().
     */
    void apply();
};

// Battery

/**
 * @brief Battery / charger state reported by the fuel gauge or PMIC.
 *
 * Mirrors Particle's @c battery_state_t values so numeric assignments are
 * preserved during migration.
 */
enum class BatteryState : uint8_t
{
    UNKNOWN = 0,      ///< State could not be determined.
    NOT_CHARGING = 1, ///< External power present but charge current is zero.
    CHARGING = 2,     ///< Actively accepting charge current.
    CHARGED = 3,      ///< Charge complete; full capacity reached.
    DISCHARGING = 4,  ///< Running on battery, no external power.
    FAULT = 5,        ///< Charger or fuel gauge reported an error.
    DISCONNECTED = 6, ///< No battery detected.
};

// Reset reasons

/**
 * @brief Cause of the most recent device reset.
 *
 * Mirrors Particle's @c __RESET_REASON_t values.  Retrieved via
 * @c SF_HAL::reset_reason() after calling @c SF_HAL::reset_info_enable()
 * during startup.
 */
enum class ResetReason : uint8_t
{
    NONE = 0,             ///< No reset has occurred (first boot or unknown).
    PIN_RESET = 1,        ///< External RESET pin asserted.
    POWER_MANAGEMENT = 2, ///< Reset triggered by the power management IC.
    POWER_DOWN = 3,       ///< Supply voltage dropped below POR threshold.
    POWER_BROWNOUT = 4,   ///< Brownout detector fired.
    WATCHDOG = 5,         ///< Hardware or software watchdog expired.
    UPDATE = 6,           ///< OTA firmware update applied.
    UPDATE_TIMEOUT = 7,   ///< OTA update timed out before completion.
    FACTORY_RESET = 8,    ///< User or cloud triggered factory reset.
    SAFE_MODE = 9,        ///< Device booted into safe / recovery mode.
    DFU_MODE = 10,        ///< Device booted into DFU (USB flashing) mode.
    PANIC = 11,           ///< Firmware panic / hard fault.
    USER = 12,            ///< Application called @c SF_HAL::system_reset().
    UNKNOWN = 13,         ///< Reset cause could not be determined.
};

// Sleep

/**
 * @brief Sleep depth / wake-source configuration.
 *
 * The two modes correspond directly to Particle sleep primitives and reflect
 * the two distinct code paths in @c sleepTask.cpp.
 */
enum class SleepMode : uint8_t
{
    /**
     * @brief Low-power sleep with optional timed wake.
     *
     * Equivalent to Particle's @c SLEEP_MODE_SOFTPOWEROFF.  Peripherals are
     * powered down but the RTC continues running, allowing the device to wake
     * after @c duration_sec seconds.
     */
    SOFT_POWER_OFF = 0,

    /**
     * @brief Deepest sleep; wake only via GPIO edge.
     *
     * Equivalent to Particle's @c SystemSleepMode::HIBERNATE.  All RAM
     * content is lost except PLATFORM_RETAINED variables.  Use
     * @c SF_HAL::system_sleep_gpio_wake() for this mode.
     */
    HIBERNATE = 1,
};

// Cloud

/**
 * @brief Cloud publish payload size limits.
 *
 * Replaces @c particle::protocol::MAX_EVENT_DATA_LENGTH and
 * @c particle::protocol::MAX_EVENT_NAME_LENGTH.  Callers should validate
 * string lengths against these constants before calling
 * @c SF_HAL::cloud_publish().
 */
namespace cloud
{
    /// Maximum number of bytes in a publish payload (excluding null terminator).
    constexpr std::size_t MAX_EVENT_DATA_LEN = 1024;

    /// Maximum number of bytes in a publish event name (excluding null terminator).
    constexpr std::size_t MAX_EVENT_NAME_LEN = 64;
} // namespace cloud

namespace ble
{

/**
 * @brief Opaque handle identifying a registered BLE service.
 *
 * Value 0 is reserved as an invalid handle.
 */
using ServiceHandle = uint16_t;

/**
 * @brief Opaque handle identifying a registered BLE characteristic.
 *
 * Value 0 is reserved as an invalid handle.
 */
using CharHandle = uint16_t;

/**
 * @brief 128-bit BLE UUID in raw byte form.
 *
 * The platform backend translates this neutral representation into its native
 * SDK UUID type.
 */
struct Uuid128
{
    uint8_t bytes[16];
};

/**
 * @brief Characteristic property bitmask.
 */
enum class CharProperty : uint8_t
{
    NONE = 0,
    READ = 1u << 0,
    WRITE = 1u << 1,
    WRITE_NO_RSP = 1u << 2,
    NOTIFY = 1u << 3,
    INDICATE = 1u << 4,
};

/**
 * @brief Combine characteristic property flags.
 * @param lhs Left-hand property.
 * @param rhs Right-hand property.
 * @return Combined property bitmask.
 */
inline constexpr CharProperty operator|(CharProperty lhs, CharProperty rhs)
{
    return static_cast<CharProperty>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

/**
 * @brief Return true when @p value contains @p flag.
 * @param value Bitmask to test.
 * @param flag  Property bit to check for.
 * @return @c true if @p flag is set in @p value.
 */
inline constexpr bool has_property(CharProperty value, CharProperty flag)
{
    return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
}

/**
 * @brief Characteristic registration descriptor.
 */
struct CharacteristicSpec
{
    Uuid128 uuid;          ///< 128-bit characteristic UUID.
    CharProperty properties; ///< Allowed operations bitmask.
    std::size_t max_len;   ///< Maximum value length in bytes.
};

/**
 * @brief Service registration descriptor.
 *
 * The @c characteristics pointer must remain valid for the duration of the
 * registration call; the backend copies or translates it as needed.
 */
struct ServiceSpec
{
    Uuid128 uuid;                            ///< 128-bit service UUID.
    const CharacteristicSpec* characteristics; ///< Array of characteristics.
    std::size_t characteristic_count;        ///< Length of @c characteristics.
};

/**
 * @brief Advertising configuration for a BLE peripheral.
 */
struct AdvertisingParams
{
    const Uuid128* primary_service_uuid; ///< UUID to include in the AD payload.
    const char* local_name;              ///< Null-terminated scan-response name.
    bool connectable;                    ///< Whether to allow connections.
};

/**
 * @brief Coarse BLE stack lifecycle state.
 */
enum class State : uint8_t
{
    OFF = 0,
    IDLE,
    ADVERTISING,
    CONNECTED,
};

/**
 * @brief Callback invoked when a central connects or disconnects.
 *
 * @param connected true on connect, false on disconnect.
 * @param context Caller-supplied context pointer.
 */
using ConnectionCallback = void (*)(bool connected, void* context);

/**
 * @brief Callback invoked when a peer writes to a characteristic.
 *
 * @param characteristic Handle of the characteristic that received the write.
 * @param data Raw payload bytes.
 * @param len Payload length in bytes.
 * @param context Caller-supplied context pointer.
 */
using WriteCallback = void (*)(CharHandle characteristic,
                               const uint8_t* data,
                               std::size_t len,
                               void* context);

/**
 * @brief Callback bundle registered with the BLE backend.
 */
struct Callbacks
{
    ConnectionCallback on_connection; ///< Fired on connect/disconnect.
    WriteCallback on_write;           ///< Fired when a peer writes a characteristic.
    void* context;                    ///< Passed verbatim to both callbacks.
};

} // namespace ble

// Threading

/**
 * @brief Thread scheduling priority.
 *
 * Mirrors Particle's @c os_thread_prio_t.  Only @c DEFAULT is used in the
 * current codebase; additional levels can be added as needed.
 */
enum class ThreadPriority : uint8_t
{
    NORMAL = 2, ///< Standard application thread priority (Particle: OS_THREAD_PRIORITY_DEFAULT).
};

/// Default thread stack size in bytes. Replaces @c OS_THREAD_STACK_SIZE_DEFAULT.
constexpr std::size_t THREAD_STACK_SIZE_DEFAULT = 3 * 1024;

/// Larger stack for threads with deeper call graphs. Replaces @c OS_THREAD_STACK_SIZE_DEFAULT_HIGH.
constexpr std::size_t THREAD_STACK_SIZE_DEFAULT_HIGH = 4 * 1024;

} // namespace SF_HAL

#endif // __SF_HAL_TYPES_HPP__
