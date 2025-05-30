/**
 * @file newIMU.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief New IMU Implementation
 * @version 0.1
 * @date 2025-05-28
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "newIMU.hpp"

#include "cli/flog.hpp"
#include "consts.hpp"
#include "product.hpp"

#include <cmath>
#include <cstring>
#include <functional>

/**
 * @brief Quaterinion Scale Factor
 *
 * 2^30
 *
 */
#define Q_SCALE 1073741824.0f

bool IMU::begin(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ICM_20948_Status_e status;
    bool success = true;

    if (NULL == _data_mtx)
    {
        _data_mtx = new Mutex();
    }
    _data_mtx->lock();
    memset(&fifo_data, 0, sizeof(struct FiFoData));
    _data_mtx->unlock();

    if (NULL == _device_mtx)
    {
        _device_mtx = new Mutex();
    }
    _device_mtx->lock();

    i2c_port.begin();
    i2c_port.setClock(400000);

    status = _device.begin(i2c_port, AD0_VAL);

    if (status != ICM_20948_Stat_Ok)
    {
        FLOG_AddError(FLOG_ICM_FAIL, (status << 8) | 0x0001);
        _device_mtx->unlock();
        return true;
    }

    status = _device.initializeDMP();
    if (status != ICM_20948_Stat_Ok)
    {
        FLOG_AddError(FLOG_ICM_DMP_INIT_FAIL, (status << 8) | 0x0002);
        _device_mtx->unlock();
        return true;
    }
    success &= (status == ICM_20948_Stat_Ok);
    // Enable the DMP sensors you want
    // Configuring DMP to output data at multiple ODRs:
    // DMP is capable of outputting multiple sensor data at different rates to FIFO.
    // Setting value can be calculated as follows:
    // Value = (DMP running rate / ODR ) - 1
    // E.g. For a 5Hz ODR rate when DMP is running at 55Hz, value = (55/5) - 1 = 10.
    success &= (_device.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
    success &=
        (_device.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER) == ICM_20948_Stat_Ok);
    success &= (_device.enableDMPSensor(INV_ICM20948_SENSOR_ACCELEROMETER) == ICM_20948_Stat_Ok);
    success &= (_device.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);
    success &= (_device.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok);
    // Reset DMP
    success &= (_device.resetDMP() == ICM_20948_Stat_Ok);

    // Reset FIFO
    success &= (_device.resetFIFO() == ICM_20948_Stat_Ok);

    // Enable the FIFO
    success &= (_device.enableFIFO() == ICM_20948_Stat_Ok);

    // Enable the DMP
    success &= (_device.enableDMP() == ICM_20948_Stat_Ok);
    _device_mtx->unlock();

    if (success == false)
    {
        FLOG_AddError(FLOG_ICM_FAIL, (_device.status << 8) | 0x0003);
        return true;
    }
    if (!_readLoop)
    {
        _readLoop = new Thread("ICM_read",
                               readLoop,
                               this,
                               OS_THREAD_PRIORITY_DEFAULT,
                               OS_THREAD_STACK_SIZE_DEFAULT_HIGH);
    }
#endif
    return false;
}

void IMU::readLoop(void *args)
{
    IMU *self = (IMU *)args;
    icm_20948_DMP_data_t dmpData;
    ICM_20948_Status_e status;
    bool has_data;

    while (1)
    {
        self->_device_mtx->lock();
        status = self->_device.readDMPdataFromFIFO(&dmpData);
        self->_device_mtx->unlock();
        has_data = false;
        switch (status)
        {
        case ICM_20948_Stat_Ok:
        case ICM_20948_Stat_FIFOMoreDataAvail:
            has_data = true;
            break;
        default:
            // FLOG_AddError(FLOG_ICM_FAIL, status);
            // All other fall through
        case ICM_20948_Stat_FIFONoDataAvail:
        case ICM_20948_Stat_FIFOIncompleteData:
            continue;
        }
        if (!has_data)
        {
            continue;
        }

        self->_data_mtx->lock();
        if (dmpData.header & DMP_header_bitmap_Accel)
        {
            self->fifo_data.RawAccel_X = dmpData.Raw_Accel.Data.X;
            self->fifo_data.RawAccel_Y = dmpData.Raw_Accel.Data.Y;
            self->fifo_data.RawAccel_Z = dmpData.Raw_Accel.Data.Z;
        }
        if (dmpData.header & DMP_header_bitmap_Gyro)
        {
            self->fifo_data.RawGyro_X = dmpData.Raw_Gyro.Data.X;
            self->fifo_data.RawGyro_Y = dmpData.Raw_Gyro.Data.Y;
            self->fifo_data.RawGyro_Z = dmpData.Raw_Gyro.Data.Z;
            self->fifo_data.RawGyro_BiasX = dmpData.Raw_Gyro.Data.BiasX;
            self->fifo_data.RawGyro_BiasY = dmpData.Raw_Gyro.Data.BiasY;
            self->fifo_data.RawGyro_BiasZ = dmpData.Raw_Gyro.Data.BiasZ;
        }
        if (dmpData.header & DMP_header_bitmap_Compass)
        {
            self->fifo_data.Compass_X = dmpData.Compass.Data.X;
            self->fifo_data.Compass_Y = dmpData.Compass.Data.Y;
            self->fifo_data.Compass_Z = dmpData.Compass.Data.Z;
        }
        if (dmpData.header & DMP_header_bitmap_ALS)
        {
            // do nothing
        }
        if (dmpData.header & DMP_header_bitmap_Quat6)
        {
            self->fifo_data.Quat6_1 = dmpData.Quat6.Data.Q1;
            self->fifo_data.Quat6_2 = dmpData.Quat6.Data.Q2;
            self->fifo_data.Quat6_3 = dmpData.Quat6.Data.Q3;
        }
        if (dmpData.header & DMP_header_bitmap_Quat9)
        {
            self->fifo_data.Quat9_1 = dmpData.Quat9.Data.Q1;
            self->fifo_data.Quat9_2 = dmpData.Quat9.Data.Q2;
            self->fifo_data.Quat9_3 = dmpData.Quat9.Data.Q3;
            self->fifo_data.Quat9_Acc = dmpData.Quat9.Data.Accuracy;
        }
        if (dmpData.header & DMP_header_bitmap_PQuat6)
        {
            self->fifo_data.PQuat6_1 = dmpData.PQuat6.Data.Q1;
            self->fifo_data.PQuat6_2 = dmpData.PQuat6.Data.Q2;
            self->fifo_data.PQuat6_3 = dmpData.PQuat6.Data.Q3;
        }
        if (dmpData.header & DMP_header_bitmap_Geomag)
        {
            self->fifo_data.Geomag_1 = dmpData.Geomag.Data.Q1;
            self->fifo_data.Geomag_2 = dmpData.Geomag.Data.Q2;
            self->fifo_data.Geomag_3 = dmpData.Geomag.Data.Q3;
            self->fifo_data.Geomag_Acc = dmpData.Geomag.Data.Accuracy;
        }
        if (dmpData.header & DMP_header_bitmap_Header2)
        {
            if (dmpData.header2 & DMP_header2_bitmap_Accel_Accuracy)
            {
                self->fifo_data.Accel_Acc = dmpData.Accel_Accuracy;
            }
            if (dmpData.header2 & DMP_header2_bitmap_Gyro_Accuracy)
            {
                self->fifo_data.Gyro_Acc = dmpData.Gyro_Accuracy;
            }
            if (dmpData.header2 & DMP_header2_bitmap_Compass_Accuracy)
            {
                self->fifo_data.Compass_Acc = dmpData.Compass_Accuracy;
            }
        }
        self->_data_mtx->unlock();
    }
}

bool IMU::getTemp_C(float &temp)
{
    bool retval = false;
    this->_device_mtx->lock();
    this->_device.getAGMT();
    if (ICM_20948_Stat_Ok != this->_device.status)
    {
        retval = true;
    }
    else
    {
        temp = this->_device.temp();
    }
    this->_device_mtx->unlock();
    return retval;
}

bool IMU::getRotVel_dps(float &rot_x, float &rot_y, float &rot_z)
{
    bool retval = false;
    this->_device_mtx->lock();
    {
        this->_device.getAGMT();
        if (ICM_20948_Stat_Ok != this->_device.status)
        {
            retval = true;
        }
        else
        {
            rot_x = this->_device.gyrX();
            rot_y = this->_device.gyrY();
            rot_z = this->_device.gyrZ();
        }
    }
    this->_device_mtx->unlock();
    return retval;
}

bool IMU::getAccel_ms2(float &acc_x, float &acc_y, float &acc_z)
{
    bool fail = false;
    this->_device_mtx->lock();
    {
        this->_device.getAGMT();
        if (ICM_20948_Stat_Ok != this->_device.status)
        {
            fail = true;
        }
        else
        {
            acc_x = this->_device.accX() / 1e3 * 9.81;
            acc_y = this->_device.accY() / 1e3 * 9.81;
            acc_z = this->_device.accZ() / 1e3 * 9.81;
        }
    }
    this->_device_mtx->unlock();
    return fail;
}

bool IMU::getMag_uT(float &mag_x, float &mag_y, float &mag_z)
{
    bool fail = false;
    this->_device_mtx->lock();
    {
        this->_device.getAGMT();
        if (ICM_20948_Stat_Ok != this->_device.status)
        {
            fail = true;
        }
        else
        {
            mag_x = this->_device.magX();
            mag_y = this->_device.magY();
            mag_z = this->_device.magZ();
        }
    }
    this->_device_mtx->unlock();
    return fail;
}

bool IMU::getDmpAccel_ms2(float &acc_x, float &acc_y, float &acc_z)
{
    bool fail = false;
    this->_data_mtx->lock();
    {
        acc_x = this->fifo_data.RawAccel_X / 4.096 / 1e3 * 9.81;
        acc_y = this->fifo_data.RawAccel_Y / 4.096 / 1e3 * 9.81;
        acc_z = this->fifo_data.RawAccel_Z / 4.096 / 1e3 * 9.81;
    }
    this->_data_mtx->unlock();
    return fail;
}

bool IMU::getDmpQuat(double &q0, double &q1, double &q2, double &q3, double *acc)
{
    bool fail = false;
    this->_data_mtx->lock();
    {
        q1 = this->fifo_data.Quat9_1 / Q_SCALE;
        q2 = this->fifo_data.Quat9_2 / Q_SCALE;
        q3 = this->fifo_data.Quat9_3 / Q_SCALE;
        if (acc)
        {
            *acc = this->fifo_data.Quat9_Acc / Q_SCALE;
        }
    }
    this->_data_mtx->unlock();
    q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
    return fail;
}

bool IMU::getDmpQuatf(float &q0, float &q1, float &q2, float &q3, float *acc)
{
    bool fail = false;
    this->_data_mtx->lock();
    {
        q1 = this->fifo_data.Quat9_1 / Q_SCALE;
        q2 = this->fifo_data.Quat9_2 / Q_SCALE;
        q3 = this->fifo_data.Quat9_3 / Q_SCALE;
        if (acc)
        {
            *acc = this->fifo_data.Quat9_Acc / Q_SCALE;
        }
    }
    this->_data_mtx->unlock();
    q0 = sqrtf(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
    return fail;
}

bool IMU::getDmpRotVel_dps(float &rot_x, float &rot_y, float &rot_z)
{
    bool fail = false;
    this->_data_mtx->lock();
    {
        rot_x = this->fifo_data.RawGyro_X / 65.5;
        rot_y = this->fifo_data.RawGyro_Y / 65.5;
        rot_z = this->fifo_data.RawGyro_Z / 65.5;
    }
    this->_data_mtx->unlock();
    return fail;
}

void IMU::dumpRegs(int (*printfn)(const char *s, ...))
{
    uint8_t data;
    struct tab_entry
    {
        uint8_t bank;
        uint8_t addr;
        const char *const name;
    };
    ICM_20948_Status_e status;
    struct tab_entry reg_table[] = {
        {0, AGB0_REG_WHO_AM_I, "WHO_AM_I"},
        {0, AGB0_REG_USER_CTRL, "USER_CTRL"},
        {0, AGB0_REG_LP_CONFIG, "LP_CONFIG"},
        {0, AGB0_REG_PWR_MGMT_1, "PWR_MGMT_1"},
        {0, AGB0_REG_PWR_MGMT_2, "PWR_MGMT_2"},
        {0, AGB0_REG_INT_PIN_CONFIG, "INT_PIN_CFG"},
        {0, AGB0_REG_INT_ENABLE, "INT_ENABLE"},
        {0, AGB0_REG_INT_ENABLE_1, "INT_ENABLE_1"},
        {0, AGB0_REG_INT_ENABLE_2, "INT_ENABLE_2"},
        {0, AGB0_REG_INT_ENABLE_3, "INT_ENABLE_3"},
        {0, AGB0_REG_I2C_MST_STATUS, "I2C_MST_STATUS"},
        {0, AGB0_REG_INT_STATUS, "INT_STATUS"},
        {0, AGB0_REG_INT_STATUS_1, "INT_STATUS_1"},
        {0, AGB0_REG_INT_STATUS_2, "INT_STATUS_2"},
        {0, AGB0_REG_INT_STATUS_3, "INT_STATUS_3"},
        {0, AGB0_REG_DELAY_TIMEH, "DELAY_TIMEH"},
        {0, AGB0_REG_DELAY_TIMEL, "DELAY_TIMEL"},
        {0, AGB0_REG_ACCEL_XOUT_H, "ACCEL_XOUT_H"},
        {0, AGB0_REG_ACCEL_XOUT_L, "ACCEL_XOUT_L"},
        {0, AGB0_REG_ACCEL_YOUT_H, "ACCEL_YOUT_H"},
        {0, AGB0_REG_ACCEL_YOUT_L, "ACCEL_YOUT_L"},
        {0, AGB0_REG_ACCEL_ZOUT_H, "ACCEL_ZOUT_H"},
        {0, AGB0_REG_ACCEL_ZOUT_L, "ACCEL_ZOUT_L"},
        {0, AGB0_REG_GYRO_XOUT_H, "GYRO_XOUT_H"},
        {0, AGB0_REG_GYRO_XOUT_L, "GYRO_XOUT_L"},
        {0, AGB0_REG_GYRO_YOUT_H, "GYRO_YOUT_H"},
        {0, AGB0_REG_GYRO_YOUT_L, "GYRO_YOUT_L"},
        {0, AGB0_REG_GYRO_ZOUT_H, "GYRO_ZOUT_H"},
        {0, AGB0_REG_GYRO_ZOUT_L, "GYRO_ZOUT_L"},
        {0, AGB0_REG_TEMP_OUT_H, "TEMP_OUT_H"},
        {0, AGB0_REG_TEMP_OUT_L, "TEMP_OUT_L"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_00, "EXT_SLV_SENS_DATA_00"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_01, "EXT_SLV_SENS_DATA_01"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_02, "EXT_SLV_SENS_DATA_02"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_03, "EXT_SLV_SENS_DATA_03"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_04, "EXT_SLV_SENS_DATA_04"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_05, "EXT_SLV_SENS_DATA_05"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_06, "EXT_SLV_SENS_DATA_06"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_07, "EXT_SLV_SENS_DATA_07"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_08, "EXT_SLV_SENS_DATA_08"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_09, "EXT_SLV_SENS_DATA_09"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_10, "EXT_SLV_SENS_DATA_10"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_11, "EXT_SLV_SENS_DATA_11"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_12, "EXT_SLV_SENS_DATA_12"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_13, "EXT_SLV_SENS_DATA_13"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_14, "EXT_SLV_SENS_DATA_14"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_15, "EXT_SLV_SENS_DATA_15"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_16, "EXT_SLV_SENS_DATA_16"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_17, "EXT_SLV_SENS_DATA_17"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_18, "EXT_SLV_SENS_DATA_18"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_19, "EXT_SLV_SENS_DATA_19"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_20, "EXT_SLV_SENS_DATA_20"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_21, "EXT_SLV_SENS_DATA_21"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_22, "EXT_SLV_SENS_DATA_22"},
        {0, AGB0_REG_EXT_PERIPH_SENS_DATA_23, "EXT_SLV_SENS_DATA_23"},
        {0, AGB0_REG_FIFO_EN_1, "FIFO_EN_1"},
        {0, AGB0_REG_FIFO_EN_2, "FIFO_EN_2"},
        {0, AGB0_REG_FIFO_RST, "FIFO_RST"},
        {0, AGB0_REG_FIFO_MODE, "FIFO_MODE"},
        {0, AGB0_REG_FIFO_COUNT_H, "FIFO_COUNTH"},
        {0, AGB0_REG_FIFO_COUNT_L, "FIFO_COUNTL"},
        {0, AGB0_REG_FIFO_R_W, "FIFO_R_W"},
        {0, AGB0_REG_DATA_RDY_STATUS, "DATA_RDY_STATUS"},
        {0, AGB0_REG_FIFO_CFG, "FIFO_CFG"},
        {0, AGB0_REG_REG_BANK_SEL, "REG_BANK_SEL"},
        {1, AGB1_REG_SELF_TEST_X_GYRO, "SELF_TEST_X_GYRO"},
        {1, AGB1_REG_SELF_TEST_Y_GYRO, "SELF_TEST_Y_GYRO"},
        {1, AGB1_REG_SELF_TEST_Z_GYRO, "SELF_TEST_Z_GYRO"},
        {1, AGB1_REG_SELF_TEST_X_ACCEL, "SELF_TEST_X_ACCEL"},
        {1, AGB1_REG_SELF_TEST_Y_ACCEL, "SELF_TEST_Y_ACCEL"},
        {1, AGB1_REG_SELF_TEST_Z_ACCEL, "SELF_TEST_Z_ACCEL"},
        {1, AGB1_REG_XA_OFFS_H, "XA_OFFS_H"},
        {1, AGB1_REG_XA_OFFS_L, "XA_OFFS_L"},
        {1, AGB1_REG_YA_OFFS_H, "YA_OFFS_H"},
        {1, AGB1_REG_YA_OFFS_L, "YA_OFFS_L"},
        {1, AGB1_REG_ZA_OFFS_H, "ZA_OFFS_H"},
        {1, AGB1_REG_ZA_OFFS_L, "ZA_OFFS_L"},
        {1, AGB1_REG_TIMEBASE_CORRECTION_PLL, "TIMEBASE_CORRECTION_PLL"},
        {1, AGB1_REG_REG_BANK_SEL, "REG_BANK_SEL"},
        {2, AGB2_REG_GYRO_SMPLRT_DIV, "GYRO_SMPLRT_DIV"},
        {2, AGB2_REG_GYRO_CONFIG_1, "GYRO_CONFIG_1"},
        {2, AGB2_REG_GYRO_CONFIG_2, "GYRO_CONFIG_2"},
        {2, AGB2_REG_XG_OFFS_USRH, "XG_OFFS_USRH"},
        {2, AGB2_REG_XG_OFFS_USRL, "XG_OFFS_USRL"},
        {2, AGB2_REG_YG_OFFS_USRH, "YG_OFFS_USRH"},
        {2, AGB2_REG_YG_OFFS_USRL, "YG_OFFS_USRL"},
        {2, AGB2_REG_ZG_OFFS_USRH, "ZG_OFFS_USRH"},
        {2, AGB2_REG_ZG_OFFS_USRL, "ZG_OFFS_USRL"},
        {2, AGB2_REG_ODR_ALIGN_EN, "ODR_ALIGN_EN"},
        {2, AGB2_REG_ACCEL_SMPLRT_DIV_1, "ACCEL_SMPLRT_DIV_1"},
        {2, AGB2_REG_ACCEL_SMPLRT_DIV_2, "ACCEL_SMPLRT_DIV_2"},
        {2, AGB2_REG_ACCEL_INTEL_CTRL, "ACCEL_INTEL_CTRL"},
        {2, AGB2_REG_ACCEL_WOM_THR, "ACCEL_WOM_THR"},
        {2, AGB2_REG_ACCEL_CONFIG, "ACCEL_CONFIG"},
        {2, AGB2_REG_ACCEL_CONFIG_2, "ACCEL_CONFIG_2"},
        {2, AGB2_REG_FSYNC_CONFIG, "FSYNC_CONFIG"},
        {2, AGB2_REG_TEMP_CONFIG, "TEMP_CONFIG"},
        {2, AGB2_REG_MOD_CTRL_USR, "MOD_CTRL_USR"},
        {2, AGB2_REG_REG_BANK_SEL, "REG_BANK_SEL"},
        {3, AGB3_REG_I2C_MST_ODR_CONFIG, "I2C_MST_ODR_CONFIG"},
        {3, AGB3_REG_I2C_MST_CTRL, "I2C_MST_CTRL"},
        {3, AGB3_REG_I2C_MST_DELAY_CTRL, "I2C_MST_DELAY_CTRL"},
        {3, AGB3_REG_I2C_PERIPH0_ADDR, "I2C_SLV0_ADDR"},
        {3, AGB3_REG_I2C_PERIPH0_REG, "I2C_SLV0_REG"},
        {3, AGB3_REG_I2C_PERIPH0_CTRL, "I2C_SLV0_CTRL"},
        {3, AGB3_REG_I2C_PERIPH0_DO, "I2C_SLV0_DO"},
        {3, AGB3_REG_I2C_PERIPH1_ADDR, "I2C_SLV1_ADDR"},
        {3, AGB3_REG_I2C_PERIPH1_REG, "I2C_SLV1_REG"},
        {3, AGB3_REG_I2C_PERIPH1_CTRL, "I2C_SLV1_CTRL"},
        {3, AGB3_REG_I2C_PERIPH1_DO, "I2C_SLV1_DO"},
        {3, AGB3_REG_I2C_PERIPH2_ADDR, "I2C_SLV2_ADDR"},
        {3, AGB3_REG_I2C_PERIPH2_REG, "I2C_SLV2_REG"},
        {3, AGB3_REG_I2C_PERIPH2_CTRL, "I2C_SLV2_CTRL"},
        {3, AGB3_REG_I2C_PERIPH2_DO, "I2C_SLV2_DO"},
        {3, AGB3_REG_I2C_PERIPH3_ADDR, "I2C_SLV3_ADDR"},
        {3, AGB3_REG_I2C_PERIPH3_REG, "I2C_SLV3_REG"},
        {3, AGB3_REG_I2C_PERIPH3_CTRL, "I2C_SLV3_CTRL"},
        {3, AGB3_REG_I2C_PERIPH3_DO, "I2C_SLV3_DO"},
        {3, AGB3_REG_I2C_PERIPH4_ADDR, "I2C_SLV4_ADDR"},
        {3, AGB3_REG_I2C_PERIPH4_REG, "I2C_SLV4_REG"},
        {3, AGB3_REG_I2C_PERIPH4_CTRL, "I2C_SLV4_CTRL"},
        {3, AGB3_REG_I2C_PERIPH4_DO, "I2C_SLV4_DO"},
        {3, AGB3_REG_REG_BANK_SEL, "REG_BANK_SEL"},
        {4, AK09916_REG_WIA2, "WIA2"},
        {4, AK09916_REG_ST1, "ST1"},
        {4, AK09916_REG_HXL, "HXL"},
        {4, AK09916_REG_HXH, "HXH"},
        {4, AK09916_REG_HYL, "HYL"},
        {4, AK09916_REG_HYH, "HYH"},
        {4, AK09916_REG_HZL, "HZL"},
        {4, AK09916_REG_HZH, "HZH"},
        {4, AK09916_REG_ST2, "ST2"},
        {4, AK09916_REG_CNTL2, "CNTL2"},
        {4, AK09916_REG_CNTL3, "CNTL3"},
        {0, 0, NULL}};
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    for (struct tab_entry *entry = reg_table; entry->name; entry++)
    {
        if (entry->bank < 4)
        {
            status = this->_device.setBank(entry->bank);
            if (ICM_20948_Stat_Ok != status)
            {
                break;
            }
            status = this->_device.read(entry->addr, &data, 1);
            if (ICM_20948_Stat_Ok != status)
            {
                break;
            }
        }
        else if (entry->bank == 4)
        {
            data = this->_device.readMag((AK09916_Reg_Addr_e)entry->addr);
        }
        printfn("%32s: 0b%d%d%d%d %d%d%d%d (0x%02x)" __NL__,
                entry->name,
                (data & 0x80) >> 7,
                (data & 0x40) >> 6,
                (data & 0x20) >> 5,
                (data & 0x10) >> 4,
                (data & 0x08) >> 3,
                (data & 0x04) >> 2,
                (data & 0x02) >> 1,
                (data & 0x01) >> 0,
                data);
    }
#endif
}