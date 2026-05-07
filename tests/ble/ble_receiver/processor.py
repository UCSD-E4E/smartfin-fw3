import asyncio
import struct
import logging
from .config import (
    TRANSPORT_HEADER_SIZE, ENSEMBLE_HEADER_SIZE,
    ENS_TEMP, ENS_TEMP_HIGH_DATA_RATE_IMU, SensorType,
)
from .state import BLEState


def hex_str(data: bytes | bytearray) -> str:
    return " ".join(f"{b:02X}" for b in data)


def ascii_str(data: bytes | bytearray) -> str:
    return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in data)


def decode_ensemble_header(data: bytes | bytearray, offset: int) -> tuple[int, int]:
    header_word = int.from_bytes(data[offset:offset + ENSEMBLE_HEADER_SIZE], "little")
    ensemble_type = header_word & 0x0F
    elapsed_time_ms = (header_word >> 4) & 0xFFFFFFF
    return ensemble_type, elapsed_time_ms


def count_ensembles(data: bytes | bytearray) -> int:
    if len(data) < TRANSPORT_HEADER_SIZE:
        return 0
    try:
        _version, _ptype, _seq, payload_len = struct.unpack_from("<BBHH", data, 0)
    except struct.error:
        return 0

    payload_end = min(len(data), TRANSPORT_HEADER_SIZE + payload_len)
    offset = TRANSPORT_HEADER_SIZE
    count = 0

    while offset + ENSEMBLE_HEADER_SIZE <= payload_end:
        header_word = int.from_bytes(data[offset:offset + ENSEMBLE_HEADER_SIZE], "little")
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


def decode_sensor_ensembles(data: bytes | bytearray, state: BLEState) -> None:
    if len(data) < TRANSPORT_HEADER_SIZE:
        return
    try:
        _version, _ptype, _seq, payload_len = struct.unpack_from("<BBHH", data, 0)
    except struct.error:
        return

    payload_end = min(len(data), TRANSPORT_HEADER_SIZE + payload_len)
    offset = TRANSPORT_HEADER_SIZE

    while offset + ENSEMBLE_HEADER_SIZE <= payload_end:
        ensemble_type, elapsed_time_ms = decode_ensemble_header(data, offset)

        if ensemble_type == ENS_TEMP:
            record_size = ENSEMBLE_HEADER_SIZE + 3
            if offset + record_size > payload_end:
                break
            scaled_temp, _water = struct.unpack_from("<hB", data, offset + ENSEMBLE_HEADER_SIZE)
            temp_c = scaled_temp / 128.0
            state.temp_time_ms.append(elapsed_time_ms)
            state.temp_c.append(temp_c)
            state.records.append([elapsed_time_ms, SensorType.TEMPERATURE, ensemble_type, temp_c])
            offset += record_size
            continue

        if ensemble_type == ENS_TEMP_HIGH_DATA_RATE_IMU:
            # Ensemble12_data_t: 9 × int16 = 18 bytes.
            # Header is 3 bytes (packed bitfield), so total = 21 bytes.
            record_size = ENSEMBLE_HEADER_SIZE + 18
            if offset + record_size > payload_end:
                break
            ax, ay, az, gx, gy, gz, mx, my, mz = struct.unpack_from(
                "<9h", data, offset + ENSEMBLE_HEADER_SIZE)
            # Scalings from Ensemble12_data_t docstring:
            #   accel: Q14  -> / 16384.0  (m/s²)
            #   gyro:  Q7   -> / 128.0    (deg/s)
            #   mag:   Q3   -> / 8.0      (µT)
            state.imu_time_ms.append(elapsed_time_ms)
            state.accel_x.append(ax / 16384.0)
            state.accel_y.append(ay / 16384.0)
            state.accel_z.append(az / 16384.0)
            state.gyro_x.append(gx / 128.0)
            state.gyro_y.append(gy / 128.0)
            state.gyro_z.append(gz / 128.0)
            state.mag_x.append(mx / 8.0)
            state.mag_y.append(my / 8.0)
            state.mag_z.append(mz / 8.0)
            t = elapsed_time_ms
            e = ensemble_type
            state.records.extend([
                [t, SensorType.ACCEL_X, e, ax / 16384.0],
                [t, SensorType.ACCEL_Y, e, ay / 16384.0],
                [t, SensorType.ACCEL_Z, e, az / 16384.0],
                [t, SensorType.GYRO_X,  e, gx / 128.0],
                [t, SensorType.GYRO_Y,  e, gy / 128.0],
                [t, SensorType.GYRO_Z,  e, gz / 128.0],
                [t, SensorType.MAG_X,   e, mx / 8.0],
                [t, SensorType.MAG_Y,   e, my / 8.0],
                [t, SensorType.MAG_Z,   e, mz / 8.0],
            ])
            offset += record_size
            continue

        break


async def packet_processor(
    queue: asyncio.Queue,
    state: BLEState,
    log: logging.Logger,
) -> None:
    """Drains raw BLE packets from the queue and decodes sensor ensembles.

    Runs concurrently with BLE reception; put None in the queue to stop.
    """
    while True:
        data = await queue.get()
        if data is None:
            queue.task_done()
            break
        try:
            decode_sensor_ensembles(data, state)
        except Exception:
            log.exception("packet_processor: error decoding packet")
        queue.task_done()
