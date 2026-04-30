from enum import IntEnum
from pathlib import Path

LOG_DIR = Path("tests/ble/logs")

DEVICE_NAME = "Smartfin"
TELEMETRY_UUID = "deeddb00-166e-407c-8158-7b9693ad2685"
CONTROL_UUID = "c39513e6-631e-439a-9b3b-affa0635b3d1"

SCAN_TIMEOUT = 30
RUN_DURATION = 60
TARGET_ENSEMBLES = 300

TRANSPORT_HEADER_SIZE = 6
# EnsembleHeader_t has 4+20 = 24 bits of bitfields. With #pragma pack(push,1)
# on ARM GCC the struct compresses to ceil(24/8) = 3 bytes, not sizeof(unsigned int) = 4.
ENSEMBLE_HEADER_SIZE = 3
ENS_TEMP = 0x01
ENS_TEMP_HIGH_DATA_RATE_IMU = 0x0C

DEQUE_MAXLEN = 12000


class SensorType(IntEnum):
    TEMPERATURE = 0
    ACCEL_X     = 1
    ACCEL_Y     = 2
    ACCEL_Z     = 3
    GYRO_X      = 4
    GYRO_Y      = 5
    GYRO_Z      = 6
    MAG_X       = 7
    MAG_Y       = 8
    MAG_Z       = 9
