import argparse
import asyncio
import logging
from pathlib import Path
import struct
import time
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData
import sys

# ---------------------------------------------------------------------------
# Args
# ---------------------------------------------------------------------------
_args = argparse.ArgumentParser()
_args.add_argument("-v", "--verbose", action="store_true",
                   help="also print logs to terminal")
args = _args.parse_args()


def _prune_old_devtty_logs() -> None:
    """Keep only the newest logs/devtty*.txt file at startup."""
    log_dir = Path("logs")
    matches = [path for path in log_dir.glob("devtty*.txt") if path.is_file()]
    if len(matches) <= 1:
        return

    newest = max(matches, key=lambda path: (path.stat().st_mtime, path.name))
    for path in matches:
        if path == newest:
            continue
        path.unlink()


_prune_old_devtty_logs()

# ---------------------------------------------------------------------------
# Logging — flush after every record, timestamps as HH:MM:SS:mmm
# ---------------------------------------------------------------------------
_fmt = logging.Formatter(
    fmt="%(asctime)s:%(msecs)03d [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)


class _FlushFileHandler(logging.FileHandler):
    """FileHandler that flushes to disk after every record."""

    def emit(self, record: logging.LogRecord) -> None:
        super().emit(record)
        self.flush()


class _FlushStreamHandler(logging.StreamHandler):
    """StreamHandler that flushes after every record."""

    def emit(self, record: logging.LogRecord) -> None:
        super().emit(record)
        self.flush()


log = logging.getLogger("ble_receiver")
log.setLevel(logging.DEBUG)

_file = _FlushFileHandler("tests/ble_log.txt", mode="w")
_file.setFormatter(_fmt)
log.addHandler(_file)

if args.verbose:
    _console = _FlushStreamHandler()
    _console.setFormatter(_fmt)
    log.addHandler(_console)

# Also log Bleak debug info
bleak_log = logging.getLogger("bleak")
bleak_log.setLevel(logging.DEBUG)
bleak_fh = _FlushFileHandler("bleak_debug.txt", mode="w")
bleak_fh.setFormatter(_fmt)
bleak_log.addHandler(bleak_fh)
if args.verbose:
    bleak_sh = _FlushStreamHandler()
    bleak_sh.setFormatter(_fmt)
    bleak_log.addHandler(bleak_sh)

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------
DEVICE_NAME = "Smartfin"
TELEMETRY_UUID = "deeddb00-166e-407c-8158-7b9693ad2685"
CONTROL_UUID = "c39513e6-631e-439a-9b3b-affa0635b3d1"
SCAN_TIMEOUT = 30
RUN_DURATION = 60

# ---------------------------------------------------------------------------
# State
# ---------------------------------------------------------------------------
_notify_count = 0
_notify_errors = 0
_first_notify = None
_last_notify = None


def _hex(data: bytes | bytearray) -> str:
    return " ".join(f"{b:02X}" for b in data)


def _ascii(data: bytes | bytearray) -> str:
    return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in data)


# ---------------------------------------------------------------------------
# Notification handler
# ---------------------------------------------------------------------------
def on_notify(char: BleakGATTCharacteristic, data: bytearray) -> None:
    global _notify_count, _notify_errors, _first_notify, _last_notify
    try:
        now = time.monotonic()
        _notify_count += 1

        if _first_notify is None:
            _first_notify = now
        elapsed = now - _first_notify
        rate = _notify_count / elapsed if elapsed > 0 else 0.0

        dt_ms = (now - _last_notify) * \
            1000.0 if _last_notify is not None else 0.0
        _last_notify = now

        log.debug("--- notify #%d ---", _notify_count)
        log.debug("  handle  : 0x%04X  uuid=%s", char.handle, char.uuid)
        log.debug("  len     : %d bytes", len(data))
        log.debug("  hex     : %s", _hex(data))
        log.debug("  ascii   : %s", _ascii(data))
        log.debug("  dt      : %.2f ms since last", dt_ms)
        log.debug("  rate    : %.1f Hz over %.2f s", rate, elapsed)

        n_words = len(data) // 4
        if n_words > 0:
            try:
                words = struct.unpack_from(f"<{n_words}I", data)
                log.debug("  uint32s : %s", " ".join(
                    f"0x{w:08X}" for w in words))
            except struct.error as e:
                _notify_errors += 1
                log.warning("  parse error: %s", e)
        else:
            log.debug("  (too short to parse as uint32s)")

        leftover = len(data) % 4
        if leftover:
            log.debug("  leftover: %s (%d bytes)",
                      _hex(data[-leftover:]), leftover)

    except Exception:
        log.exception("on_notify: unhandled exception")


# ---------------------------------------------------------------------------
# Scan callback
# ---------------------------------------------------------------------------
def on_scan(device: BLEDevice, adv: AdvertisementData) -> None:
    try:
        log.debug("scan: addr=%s rssi=%s name=%r uuids=%s",
                  device.address, adv.rssi,
                  device.name or adv.local_name,
                  adv.service_uuids or [])
    except Exception:
        log.exception("on_scan: unhandled exception")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
async def main() -> None:
    log.info("=== ble_receiver starting ===")
    log.info("target         : %r", DEVICE_NAME)
    log.info("telemetry UUID : %s", TELEMETRY_UUID)
    log.info("control UUID   : %s", CONTROL_UUID)
    log.info("scan timeout   : %d s", SCAN_TIMEOUT)
    log.info("run duration   : %d s", RUN_DURATION)

    # Scan — use BleakScanner as context manager so detection_callback works
    log.info("starting scan (timeout=%ds)...", SCAN_TIMEOUT)
    t0 = time.monotonic()
    device: BLEDevice | None = None
    found_event = asyncio.Event()

    def on_scan_find(d: BLEDevice, adv: AdvertisementData) -> None:
        nonlocal device
        on_scan(d, adv)
        if (d.name == DEVICE_NAME or adv.local_name == DEVICE_NAME) and not found_event.is_set():
            log.info("target found: name=%r addr=%s rssi=%s",
                     d.name, d.address, adv.rssi)
            device = d
            found_event.set()

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
        log.error("device %r not found after %.1f s — giving up",
                  DEVICE_NAME, time.monotonic() - t0)
        return

    log.info("found: name=%r addr=%s rssi=%s",
             device.name, device.address, getattr(device, "rssi", "?"))

    # Connect
    log.info("connecting to %s...", device.address)
    t1 = time.monotonic()
    try:
        # macOS workaround: disable cached services and set winrt parameter
        log.debug(
            f"creating BleakClient with timeout={SCAN_TIMEOUT}s, use_cached=False")
        async with BleakClient(device, timeout=SCAN_TIMEOUT, use_cached=False) as client:
            log.info("connected in %.2f s  mtu=%s",
                     time.monotonic() - t1, getattr(client, "mtu_size", "?"))

            # Try to get MTU info
            try:
                mtu = client.mtu_size
                log.info("MTU size: %d", mtu)
            except Exception as e:
                log.warning("Could not get MTU: %s", e)

            # Dump GATT table
            log.debug("--- GATT table ---")
            for svc in client.services:
                log.debug("  service: %s  (%s)", svc.uuid, svc.description)
                for char in svc.characteristics:
                    log.debug("    char: %s  props=%s  handle=0x%04X  (%s)",
                              char.uuid, char.properties, char.handle, char.description)
                    for desc in char.descriptors:
                        log.debug("      desc: %s  handle=0x%04X",
                                  desc.uuid, desc.handle)
            log.debug("--- end GATT ---")

            tele_char = client.services.get_characteristic(TELEMETRY_UUID)
            if tele_char:
                log.info("telemetry char found: handle=0x%04X props=%s",
                         tele_char.handle, tele_char.properties)
            else:
                log.warning("telemetry char %s NOT in GATT table",
                            TELEMETRY_UUID)

            ctrl_char = client.services.get_characteristic(CONTROL_UUID)
            if ctrl_char:
                log.info("control char found: handle=0x%04X props=%s",
                         ctrl_char.handle, ctrl_char.properties)
            else:
                log.info("control char %s not found (optional)", CONTROL_UUID)

            # Subscribe
            log.info("subscribing to telemetry notifications...")
            try:
                await client.start_notify(TELEMETRY_UUID, on_notify)
            except Exception:
                log.exception("start_notify failed")
                return
            log.info("subscribed — running for %d s", RUN_DURATION)

            t_run = time.monotonic()
            try:
                while True:
                    elapsed = time.monotonic() - t_run
                    if elapsed >= RUN_DURATION:
                        break
                    log.debug("status: %.0f s remaining  notifies=%d errors=%d",
                              RUN_DURATION - elapsed, _notify_count, _notify_errors)
                    await asyncio.sleep(5)
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
            log.info("  notifies   : %d", _notify_count)
            log.info("  parse errs : %d", _notify_errors)
            log.info("  avg rate   : %.2f Hz", _notify_count /
                     run_time if run_time > 0 else 0)

    except Exception:
        log.exception("connection-level exception (connect/GATT/disconnect)")
        import traceback
        tb = traceback.format_exc()
        log.error("Full traceback:\n%s", tb)

    log.info("disconnected — done")


try:
    asyncio.run(main())
except KeyboardInterrupt:
    log.info("interrupted by user")
except Exception:
    log.exception("top-level exception in asyncio.run")
