import argparse
import asyncio
import logging
from pathlib import Path
import struct
import time
from collections import deque
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData
import sys

try:
    import matplotlib.pyplot as plt
except Exception:
    plt = None

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
TARGET_ENSEMBLES = 300
TRANSPORT_HEADER_SIZE = 6
ENSEMBLE_HEADER_SIZE = 4
ENS_TEMP = 0x01
ENS_TEMP_HIGH_DATA_RATE_IMU = 0x0C

# ---------------------------------------------------------------------------
# State
# ---------------------------------------------------------------------------
_notify_count = 0
_ensemble_count = 0
_notify_errors = 0
_first_notify = None
_last_notify = None
_notify_index = deque(maxlen=12000)
_notify_dt_ms = deque(maxlen=12000)
_notify_ensemble_batch = deque(maxlen=12000)
_notify_payload_bytes = deque(maxlen=12000)
_notify_elapsed_s = deque(maxlen=12000)
_cumulative_ensembles = deque(maxlen=12000)
_temp_time_ds = deque(maxlen=12000)
_temp_c = deque(maxlen=12000)
_imu_time_ds = deque(maxlen=12000)
_accel_x = deque(maxlen=12000)
_accel_y = deque(maxlen=12000)
_accel_z = deque(maxlen=12000)
_gyro_x = deque(maxlen=12000)
_gyro_y = deque(maxlen=12000)
_gyro_z = deque(maxlen=12000)
_mag_x = deque(maxlen=12000)
_mag_y = deque(maxlen=12000)
_mag_z = deque(maxlen=12000)


def _hex(data: bytes | bytearray) -> str:
    return " ".join(f"{b:02X}" for b in data)


def _ascii(data: bytes | bytearray) -> str:
    return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in data)


def _decode_ensemble_header(data: bytes | bytearray, offset: int) -> tuple[int, int]:
    header_word = int.from_bytes(
        data[offset:offset + ENSEMBLE_HEADER_SIZE], "little")
    ensemble_type = header_word & 0x0F
    elapsed_time_ds = (header_word >> 4) & 0xFFFFF
    return ensemble_type, elapsed_time_ds


def _count_ensembles(data: bytes | bytearray) -> int:
    """Count known ensembles inside one BLE transport packet."""
    if len(data) < TRANSPORT_HEADER_SIZE:
        return 0

    try:
        _version, _ptype, _seq, payload_len = struct.unpack_from(
            "<BBHH", data, 0)
    except struct.error:
        return 0

    payload_end = min(len(data), TRANSPORT_HEADER_SIZE + payload_len)
    offset = TRANSPORT_HEADER_SIZE
    count = 0

    while offset + ENSEMBLE_HEADER_SIZE <= payload_end:
        header_word = int.from_bytes(
            data[offset:offset + ENSEMBLE_HEADER_SIZE], "little")
        ensemble_type = header_word & 0x0F

        if ensemble_type == ENS_TEMP:
            record_size = ENSEMBLE_HEADER_SIZE + 3
        elif ensemble_type == ENS_TEMP_HIGH_DATA_RATE_IMU:
            record_size = ENSEMBLE_HEADER_SIZE + 18
        else:
            break

        if offset + record_size > payload_end:
            break

        count += 1
        offset += record_size

    return count


def _decode_sensor_ensembles(data: bytes | bytearray) -> None:
    if len(data) < TRANSPORT_HEADER_SIZE:
        return

    try:
        _version, _ptype, _seq, payload_len = struct.unpack_from(
            "<BBHH", data, 0)
    except struct.error:
        return

    payload_end = min(len(data), TRANSPORT_HEADER_SIZE + payload_len)
    offset = TRANSPORT_HEADER_SIZE

    while offset + ENSEMBLE_HEADER_SIZE <= payload_end:
        ensemble_type, elapsed_time_ds = _decode_ensemble_header(data, offset)

        if ensemble_type == ENS_TEMP:
            record_size = ENSEMBLE_HEADER_SIZE + 3
            if offset + record_size > payload_end:
                break
            scaled_temp, _water = struct.unpack_from(
                "<hB", data, offset + ENSEMBLE_HEADER_SIZE)
            _temp_time_ds.append(elapsed_time_ds)
            _temp_c.append(scaled_temp / 128.0)
            offset += record_size
            continue

        if ensemble_type == ENS_TEMP_HIGH_DATA_RATE_IMU:
            record_size = ENSEMBLE_HEADER_SIZE + 18
            if offset + record_size > payload_end:
                break
            ax, ay, az, gx, gy, gz, mx, my, mz = struct.unpack_from(
                "<9h", data, offset + ENSEMBLE_HEADER_SIZE)
            _imu_time_ds.append(elapsed_time_ds)
            _accel_x.append(ax / 1024.0)
            _accel_y.append(ay / 1024.0)
            _accel_z.append(az / 1024.0)
            _gyro_x.append(gx / 128.0)
            _gyro_y.append(gy / 128.0)
            _gyro_z.append(gz / 128.0)
            _mag_x.append(mx / 8.0)
            _mag_y.append(my / 8.0)
            _mag_z.append(mz / 8.0)
            offset += record_size
            continue

        break


def _plot_collected_ensembles() -> None:
    if plt is None:
        log.warning("matplotlib unavailable; skipping plot generation")
        return
    if not _temp_time_ds and not _imu_time_ds and not _notify_index:
        log.warning("no decodable data collected; skipping plot generation")
        return

    if _temp_time_ds or _imu_time_ds:
        fig_sensor, axes = plt.subplots(2, 2, figsize=(12, 7))
        fig_sensor.canvas.manager.set_window_title(
            "Smartfin BLE Sensor Summary")
        ax_temp = axes[0][0]
        ax_acc = axes[0][1]
        ax_gyro = axes[1][0]
        ax_mag = axes[1][1]

        if _temp_time_ds:
            ax_temp.plot(list(_temp_time_ds), list(
                _temp_c), lw=1.8, color="#CA472F")
        ax_temp.set_title("Temperature")
        ax_temp.set_xlabel("elapsed time (ds)")
        ax_temp.set_ylabel("deg C")

        if _imu_time_ds:
            x = list(_imu_time_ds)
            ax_acc.plot(x, list(_accel_x), lw=1.2, color="#0B84A5", label="ax")
            ax_acc.plot(x, list(_accel_y), lw=1.2, color="#F6C85F", label="ay")
            ax_acc.plot(x, list(_accel_z), lw=1.2, color="#6F4E7C", label="az")
            ax_gyro.plot(x, list(_gyro_x), lw=1.2, color="#9DD866", label="gx")
            ax_gyro.plot(x, list(_gyro_y), lw=1.2, color="#FFA600", label="gy")
            ax_gyro.plot(x, list(_gyro_z), lw=1.2, color="#BC5090", label="gz")
            ax_mag.plot(x, list(_mag_x), lw=1.2, color="#003F5C", label="mx")
            ax_mag.plot(x, list(_mag_y), lw=1.2, color="#58508D", label="my")
            ax_mag.plot(x, list(_mag_z), lw=1.2, color="#FF6361", label="mz")
            ax_acc.legend(loc="upper right")
            ax_gyro.legend(loc="upper right")
            ax_mag.legend(loc="upper right")

        ax_acc.set_title("Acceleration")
        ax_acc.set_xlabel("elapsed time (ds)")
        ax_acc.set_ylabel("m/s^2")
        ax_gyro.set_title("Gyroscope")
        ax_gyro.set_xlabel("elapsed time (ds)")
        ax_gyro.set_ylabel("deg/s")
        ax_mag.set_title("Magnetometer")
        ax_mag.set_xlabel("elapsed time (ds)")
        ax_mag.set_ylabel("uT")

        fig_sensor.suptitle(
            f"Smartfin BLE sensor summary  ensembles={_ensemble_count}  notifies={_notify_count}"
        )
        fig_sensor.tight_layout()

    if _notify_index:
        fig_ble, axes = plt.subplots(2, 2, figsize=(12, 7))
        fig_ble.canvas.manager.set_window_title(
            "Smartfin BLE Transport Analysis")
        ax_dt = axes[0][0]
        ax_batch = axes[0][1]
        ax_throughput = axes[1][0]
        ax_payload = axes[1][1]

        n = list(_notify_index)
        elapsed_s = list(_notify_elapsed_s)
        dt_ms = list(_notify_dt_ms)
        batch = list(_notify_ensemble_batch)
        payload = list(_notify_payload_bytes)
        cumulative = list(_cumulative_ensembles)

        ax_dt.plot(n, dt_ms, lw=1.2, color="#2F4B7C")
        if dt_ms:
            mean_dt = sum(dt_ms) / len(dt_ms)
            ax_dt.axhline(mean_dt, color="#D45087", lw=1.0,
                          linestyle="--", label=f"mean {mean_dt:.2f} ms")
            ax_dt.legend(loc="upper right")
        ax_dt.set_title("Notify Interval Jitter")
        ax_dt.set_xlabel("notification index")
        ax_dt.set_ylabel("ms")

        ax_batch.plot(n, batch, lw=1.2, color="#FFA600")
        ax_batch.set_title("Ensembles Per Notify")
        ax_batch.set_xlabel("notification index")
        ax_batch.set_ylabel("ensembles")

        throughput = []
        for t, c in zip(elapsed_s, cumulative):
            throughput.append(c / t if t > 0 else 0.0)
        ax_throughput.plot(elapsed_s, throughput, lw=1.4,
                           color="#00A6ED", label="avg ensembles/s")
        ax_throughput.plot(elapsed_s, cumulative, lw=1.0,
                           color="#7A5195", label="cumulative ensembles")
        ax_throughput.set_title("Throughput / Progress")
        ax_throughput.set_xlabel("elapsed time (s)")
        ax_throughput.set_ylabel("ensembles")
        ax_throughput.legend(loc="upper left")

        ax_payload.plot(n, payload, lw=1.2, color="#3CAEA3",
                        label="payload bytes")
        if batch:
            efficiency = [p / b if b > 0 else 0 for p,
                          b in zip(payload, batch)]
            ax_payload.plot(n, efficiency, lw=1.0,
                            color="#ED553B", label="bytes per ensemble")
        ax_payload.set_title("Batching Efficiency")
        ax_payload.set_xlabel("notification index")
        ax_payload.set_ylabel("bytes")
        ax_payload.legend(loc="upper right")

        fig_ble.suptitle(
            f"Smartfin BLE transport analysis  ensembles={_ensemble_count}  notifies={_notify_count}"
        )
        fig_ble.tight_layout()

    plt.show()


# ---------------------------------------------------------------------------
# Notification handler
# ---------------------------------------------------------------------------
def on_notify(char: BleakGATTCharacteristic, data: bytearray) -> None:
    global _notify_count, _ensemble_count, _notify_errors, _first_notify, _last_notify
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

        ensembles_in_packet = _count_ensembles(data)
        _ensemble_count += ensembles_in_packet
        _decode_sensor_ensembles(data)
        _notify_index.append(_notify_count)
        _notify_dt_ms.append(dt_ms)
        _notify_ensemble_batch.append(ensembles_in_packet)
        _notify_payload_bytes.append(len(data))
        _notify_elapsed_s.append(elapsed)
        _cumulative_ensembles.append(_ensemble_count)

        log.debug("--- notify #%d ---", _notify_count)
        log.debug("  handle  : 0x%04X  uuid=%s", char.handle, char.uuid)
        log.debug("  len     : %d bytes", len(data))
        log.debug("  hex     : %s", _hex(data))
        log.debug("  ascii   : %s", _ascii(data))
        log.debug("  dt      : %.2f ms since last", dt_ms)
        log.debug("  rate    : %.1f Hz over %.2f s", rate, elapsed)
        log.debug("  ensembles total: %d (+%d)",
                  _ensemble_count, ensembles_in_packet)

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
                    if _ensemble_count >= TARGET_ENSEMBLES:
                        log.info("target reached: ensembles=%d >= %d",
                                 _ensemble_count, TARGET_ENSEMBLES)
                        break
                    end_c = "\r" if TARGET_ENSEMBLES > _ensemble_count else "\n"
                    print(f"Ensembles recieved: {_ensemble_count}", end=end_c)
                    log.debug("status: %.0f s remaining  notifies=%d ensembles=%d errors=%d",
                              RUN_DURATION - elapsed, _notify_count, _ensemble_count, _notify_errors)
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
            log.info("  notifies   : %d", _notify_count)
            log.info("  ensembles  : %d", _ensemble_count)
            log.info("  parse errs : %d", _notify_errors)
            log.info("  avg rate   : %.2f Hz", _notify_count /
                     run_time if run_time > 0 else 0)
            _plot_collected_ensembles()

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
