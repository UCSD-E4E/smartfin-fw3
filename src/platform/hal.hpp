/**
 * @file hal.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Platform abstraction layer declarations for the Smartfin firmware.
 * @date 2026-04-27
 *
 * Declares every hardware-facing operation the application needs. All
 * declarations live in the SF_HAL namespace; no Particle SDK symbols appear
 * here. Platform implementations provide the definitions in a paired .cpp
 * (e.g. particle/hal_particle.cpp, stm32/hal_stm32.cpp).
 *
 * Porting guide (new target):
 *   1. Create a directory alongside pc_hal/ (e.g. stm32_hal/).
 *   2. Implement every function declared below in one or more .cpp files.
 *   3. Add the new HAL directory first in the build system's include path so
 *      it shadows Particle.h for any files not yet migrated.
 *   4. Define PLATFORM_RETAINED appropriately before hal_types.hpp is parsed.
 */
#ifndef __SF_HAL_HPP__
#define __SF_HAL_HPP__

#include "hal_types.hpp"

#include <cstddef>
#include <cstdint>

namespace SF_HAL
{

// GPIO pin identifiers
// Replaces board-specific #defines (e.g. A4, D9) in product.hpp.
// Platform implementations map these to actual pin integers.

/**
 * @brief Logical GPIO pin identifiers.
 *
 * Each value names a physical net on the Smartfin board. Platform
 * implementations in src/platform/<target>/pins.hpp translate these to
 * the target-specific integer constants (e.g. Particle A4, D9).
 */
enum class PinId
{
    UsbPwrDetect,  ///< USB power-detect input (was SF_USB_PWR_DETECT_PIN)
    StatLed,       ///< Battery status LED output (was STAT_LED_PIN)
    WaterDetectEn, ///< Water-detect circuit enable output (was WATER_DETECT_EN_PIN)
    WaterDetect,   ///< Water-detect sense input (was WATER_DETECT_PIN)
    WaterStatusLed,///< Wet/dry status LED output (was WATER_STATUS_LED)
    WaterMfgTestEn,///< Manufacturing water-detect enable output (was WATER_MFG_TEST_EN)
    Wkp,           ///< Wake-from-sleep pin (was WKP_PIN)
};

// Timing
// Replaces: millis(), delay(), delayMicroseconds()

/**
 * @brief Returns milliseconds elapsed since boot.
 *
 * Wraps around after 49 days.  Callers must compute elapsed time via
 * subtraction (e.g. @c millis() - start) to handle rollover correctly.
 *
 * @return Milliseconds since device boot as a @c tick_t.
 */
tick_t millis();

/**
 * @brief Block the calling thread for at least @p ms milliseconds.
 *
 * On Particle, yields to the system loop during the wait.  On bare-metal,
 * this is a busy-wait or an RTOS sleep depending on the platform HAL.
 *
 * @param ms Duration to block in milliseconds.
 */
void delay_ms(uint32_t ms);

/**
 * @brief Block the calling thread for at least @p us microseconds.
 *
 * Intended for short hardware timing delays (e.g. sensor enable settle time).
 * Prefer @c delay_ms() for durations longer than a few milliseconds.
 *
 * @param us Duration to block in microseconds.
 */
void delay_us(uint32_t us);

// GPIO
// Replaces: pinMode(), digitalWrite(), digitalRead(),
//           digitalWriteFast(), pinReadFast()

/**
 * @brief Configure a pin's direction and pull resistor.
 *
 * Must be called before the first @c gpio_write() or @c gpio_read() on a pin.
 *
 * @param pin  Logical pin identifier.
 * @param mode Desired @c GpioMode.
 */
void gpio_set_mode(PinId pin, GpioMode mode);

/**
 * @brief Drive a pin to a logic level.
 * @param pin   Logical pin identifier.
 * @param state Desired @c GpioState (@c LOW or @c HIGH).
 */
void gpio_write(PinId pin, GpioState state);

/**
 * @brief Sample the current logic level on a pin.
 * @param pin Logical pin identifier.
 * @return @c true if the pin reads @c HIGH, @c false if @c LOW.
 */
bool gpio_read(PinId pin);

/**
 * @brief Drive a pin to a logic level using the fastest available path.
 *
 * On platforms with direct register access (e.g. Particle STM32) this
 * bypasses the GPIO abstraction layer for minimal latency.  Falls back to
 * @c gpio_write() on targets that do not have a dedicated fast path.
 *
 * @param pin   Logical pin identifier.
 * @param state Desired @c GpioState.
 */
void gpio_write_fast(PinId pin, GpioState state);

/**
 * @brief Sample a pin using the fastest available path.
 *
 * Same trade-off as @c gpio_write_fast(); falls back to @c gpio_read() where
 * no fast variant exists.
 *
 * @param pin Logical pin identifier.
 * @return @c true if the pin reads @c HIGH, @c false if @c LOW.
 */
bool gpio_read_fast(PinId pin);

// Serial / USART
// Replaces: Serial.begin(), Serial.available(), Serial.read(),
//           Serial.print(), Serial.write()

/**
 * @brief Initialise the debug/CLI serial port.
 *
 * Must be called once during system init before any other @c serial_* call.
 * On Particle this configures USB CDC; on bare-metal it initialises the UART
 * peripheral and, optionally, DMA.
 *
 * @param baud Baud rate in bits per second (e.g. 115200).
 */
void serial_begin(uint32_t baud);

/**
 * @brief Return the number of bytes waiting in the receive buffer.
 * @return Number of bytes available to read without blocking.
 */
int serial_available();

/**
 * @brief Read one byte from the receive buffer.
 * @return The next byte as an @c int, or @c -1 if no data is available.
 */
int serial_read();

/**
 * @brief Transmit a single character.
 * @param ch Character to send.
 */
void serial_print_char(char ch);

/**
 * @brief Transmit a raw byte buffer.
 * @param buf Pointer to the data to transmit.
 * @param len Number of bytes to send.
 */
void serial_write(const uint8_t* buf, std::size_t len);

// I2C
// Replaces: Wire.begin(), Wire.isEnabled(), Wire.beginTransmission(),
//           Wire.requestFrom(), Wire.read(), Wire.write(), Wire.endTransmission(),
//           Wire (direct reference)

} // namespace SF_HAL
// Forward declaration in global namespace — avoids including Particle.h / Wire.h.
class TwoWire;
namespace SF_HAL
{

/**
 * @brief Initialise the I2C (Wire) bus as controller.
 *
 * Must be called before any @c i2c_read() or @c i2c_write() transaction.
 */
void i2c_begin();

/**
 * @brief Return whether the I2C peripheral has been initialised.
 * @return @c true if @c i2c_begin() has been called successfully.
 */
bool i2c_is_enabled();

/**
 * @brief Read bytes from a device over I2C.
 *
 * Performs a write of the address followed by a repeated-start read of
 * @p length bytes into @p data.
 *
 * @param address     7-bit I2C device address.
 * @param data        Output buffer; must be at least @p length bytes.
 * @param length      Number of bytes to read.
 * @param repeated    If @c true, hold the bus (repeated start); release otherwise.
 * @return Number of bytes read, or a negative error code on failure.
 */
int i2c_read(uint8_t address, char* data, int length, bool repeated);

/**
 * @brief Write bytes to a device over I2C.
 *
 * Sends @p length bytes from @p data in a single transaction.
 *
 * @param address     7-bit I2C device address.
 * @param data        Data to transmit; must be at least @p length bytes.
 * @param length      Number of bytes to write.
 * @param repeated    If @c true, hold the bus (repeated start); release otherwise.
 * @return Number of bytes written, or a negative error code on failure.
 */
int i2c_write(uint8_t address, const char* data, int length, bool repeated);

/**
 * @brief Return the platform I2C bus handle.
 *
 * Required by drivers that accept a @c TwoWire reference directly (e.g. IMU).
 * Callers must include the appropriate platform header to use the returned
 * reference.
 *
 * @return Reference to the platform I2C peripheral.
 */
TwoWire& i2c_get_wire();

/**
 * @brief Acquire exclusive access to the I2C bus.
 *
 * Must be paired with @c i2c_unlock(). Callers that hold this lock must not
 * call any other @c i2c_* function that itself tries to acquire the lock, as
 * the underlying mutex is not reentrant.  On Particle this calls
 * @c Wire.lock(), matching the acquire side of @c WITH_LOCK(Wire).
 */
void i2c_lock();

/**
 * @brief Release exclusive access to the I2C bus.
 *
 * Must be called once for every preceding @c i2c_lock().  On Particle this
 * calls @c Wire.unlock(), matching the release side of @c WITH_LOCK(Wire).
 */
void i2c_unlock();

// Non-volatile memory
// Replaces: EEPROM.get(), EEPROM.put()

/**
 * @brief Read bytes from non-volatile storage.
 *
 * Addresses correspond directly to the byte offsets defined in
 * @c NVRAM::LAYOUT.  No bounds checking is performed; callers are responsible
 * for staying within the storage capacity of the target platform.
 *
 * @param addr Byte offset within NVM.
 * @param out  Destination buffer; must be at least @p len bytes.
 * @param len  Number of bytes to read.
 */
void nvm_read(uint32_t addr, void* out, std::size_t len);

/**
 * @brief Write bytes to non-volatile storage.
 *
 * On flash-backed platforms (e.g. Particle EEPROM emulation) this may trigger
 * an erase cycle; avoid calling in tight loops.
 *
 * @param addr Byte offset within NVM.
 * @param in   Source buffer; must be at least @p len bytes.
 * @param len  Number of bytes to write.
 */
void nvm_write(uint32_t addr, const void* in, std::size_t len);

// Cloud connectivity
// Replaces: Particle.connect/disconnect/connected/disconnected,
//           Particle.publish(), Particle.syncTime/syncTimeDone(),
//           Particle.process(), Cellular.isOn(), Cellular.ready()

/**
 * @brief Attempt to establish a cloud connection, blocking until success or timeout.
 *
 * @param timeout_ms Maximum time to wait for connection in milliseconds.
 * @return 0 on success, -1 on timeout.
 */
int cloud_connect(uint32_t timeout_ms);

/**
 * @brief Disconnect from the cloud, blocking until disconnected or timeout.
 * @param timeout_ms Maximum time to wait for clean disconnect in milliseconds.
 * @return 0 on success, -1 on timeout.
 */
int cloud_disconnect(uint32_t timeout_ms);

/**
 * @brief Return whether a cloud session is currently active.
 * @return @c true if connected, @c false otherwise.
 */
bool cloud_connected();

/**
 * @brief Publish a named event with a string payload.
 *
 * Callers must verify payload and name lengths against
 * @c SF_HAL::cloud::MAX_EVENT_DATA_LEN and @c SF_HAL::cloud::MAX_EVENT_NAME_LEN
 * before calling.  On platforms without cloud connectivity this is a no-op
 * that returns @c false.
 *
 * @param event Null-terminated event name.
 * @param data  Null-terminated payload string.
 * @return @c true if the publish was accepted by the platform, @c false otherwise.
 */
bool cloud_publish(const char* event, const char* data);

/**
 * @brief Request a time synchronisation from the cloud.
 *
 * Non-blocking; check completion with @c cloud_sync_time_done().
 */
void cloud_sync_time();

/**
 * @brief Return whether a pending time sync has completed.
 * @return @c true once the RTC has been updated following @c cloud_sync_time().
 */
bool cloud_sync_time_done();

/**
 * @brief Return whether the cellular modem is powered on.
 * @return @c true if the modem is on, @c false otherwise.
 */
bool cellular_is_on();

/**
 * @brief Return whether the cellular modem has a network connection.
 * @return @c true if the modem is registered and ready, @c false otherwise.
 */
bool cellular_is_ready();

/**
 * @brief Yield to the platform network stack.
 *
 * On Particle this calls @c Particle.process() to allow the system thread to
 * service the cloud connection.  On platforms with a true RTOS or no managed
 * network stack this is a no-op.  Call periodically inside blocking loops to
 * prevent connection timeouts.
 */
void cloud_process();

// BLE
// Replaces: BLE.on(), BLE.setDeviceName(), BLE.addCharacteristic(),
//           BLE.onConnected(), BLE.onDisconnected(), BLE.advertise(),
//           BLE.stopAdvertising(), BleCharacteristic::setValue()
//
// Concrete Particle BLE types (BleUuid, BleCharacteristic, etc.) are confined
// to src/platform/particle/ble.cpp behind a platform guard. sf_ble.cpp is
// fully platform-agnostic and calls only the functions declared here.

/**
 * @brief Register connection and write callbacks with the BLE backend.
 *
 * Must be called before @c ble_init() so that the backend can wire the
 * callbacks before any connection event can fire.
 *
 * @param callbacks Struct containing @c on_connection, @c on_write, and
 *                  @c context pointers.
 */
void ble_set_callbacks(const ble::Callbacks& callbacks);

/**
 * @brief Initialise the BLE stack and register the GATT service.
 *
 * Calls @c BLE.on(), sets the device name, and registers the telemetry
 * (NOTIFY) and control (WRITE_WO_RSP) characteristics.  Safe to call
 * multiple times; subsequent calls are no-ops.
 *
 * @param device_name         Null-terminated device name (max 20 bytes).
 * @param service_uuid        128-bit service UUID as a null-terminated string.
 * @param telemetry_char_uuid 128-bit telemetry characteristic UUID string.
 * @param control_char_uuid   128-bit control characteristic UUID string.
 * @return @c true on success, @c false if any argument is null or init fails.
 */
bool ble_init(const char* device_name,
              const char* service_uuid,
              const char* telemetry_char_uuid,
              const char* control_char_uuid);

/**
 * @brief Begin advertising the registered service.
 *
 * Builds an advertising payload from @p service_uuid and starts broadcasting.
 * Safe to call repeatedly; updates the device name in the scan response if
 * @p local_name is non-null.
 *
 * @param service_uuid 128-bit service UUID as a null-terminated string.
 * @param local_name   Null-terminated local name for the scan response.
 * @return @c true on success, @c false if not initialised or @p service_uuid
 *         is null.
 */
bool ble_advertise(const char* service_uuid, const char* local_name);

/**
 * @brief Stop BLE advertising.
 *
 * Does not affect any existing connection; only suppresses new connection
 * attempts.
 */
void ble_stop_advertising();

/**
 * @brief Return whether a central is currently connected.
 * @return @c true if a peer connection is active, @c false otherwise.
 */
bool ble_is_connected();

/**
 * @brief Send a NOTIFY payload on the telemetry characteristic.
 *
 * No-op if not initialised, not connected, @p data is null, or @p len is 0.
 *
 * @param data Pointer to the payload bytes.
 * @param len  Number of bytes to send.
 * @return @c true if the notification was accepted by the stack.
 */
bool ble_notify(const uint8_t* data, std::size_t len);

// System identity and control
// Replaces: System.deviceID(), System.version(), System.reset(),
//           System.sleep(), System.enableFeature(), System.batteryState()

/**
 * @brief Return the platform's unique device identifier string.
 *
 * On Particle this is the 24-character hex Device ID.  The returned pointer
 * is valid for the lifetime of the process; callers must not free it.
 *
 * @return Null-terminated device ID string.
 */
const char* system_device_id();

/**
 * @brief Return the platform OS or firmware version string.
 *
 * On Particle this is the Device OS semver string (e.g. "5.6.0").
 *
 * @return Null-terminated version string.
 */
const char* system_os_version();

/**
 * @brief Perform a software reset of the device.
 *
 * Does not return.  The reset reason will be reported as
 * @c ResetReason::USER on the next boot.
 */
void system_reset();

/**
 * @brief Enter a low-power sleep state.
 *
 * For @c SleepMode::SOFT_POWER_OFF the device wakes after @p duration_sec
 * seconds (or immediately if 0).  For @c SleepMode::HIBERNATE @p duration_sec
 * is ignored; use @c system_sleep_gpio_wake() instead.
 *
 * @param mode         Target sleep depth.
 * @param duration_sec Seconds to sleep before waking (SOFT_POWER_OFF only).
 */
void system_sleep(SleepMode mode, uint32_t duration_sec = 0);

/**
 * @brief Enter hibernate sleep and wake on a rising GPIO edge.
 *
 * Replaces the @c SystemSleepConfiguration pattern:
 * @code
 *   config.mode(SystemSleepMode::HIBERNATE).gpio(pin, RISING);
 *   System.sleep(config);
 * @endcode
 * All RAM content except PLATFORM_RETAINED variables is lost.
 *
 * @param wake_pin Logical pin identifier to monitor for the wake edge.
 */
void system_sleep_gpio_wake(PinId wake_pin);

// Battery / fuel gauge
// Replaces: FuelGauge::getVCell(), System.batteryState()

/**
 * @brief Return the battery terminal voltage in volts.
 *
 * Read from the fuel gauge IC (e.g. MAX17043 on Particle Boron).  Returns
 * @c 0.0f on platforms without a fuel gauge.
 *
 * @return Battery voltage in volts.
 */
float battery_voltage();

/**
 * @brief Return the current battery / charger state.
 * @return A @c BatteryState enum value; @c BatteryState::UNKNOWN if the
 *         platform cannot determine state.
 */
BatteryState battery_state();

// Reset diagnostics
// Replaces: System.resetReason(), System.resetReasonData(),
//           System.enableFeature(FEATURE_RESET_INFO)

/**
 * @brief Enable reset-cause recording.
 *
 * Must be called during system initialisation (before any potential reset)
 * so that the platform preserves the reset reason across the next reboot.
 * Replaces @c System.enableFeature(FEATURE_RESET_INFO) on Particle.
 */
void reset_info_enable();

/**
 * @brief Return the cause of the most recent reset.
 *
 * Valid only after @c reset_info_enable() has been called on a prior boot.
 *
 * @return @c ResetReason enum value.
 */
ResetReason reset_reason();

/**
 * @brief Return platform-specific auxiliary data associated with the last reset.
 *
 * For @c ResetReason::PANIC this contains the fault code.  Interpretation is
 * platform-dependent; refer to the Particle Device OS docs for Particle
 * targets.
 *
 * @return 32-bit auxiliary reset data word; @c 0 if unavailable.
 */
uint32_t reset_reason_data();

// Threading
// Replaces: Particle Thread class, os_thread_yield()

/**
 * @brief Function pointer type for thread entry points.
 *
 * Matches the signature expected by Particle's @c Thread constructor and by
 * FreeRTOS / CMSIS-RTOS task functions.
 */
using ThreadFn = void (*)(void*);

/**
 * @brief Create and immediately start a new thread.
 *
 * The returned handle is opaque; on Particle it wraps a @c Thread object, on
 * FreeRTOS it wraps a @c TaskHandle_t.  Callers do not need to free it for
 * threads that run indefinitely (the current usage pattern).
 *
 * @param name       Null-terminated debug name shown in the thread list.
 * @param function   Thread entry point; must never return.
 * @param param      Opaque argument forwarded to @p function.
 * @param priority   Scheduling priority; defaults to @c ThreadPriority::NORMAL.
 * @param stack_size Stack allocation in bytes; defaults to
 *                   @c THREAD_STACK_SIZE_DEFAULT.
 * @return Opaque thread handle, or @c nullptr if creation failed.
 */
void* thread_create(const char* name,
                    ThreadFn function,
                    void* param = nullptr,
                    ThreadPriority priority = ThreadPriority::NORMAL,
                    std::size_t stack_size = THREAD_STACK_SIZE_DEFAULT);

/**
 * @brief Yield the calling thread to allow other threads to run.
 *
 * Replaces the @c os_thread_yield() macro and the @c delay(1) idiom used in
 * tight loops.  On cooperative schedulers this is mandatory to prevent
 * starvation; on preemptive schedulers it is a hint only.
 */
void thread_yield();

/**
 * @brief Platform-agnostic mutual exclusion primitive.
 *
 * Satisfies the C++ @c BasicLockable concept, so instances can be used
 * directly with @c std::lock_guard and the @c WITH_LOCK macro.  The
 * underlying OS handle is allocated in the constructor and released in
 * the destructor; do not copy or move instances.
 */
class Mutex
{
public:
    /**
     * @brief Construct and initialise the underlying OS mutex.
     */
    Mutex();

    /**
     * @brief Destroy the mutex and release the underlying OS handle.
     */
    ~Mutex();

    /**
     * @brief Acquire the mutex, blocking until it becomes available.
     */
    void lock();

    /**
     * @brief Release the mutex.
     */
    void unlock();

private:
    /**
     * @brief Opaque handle to the platform mutex object.
     */
    void *_handle;
};

} // namespace SF_HAL

#endif // __SF_HAL_HPP__
