import asyncio
import logging
import struct
import time
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

from .config import (
    DEVICE_NAME, TELEMETRY_UUID, CONTROL_UUID,
    SCAN_TIMEOUT, RUN_DURATION, TARGET_ENSEMBLES,
)
from .processor import count_ensembles, hex_str, ascii_str
from .state import BLEState


def make_notify_handler(
    state: BLEState,
    queue: asyncio.Queue,
    log: logging.Logger,
):
    """Returns an on_notify callback that records BLE transport stats and
    enqueues raw packets for async sensor decoding."""

    def on_notify(char: BleakGATTCharacteristic, data: bytearray) -> None:
        try:
            now = time.monotonic()
            state.notify_count += 1

            if state.first_notify is None:
                state.first_notify = now
            elapsed = now - state.first_notify
            rate = state.notify_count / elapsed if elapsed > 0 else 0.0

            dt_ms = (now - state.last_notify) * 1000.0 if state.last_notify is not None else 0.0
            state.last_notify = now

            ensembles_in_packet = count_ensembles(data)
            state.ensemble_count += ensembles_in_packet

            state.notify_index.append(state.notify_count)
            state.notify_dt_ms.append(dt_ms)
            state.notify_ensemble_batch.append(ensembles_in_packet)
            state.notify_payload_bytes.append(len(data))
            state.notify_elapsed_s.append(elapsed)
            state.cumulative_ensembles.append(state.ensemble_count)

            # Enqueue raw bytes for async sensor decoding - never blocks BLE
            queue.put_nowait(bytes(data))

            log.debug("--- notify #%d ---", state.notify_count)
            log.debug("  handle  : 0x%04X  uuid=%s", char.handle, char.uuid)
            log.debug("  len     : %d bytes", len(data))
            log.debug("  hex     : %s", hex_str(data))
            log.debug("  ascii   : %s", ascii_str(data))
            log.debug("  dt      : %.2f ms since last", dt_ms)
            log.debug("  rate    : %.1f Hz over %.2f s", rate, elapsed)
            log.debug("  ensembles total: %d (+%d)", state.ensemble_count, ensembles_in_packet)

            n_words = len(data) // 4
            if n_words > 0:
                try:
                    words = struct.unpack_from(f"<{n_words}I", data)
                    log.debug("  uint32s : %s", " ".join(f"0x{w:08X}" for w in words))
                except struct.error as e:
                    state.notify_errors += 1
                    log.warning("  parse error: %s", e)
            else:
                log.debug("  (too short to parse as uint32s)")

            leftover = len(data) % 4
            if leftover:
                log.debug("  leftover: %s (%d bytes)", hex_str(data[-leftover:]), leftover)

        except Exception:
            log.exception("on_notify: unhandled exception")

    return on_notify


def make_scan_callback(log: logging.Logger):
    def on_scan(device: BLEDevice, adv: AdvertisementData) -> None:
        try:
            log.debug("scan: addr=%s rssi=%s name=%r uuids=%s",
                      device.address, adv.rssi,
                      device.name or adv.local_name,
                      adv.service_uuids or [])
        except Exception:
            log.exception("on_scan: unhandled exception")
    return on_scan


async def run_ble(
    state: BLEState,
    queue: asyncio.Queue,
    log: logging.Logger,
) -> None:
    log.info("=== ble_receiver starting ===")
    log.info("target         : %r", DEVICE_NAME)
    log.info("telemetry UUID : %s", TELEMETRY_UUID)
    log.info("control UUID   : %s", CONTROL_UUID)
    log.info("scan timeout   : %d s", SCAN_TIMEOUT)
    log.info("run duration   : %d s", RUN_DURATION)

    t0 = time.monotonic()
    device: BLEDevice | None = None
    found_event = asyncio.Event()
    on_scan = make_scan_callback(log)

    def on_scan_find(d: BLEDevice, adv: AdvertisementData) -> None:
        nonlocal device
        on_scan(d, adv)
        if (d.name == DEVICE_NAME or adv.local_name == DEVICE_NAME) and not found_event.is_set():
            log.info("target found: name=%r addr=%s rssi=%s", d.name, d.address, adv.rssi)
            device = d
            found_event.set()

    log.info("starting scan (timeout=%ds)...", SCAN_TIMEOUT)
    try:
        async with BleakScanner(detection_callback=on_scan_find):
            try:
                await asyncio.wait_for(found_event.wait(), timeout=SCAN_TIMEOUT)
            except asyncio.TimeoutError:
                pass
    except Exception:
        log.exception("scan failed with exception")
        return

    log.info("scan done in %.2f s", time.monotonic() - t0)

    if not device:
        log.error("device %r not found after %.1f s - giving up",
                  DEVICE_NAME, time.monotonic() - t0)
        return

    log.info("found: name=%r addr=%s rssi=%s",
             device.name, device.address, getattr(device, "rssi", "?"))

    log.info("connecting to %s...", device.address)
    t1 = time.monotonic()
    try:
        async with BleakClient(device, timeout=SCAN_TIMEOUT, use_cached=False) as client:
            log.info("connected in %.2f s  mtu=%s",
                     time.monotonic() - t1, getattr(client, "mtu_size", "?"))

            try:
                log.info("MTU size: %d", client.mtu_size)
            except Exception as e:
                log.warning("Could not get MTU: %s", e)

            log.debug("--- GATT table ---")
            for svc in client.services:
                log.debug("  service: %s  (%s)", svc.uuid, svc.description)
                for char in svc.characteristics:
                    log.debug("    char: %s  props=%s  handle=0x%04X  (%s)",
                              char.uuid, char.properties, char.handle, char.description)
                    for desc in char.descriptors:
                        log.debug("      desc: %s  handle=0x%04X", desc.uuid, desc.handle)
            log.debug("--- end GATT ---")

            tele_char = client.services.get_characteristic(TELEMETRY_UUID)
            if tele_char:
                log.info("telemetry char found: handle=0x%04X props=%s",
                         tele_char.handle, tele_char.properties)
            else:
                log.warning("telemetry char %s NOT in GATT table", TELEMETRY_UUID)

            ctrl_char = client.services.get_characteristic(CONTROL_UUID)
            if ctrl_char:
                log.info("control char found: handle=0x%04X props=%s",
                         ctrl_char.handle, ctrl_char.properties)
            else:
                log.info("control char %s not found (optional)", CONTROL_UUID)

            on_notify = make_notify_handler(state, queue, log)
            log.info("subscribing to telemetry notifications...")
            try:
                await client.start_notify(TELEMETRY_UUID, on_notify)
            except Exception:
                log.exception("start_notify failed")
                return
            log.info("subscribed - running for %d s", RUN_DURATION)

            t_run = time.monotonic()
            try:
                while True:
                    elapsed = time.monotonic() - t_run
                    if elapsed >= RUN_DURATION:
                        break
                    if state.ensemble_count >= TARGET_ENSEMBLES:
                        log.info("target reached: ensembles=%d >= %d",
                                 state.ensemble_count, TARGET_ENSEMBLES)
                        break
                    end_c = "\r" if TARGET_ENSEMBLES > state.ensemble_count else "\n"
                    print(f"Ensembles received: {state.ensemble_count}", end=end_c)
                    log.debug("status: %.0f s remaining  notifies=%d ensembles=%d errors=%d",
                              RUN_DURATION - elapsed, state.notify_count,
                              state.ensemble_count, state.notify_errors)
                    await asyncio.sleep(0.25)
            except asyncio.CancelledError:
                log.warning("run loop cancelled")
                raise
            except Exception:
                log.exception("run loop exception")

            log.info("stopping notifications...")
            try:
                await client.stop_notify(TELEMETRY_UUID)
            except Exception:
                log.exception("stop_notify failed")
            log.info("unsubscribed")

            run_time = time.monotonic() - t_run
            log.info("=== session summary ===")
            log.info("  run time   : %.2f s", run_time)
            log.info("  notifies   : %d", state.notify_count)
            log.info("  ensembles  : %d", state.ensemble_count)
            log.info("  parse errs : %d", state.notify_errors)
            log.info("  avg rate   : %.2f Hz",
                     state.notify_count / run_time if run_time > 0 else 0)

    except Exception:
        log.exception("connection-level exception (connect/GATT/disconnect)")
        import traceback
        log.error("Full traceback:\n%s", traceback.format_exc())

    log.info("disconnected - done")
