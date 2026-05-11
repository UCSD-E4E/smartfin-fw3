from enum import IntEnum
from pathlib import Path

LOG_DIR = Path("tests/ble/logs")

DEVICE_NAME = "Smartfin"
TELEMETRY_UUID = "deeddb00-166e-407c-8158-7b9693ad2685"
CONTROL_UUID = "c39513e6-631e-439a-9b3b-affa0635b3d1"

SCAN_TIMEOUT = 30
RUN_DURATION = 60
TARGET_ENSEMBLES = 20000

TRANSPORT_HEADER_SIZE = 6
# EnsembleHeader_t has 4+28 = 32 bits of bitfields in unsigned int base type.
# With #pragma pack(push,1) on ARM GCC, bitfields still occupy their base type's
# storage unit, so sizeof(EnsembleHeader_t) = 4 bytes.
ENSEMBLE_HEADER_SIZE = 4
ENS_TEMP = 0x01
ENS_TEMP_HIGH_DATA_RATE_IMU = 0x0C
ENS_TEXT = 0x0F

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
