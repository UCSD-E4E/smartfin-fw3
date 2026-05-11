from collections import deque
from dataclasses import dataclass, field
import numpy as np
from .config import DEQUE_MAXLEN


def _dq() -> deque:
    return deque(maxlen=DEQUE_MAXLEN)


@dataclass
class BLEState:
    # BLE transport stats
    notify_count: int = 0
    ensemble_count: int = 0
    notify_errors: int = 0
    first_notify: float | None = None
    last_notify: float | None = None

    notify_index: deque = field(default_factory=_dq)
    notify_dt_ms: deque = field(default_factory=_dq)
    notify_ensemble_batch: deque = field(default_factory=_dq)
    notify_payload_bytes: deque = field(default_factory=_dq)
    notify_elapsed_s: deque = field(default_factory=_dq)
    cumulative_ensembles: deque = field(default_factory=_dq)

    # Sensor data
    temp_time_ms: deque = field(default_factory=_dq)
    temp_c: deque = field(default_factory=_dq)

    imu_time_ms: deque = field(default_factory=_dq)
    accel_x: deque = field(default_factory=_dq)
    accel_y: deque = field(default_factory=_dq)
    accel_z: deque = field(default_factory=_dq)
    gyro_x: deque = field(default_factory=_dq)
    gyro_y: deque = field(default_factory=_dq)
    gyro_z: deque = field(default_factory=_dq)
    mag_x: deque = field(default_factory=_dq)
    mag_y: deque = field(default_factory=_dq)
    mag_z: deque = field(default_factory=_dq)

    # Firmware version string received via ENS_TEXT ensemble.
    fw_version: str = ""

    # Flat record log: one row per scalar reading.
    # Columns: [time_ms, sensor_type (SensorType), ensemble_type, value]
    records: list = field(default_factory=list)

    def as_numpy(self) -> np.ndarray:
        """Return records as a float64 array of shape (N, 4).

        Columns: time_ms | sensor_type | ensemble_type | value
        """
        if not self.records:
            return np.empty((0, 4), dtype=np.float64)
        return np.array(self.records, dtype=np.float64)
