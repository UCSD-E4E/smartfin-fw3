/**
 * @file ble.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated (Reviewed) by Brent Brewster
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL BLE functions.
 * @date 2026-09-22
 *
 * The BLE radio (WCBN3536A) is handled entirely by Linux via BlueZ; the
 * STM32U585 is not involved. Concrete BlueZ types (D-Bus proxies, GATT
 * application objects) are confined to this file, matching how
 * platform/particle/ble.cpp confines Particle's BLE types.
 *
 * BlueZ exposes GATT peripherals through its D-Bus API (GattManager1,
 * LEAdvertisingManager1) rather than raw HCI sockets: bluetoothd owns the
 * controller's HCI channel exclusively, and the ATT/GATT server logic
 * lives in bluetoothd itself, not the kernel. This backend registers a
 * GATT application and an advertisement as D-Bus objects under
 * /org/smartfin/ble0 and relies on sd-bus's object manager helper to
 * answer BlueZ's GetManagedObjects() introspection automatically, rather
 * than implementing org.freedesktop.DBus.ObjectManager by hand.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <systemd/sd-bus.h>
#include <vector>

namespace
{

    constexpr const char *kBluezService = "org.bluez";
    constexpr const char *kAdapterPath = "/org/bluez/hci0";
    constexpr const char *kAppRootPath = "/org/smartfin/ble0";
    constexpr const char *kServicePath = "/org/smartfin/ble0/service0";
    constexpr const char *kTelemetryCharPath = "/org/smartfin/ble0/service0/char0";
    constexpr const char *kControlCharPath = "/org/smartfin/ble0/service0/char1";
    constexpr const char *kAdvertisementPath = "/org/smartfin/ble0/advertisement0";

    // callbacks_ is written once by ble_set_callbacks() before ble_init() and
    // read-only from the sd-bus pump thread after that. Unlike the Particle
    // backend this is not asserted single-writer at runtime: hal.hpp documents
    // the double-set assertion as Particle-specific behaviour.
    SF_HAL::ble::Callbacks callbacks_{nullptr, nullptr, nullptr};

    /**
     * @brief BlueZ D-Bus BLE backend: owns the sd-bus connection and the GATT
     *        application/advertisement objects, and routes BlueZ method calls
     *        to the registered SF_HAL callbacks.
     *
     * Confined to this translation unit so no sd-bus/BlueZ types leak outward.
     * sd-bus connections are not safe for concurrent use from multiple
     * threads; busMutex_ serialises the background pump thread's
     * sd_bus_process() calls against sd_bus_call_method()/
     * sd_bus_add_object_vtable()/sd_bus_emit_properties_changed() calls made
     * from whichever application thread calls ble_advertise()/ble_notify().
     */
    class BlueZBleBackend
    {
    public:
        /**
         * @brief Return the singleton backend instance.
         */
        static BlueZBleBackend &getInstance()
        {
            static BlueZBleBackend instance;
            return instance;
        }

        /**
         * @brief Open the system bus, register the GATT application, and start
         *        the sd-bus pump thread.
         *
         * Idempotent: subsequent calls return true immediately without
         * re-registering.
         *
         * @param device_name         Adapter alias to broadcast.
         * @param service_uuid        128-bit service UUID string.
         * @param telemetry_char_uuid 128-bit telemetry characteristic UUID string.
         * @param control_char_uuid   128-bit control characteristic UUID string.
         * @return true on success, false if any D-Bus call or registration fails.
         */
        bool init(const char *device_name,
                  const char *service_uuid,
                  const char *telemetry_char_uuid,
                  const char *control_char_uuid)
        {
            if (initialized_)
            {
                return true;
            }

            serviceUuid_ = service_uuid;
            telemetryUuid_ = telemetry_char_uuid;
            controlUuid_ = control_char_uuid;

            if (sd_bus_open_system(&bus_) < 0)
            {
                return false;
            }

            {
                std::lock_guard<std::mutex> lock(busMutex_);

                if (!registerObjects())
                {
                    return false;
                }

                sd_bus_error error = SD_BUS_ERROR_NULL;
                sd_bus_set_property(bus_,
                                    kBluezService,
                                    kAdapterPath,
                                    "org.bluez.Adapter1",
                                    "Alias",
                                    &error,
                                    "s",
                                    device_name);
                sd_bus_error_free(&error);
            }

            // The pump thread must be running before RegisterApplication:
            // BlueZ calls GetManagedObjects() back on us before it replies.
            pumpThread_ =
                SF_HAL::thread_create("ble_dbus", &BlueZBleBackend::pumpThreadEntry, this);
            if (!pumpThread_)
            {
                return false;
            }

            if (!callRegister("org.bluez.GattManager1",
                              "RegisterApplication",
                              kAppRootPath,
                              appRegResult_))
            {
                return false;
            }

            initialized_ = true;
            return true;
        }

        /**
         * @brief Register (or re-register) the advertisement object and start
         *        advertising it via BlueZ.
         *
         * @param service_uuid 128-bit service UUID as a null-terminated string.
         * @param local_name   Null-terminated scan-response name, or null.
         * @return true if BlueZ accepted the advertisement, false otherwise.
         */
        bool advertise(const char *service_uuid, const char *local_name)
        {
            if (!initialized_)
            {
                return false;
            }

            advertisedServiceUuid_ = service_uuid;
            localName_ = local_name ? local_name : "";

            {
                std::lock_guard<std::mutex> lock(busMutex_);

                if (!advSlot_)
                {
                    sd_bus_slot *slot = nullptr;
                    if (sd_bus_add_object_vtable(bus_,
                                                 &slot,
                                                 kAdvertisementPath,
                                                 "org.bluez.LEAdvertisement1",
                                                 advertisementVtable_,
                                                 this) < 0)
                    {
                        return false;
                    }
                    advSlot_ = slot;
                }
            }

            return callRegister("org.bluez.LEAdvertisingManager1",
                                "RegisterAdvertisement",
                                kAdvertisementPath,
                                advRegResult_);
        }

        /**
         * @brief Unregister the advertisement from BlueZ.
         */
        void stopAdvertising()
        {
            if (!initialized_)
            {
                return;
            }

            std::lock_guard<std::mutex> lock(busMutex_);

            sd_bus_error error = SD_BUS_ERROR_NULL;
            sd_bus_call_method(bus_,
                               kBluezService,
                               kAdapterPath,
                               "org.bluez.LEAdvertisingManager1",
                               "UnregisterAdvertisement",
                               &error,
                               nullptr,
                               "o",
                               kAdvertisementPath);
            sd_bus_error_free(&error);
        }

        /**
         * @brief Update the telemetry characteristic's Value and emit a
         *        PropertiesChanged signal so subscribed centrals get a NOTIFY.
         *
         * @param data Payload bytes.
         * @param len  Number of bytes to send.
         * @return true if the signal was emitted, false if not initialised or
         *         @p data/@p len are empty.
         */
        bool notifyTelemetry(const uint8_t *data, std::size_t len)
        {
            if (!initialized_ || !data || len == 0)
            {
                return false;
            }

            {
                std::lock_guard<std::mutex> valueLock(valueMutex_);
                telemetryValue_.assign(data, data + len);
            }

            std::lock_guard<std::mutex> lock(busMutex_);
            return sd_bus_emit_properties_changed(bus_,
                                                  kTelemetryCharPath,
                                                  "org.bluez.GattCharacteristic1",
                                                  "Value",
                                                  nullptr) >= 0;
        }

    private:
        BlueZBleBackend() = default;

        /**
         * @brief Register the GATT service/characteristic objects and the
         *        object manager that advertises them to BlueZ.
         * @return true if every registration succeeded.
         */
        bool registerObjects()
        {
            if (sd_bus_add_object_manager(bus_, &objectManagerSlot_, kAppRootPath) < 0)
            {
                return false;
            }
            if (sd_bus_add_object_vtable(bus_,
                                         &serviceSlot_,
                                         kServicePath,
                                         "org.bluez.GattService1",
                                         serviceVtable_,
                                         this) < 0)
            {
                return false;
            }
            if (sd_bus_add_object_vtable(bus_,
                                         &telemetryCharSlot_,
                                         kTelemetryCharPath,
                                         "org.bluez.GattCharacteristic1",
                                         telemetryCharVtable_,
                                         this) < 0)
            {
                return false;
            }
            if (sd_bus_add_object_vtable(bus_,
                                         &controlCharSlot_,
                                         kControlCharPath,
                                         "org.bluez.GattCharacteristic1",
                                         controlCharVtable_,
                                         this) < 0)
            {
                return false;
            }
            return true;
        }

        /**
         * @brief Reply handler for callRegister(); runs on the pump thread
         *        inside sd_bus_process().
         * @param m Method return or error reply from BlueZ.
         * @param userdata Raw @c std::atomic<int> result pointer.
         * @return 0 (reply consumed).
         */
        static int onRegisterReply(sd_bus_message *m, void *userdata, sd_bus_error *)
        {
            auto *result = static_cast<std::atomic<int> *>(userdata);
            const sd_bus_error *e = sd_bus_message_get_error(m);
            if (e)
            {
                std::fprintf(stderr, "BlueZ register failed: %s: %s\n", e->name, e->message);
            }
            result->store(e ? -1 : 1);
            return 0;
        }

        /**
         * @brief Send @p method(@p path, {}) to the BlueZ adapter
         *        asynchronously and wait for the pump thread to deliver the
         *        reply.
         *
         * BlueZ calls back into our exported objects before replying to
         * RegisterApplication/RegisterAdvertisement, so a blocking
         * sd_bus_call_method() here would deadlock until the D-Bus timeout.
         * busMutex_ is held only while sending, never while waiting.
         *
         * @param iface  BlueZ manager interface name.
         * @param method Method to call on @p iface.
         * @param path   Object path argument to pass.
         * @param result Result slot: 0 pending, 1 accepted, -1 rejected.
         * @return true if BlueZ accepted the registration within 5 s.
         */
        bool callRegister(const char *iface,
                          const char *method,
                          const char *path,
                          std::atomic<int> &result)
        {
            result.store(0);
            {
                std::lock_guard<std::mutex> lock(busMutex_);
                if (sd_bus_call_method_async(bus_,
                                             nullptr,
                                             kBluezService,
                                             kAdapterPath,
                                             iface,
                                             method,
                                             &BlueZBleBackend::onRegisterReply,
                                             &result,
                                             "oa{sv}",
                                             path,
                                             0) < 0)
                {
                    return false;
                }
            }

            for (int i = 0; i < 100 && result.load() == 0; ++i)
            {
                SF_HAL::delay_ms(50);
            }
            return result.load() == 1;
        }

        /**
         * @brief Pump thread entry point: services the sd-bus connection until
         *        it is closed. Never returns, matching SF_HAL::ThreadFn.
         * @param param Raw @c BlueZBleBackend pointer.
         */
        static void pumpThreadEntry(void *param)
        {
            auto *self = static_cast<BlueZBleBackend *>(param);
            for (;;)
            {
                int r;
                {
                    std::lock_guard<std::mutex> lock(self->busMutex_);
                    r = sd_bus_process(self->bus_, nullptr);
                }
                if (r < 0)
                {
                    break;
                }
                if (r == 0)
                {
                    sd_bus_wait(self->bus_, static_cast<uint64_t>(-1));
                }
            }
        }

        /**
         * @brief GattService1.UUID property getter.
         */
        static int getServiceUuid(sd_bus *,
                                  const char *,
                                  const char *,
                                  const char *,
                                  sd_bus_message *reply,
                                  void *userdata,
                                  sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            return sd_bus_message_append(reply, "s", self->serviceUuid_.c_str());
        }

        /**
         * @brief GattService1.Primary property getter; smartfin only registers
         *        one, always-primary service.
         */
        static int getServicePrimary(sd_bus *,
                                     const char *,
                                     const char *,
                                     const char *,
                                     sd_bus_message *reply,
                                     void *,
                                     sd_bus_error *)
        {
            return sd_bus_message_append(reply, "b", 1);
        }

        /**
         * @brief GattCharacteristic1.UUID property getter for the telemetry
         *        characteristic.
         */
        static int getTelemetryUuid(sd_bus *,
                                    const char *,
                                    const char *,
                                    const char *,
                                    sd_bus_message *reply,
                                    void *userdata,
                                    sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            return sd_bus_message_append(reply, "s", self->telemetryUuid_.c_str());
        }

        /**
         * @brief GattCharacteristic1.UUID property getter for the control
         *        characteristic.
         */
        static int getControlUuid(sd_bus *,
                                  const char *,
                                  const char *,
                                  const char *,
                                  sd_bus_message *reply,
                                  void *userdata,
                                  sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            return sd_bus_message_append(reply, "s", self->controlUuid_.c_str());
        }

        /**
         * @brief GattCharacteristic1.Service property getter; both
         *        characteristics belong to the single registered service.
         */
        static int getCharService(sd_bus *,
                                  const char *,
                                  const char *,
                                  const char *,
                                  sd_bus_message *reply,
                                  void *,
                                  sd_bus_error *)
        {
            return sd_bus_message_append(reply, "o", kServicePath);
        }

        /**
         * @brief GattCharacteristic1.Flags property getter for the telemetry
         *        characteristic ("notify" only).
         */
        static int getTelemetryFlags(sd_bus *,
                                     const char *,
                                     const char *,
                                     const char *,
                                     sd_bus_message *reply,
                                     void *,
                                     sd_bus_error *)
        {
            int r = sd_bus_message_open_container(reply, 'a', "s");
            if (r < 0)
            {
                return r;
            }
            r = sd_bus_message_append(reply, "s", "notify");
            if (r < 0)
            {
                return r;
            }
            return sd_bus_message_close_container(reply);
        }

        /**
         * @brief GattCharacteristic1.Flags property getter for the control
         *        characteristic ("write-without-response" only).
         */
        static int getControlFlags(sd_bus *,
                                   const char *,
                                   const char *,
                                   const char *,
                                   sd_bus_message *reply,
                                   void *,
                                   sd_bus_error *)
        {
            int r = sd_bus_message_open_container(reply, 'a', "s");
            if (r < 0)
            {
                return r;
            }
            r = sd_bus_message_append(reply, "s", "write-without-response");
            if (r < 0)
            {
                return r;
            }
            return sd_bus_message_close_container(reply);
        }

        /**
         * @brief GattCharacteristic1.Value property getter for the telemetry
         *        characteristic; returns the last payload passed to
         *        notifyTelemetry().
         */
        static int getTelemetryValue(sd_bus *,
                                     const char *,
                                     const char *,
                                     const char *,
                                     sd_bus_message *reply,
                                     void *userdata,
                                     sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            std::lock_guard<std::mutex> lock(self->valueMutex_);
            return sd_bus_message_append_array(
                reply, 'y', self->telemetryValue_.data(), self->telemetryValue_.size());
        }

        /**
         * @brief GattCharacteristic1.StartNotify method handler.
         *
         * BlueZ does not surface a per-app "central connected" event at this
         * layer; a central subscribing to telemetry notifications is the
         * closest available proxy for "connected and interested", so it drives
         * @c on_connection() here rather than watching org.bluez.Device1.
         *
         * @param m Incoming method call message.
         * @param userdata Raw @c BlueZBleBackend pointer.
         * @return Result of replying to @p m.
         */
        static int handleStartNotify(sd_bus_message *m, void *userdata, sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            self->notifying_ = true;
            if (callbacks_.on_connection)
            {
                callbacks_.on_connection(true, callbacks_.context);
            }
            return sd_bus_reply_method_return(m, "");
        }

        /**
         * @brief GattCharacteristic1.StopNotify method handler; see
         *        handleStartNotify() for why this drives on_connection().
         *
         * @param m Incoming method call message.
         * @param userdata Raw @c BlueZBleBackend pointer.
         * @return Result of replying to @p m.
         */
        static int handleStopNotify(sd_bus_message *m, void *userdata, sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            self->notifying_ = false;
            if (callbacks_.on_connection)
            {
                callbacks_.on_connection(false, callbacks_.context);
            }
            return sd_bus_reply_method_return(m, "");
        }

        /**
         * @brief GattCharacteristic1.WriteValue method handler for the control
         *        characteristic; forwards the payload to the registered
         *        on_write callback with characteristic handle 0.
         *
         * @param m Incoming method call message; first argument is the byte
         *          array, second is an options dict this backend ignores.
         * @return Result of replying to @p m, or a negative sd-bus error code
         *         if the byte array could not be read.
         */
        static int handleWriteValue(sd_bus_message *m, void *, sd_bus_error *)
        {
            const void *data = nullptr;
            std::size_t len = 0;
            int r = sd_bus_message_read_array(m, 'y', &data, &len);
            if (r < 0)
            {
                return r;
            }
            if (callbacks_.on_write && data && len > 0)
            {
                callbacks_.on_write(0, static_cast<const uint8_t *>(data), len, callbacks_.context);
            }
            return sd_bus_reply_method_return(m, "");
        }

        /**
         * @brief LEAdvertisement1.Type property getter; smartfin only
         *        advertises as a connectable peripheral.
         */
        static int getAdvertisementType(sd_bus *,
                                        const char *,
                                        const char *,
                                        const char *,
                                        sd_bus_message *reply,
                                        void *,
                                        sd_bus_error *)
        {
            return sd_bus_message_append(reply, "s", "peripheral");
        }

        /**
         * @brief LEAdvertisement1.ServiceUUIDs property getter.
         */
        static int getAdvertisementServiceUuids(sd_bus *,
                                                const char *,
                                                const char *,
                                                const char *,
                                                sd_bus_message *reply,
                                                void *userdata,
                                                sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            int r = sd_bus_message_open_container(reply, 'a', "s");
            if (r < 0)
            {
                return r;
            }
            r = sd_bus_message_append(reply, "s", self->advertisedServiceUuid_.c_str());
            if (r < 0)
            {
                return r;
            }
            return sd_bus_message_close_container(reply);
        }

        /**
         * @brief LEAdvertisement1.LocalName property getter.
         */
        static int getAdvertisementLocalName(sd_bus *,
                                             const char *,
                                             const char *,
                                             const char *,
                                             sd_bus_message *reply,
                                             void *userdata,
                                             sd_bus_error *)
        {
            auto *self = static_cast<BlueZBleBackend *>(userdata);
            return sd_bus_message_append(reply, "s", self->localName_.c_str());
        }

        /**
         * @brief LEAdvertisement1.Release method handler; BlueZ calls this when
         *        it drops the advertisement, which this backend does not need
         *        to react to since UnregisterAdvertisement is always
         *        BlueZ-initiated through stopAdvertising().
         *
         * @param m Incoming method call message.
         * @return Result of replying to @p m.
         */
        static int handleRelease(sd_bus_message *m, void *, sd_bus_error *)
        {
            return sd_bus_reply_method_return(m, "");
        }

        sd_bus *bus_ = nullptr;
        sd_bus_slot *objectManagerSlot_ = nullptr;
        sd_bus_slot *serviceSlot_ = nullptr;
        sd_bus_slot *telemetryCharSlot_ = nullptr;
        sd_bus_slot *controlCharSlot_ = nullptr;
        sd_bus_slot *advSlot_ = nullptr;
        void *pumpThread_ = nullptr;

        bool initialized_ = false;
        bool notifying_ = false;

        std::string serviceUuid_;
        std::string telemetryUuid_;
        std::string controlUuid_;
        std::string advertisedServiceUuid_;
        std::string localName_;

        std::atomic<int> appRegResult_{0};
        std::atomic<int> advRegResult_{0};

        std::mutex busMutex_;
        std::mutex valueMutex_;
        std::vector<uint8_t> telemetryValue_;

        static const sd_bus_vtable serviceVtable_[];
        static const sd_bus_vtable telemetryCharVtable_[];
        static const sd_bus_vtable controlCharVtable_[];
        static const sd_bus_vtable advertisementVtable_[];
    };

    const sd_bus_vtable BlueZBleBackend::serviceVtable_[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_PROPERTY("UUID",
                        "s",
                        &BlueZBleBackend::getServiceUuid,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Primary",
                        "b",
                        &BlueZBleBackend::getServicePrimary,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_VTABLE_END};

    const sd_bus_vtable BlueZBleBackend::telemetryCharVtable_[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_PROPERTY("UUID",
                        "s",
                        &BlueZBleBackend::getTelemetryUuid,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Service",
                        "o",
                        &BlueZBleBackend::getCharService,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Flags",
                        "as",
                        &BlueZBleBackend::getTelemetryFlags,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Value", "ay", &BlueZBleBackend::getTelemetryValue, 0, 0),
        SD_BUS_METHOD("StartNotify",
                      "",
                      "",
                      &BlueZBleBackend::handleStartNotify,
                      SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_METHOD("StopNotify",
                      "",
                      "",
                      &BlueZBleBackend::handleStopNotify,
                      SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_VTABLE_END};

    const sd_bus_vtable BlueZBleBackend::controlCharVtable_[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_PROPERTY("UUID",
                        "s",
                        &BlueZBleBackend::getControlUuid,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Service",
                        "o",
                        &BlueZBleBackend::getCharService,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("Flags",
                        "as",
                        &BlueZBleBackend::getControlFlags,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_METHOD("WriteValue",
                      "aya{sv}",
                      "",
                      &BlueZBleBackend::handleWriteValue,
                      SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_VTABLE_END};

    const sd_bus_vtable BlueZBleBackend::advertisementVtable_[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_PROPERTY("Type",
                        "s",
                        &BlueZBleBackend::getAdvertisementType,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("ServiceUUIDs",
                        "as",
                        &BlueZBleBackend::getAdvertisementServiceUuids,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_PROPERTY("LocalName",
                        "s",
                        &BlueZBleBackend::getAdvertisementLocalName,
                        0,
                        SD_BUS_VTABLE_PROPERTY_CONST),
        SD_BUS_METHOD("Release",
                      "",
                      "",
                      &BlueZBleBackend::handleRelease,
                      SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_VTABLE_END};

} // namespace

namespace SF_HAL
{

    /**
     * @brief Register BLE event callbacks.
     *
     * Must be called before @c ble_init(); see hal.hpp for the ordering
     * contract. Not asserted here, unlike the Particle backend.
     */
    void ble_set_callbacks(const ble::Callbacks &callbacks)
    {
        callbacks_ = callbacks;
    }

    bool ble_init(const char *device_name,
                  const char *service_uuid,
                  const char *telemetry_char_uuid,
                  const char *control_char_uuid)
    {
        if (!device_name || !service_uuid || !telemetry_char_uuid || !control_char_uuid)
        {
            return false;
        }

        return BlueZBleBackend::getInstance().init(
            device_name, service_uuid, telemetry_char_uuid, control_char_uuid);
    }

    bool ble_advertise(const char *service_uuid, const char *local_name)
    {
        if (!service_uuid)
        {
            return false;
        }

        return BlueZBleBackend::getInstance().advertise(service_uuid, local_name);
    }

    void ble_stop_advertising()
    {
        BlueZBleBackend::getInstance().stopAdvertising();
    }

    bool ble_notify(const uint8_t *data, std::size_t len)
    {
        return BlueZBleBackend::getInstance().notifyTelemetry(data, len);
    }

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
