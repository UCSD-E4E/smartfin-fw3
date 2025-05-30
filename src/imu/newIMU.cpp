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

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
// Combine all of the DMP start-up code from the earlier DMP examples
// This function is defined as __attribute__((weak)) so you can overwrite it if you want to,
//   e.g. to modify the sample rate
ICM_20948_Status_e ICM_20948::initializeDMP(void)
{
    // First, let's check if the DMP is available
    if (_device._dmp_firmware_available != true)
    {
        debugPrint(F("ICM_20948::startupDMP: DMP is not available. Please check that you have "
                     "uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
        return ICM_20948_Stat_DMPNotSupported;
    }

    ICM_20948_Status_e worstResult = ICM_20948_Stat_Ok;

#if defined(ICM_20948_USE_DMP)

    // The ICM-20948 is awake and ready but hasn't been configured. Let's step through the
    // configuration sequence from InvenSense's _confidential_ Application Note "Programming
    // Sequence for DMP Hardware Functions".

    ICM_20948_Status_e result =
        ICM_20948_Stat_Ok; // Use result and worstResult to show if the configuration was successful

    // Normally, when the DMP is not enabled, startupMagnetometer (called by startupDefault, which
    // is called by begin) configures the AK09916 magnetometer to run at 100Hz by setting the CNTL2
    // register (0x31) to 0x08. Then the ICM20948's I2C_SLV0 is configured to read nine bytes from
    // the mag every sample, starting from the STATUS1 register (0x10). ST1 includes the DRDY (Data
    // Ready) bit. Next are the six magnetometer readings (little endian). After a dummy byte, the
    // STATUS2 register (0x18) contains the HOFL (Overflow) bit.
    //
    // But looking very closely at the InvenSense example code, we can see in
    // inv_icm20948_resume_akm (in Icm20948AuxCompassAkm.c) that, when the DMP is running, the
    // magnetometer is set to Single Measurement (SM) mode and that ten bytes are read, starting
    // from the reserved RSV2 register (0x03). The datasheet does not define what registers 0x04 to
    // 0x0C contain. There is definitely some secret sauce in here... The magnetometer data appears
    // to be big endian (not little endian like the HX/Y/Z registers) and starts at register 0x04.
    // We had to examine the I2C traffic between the master and the AK09916 on the AUX_DA and AUX_CL
    // pins to discover this...
    //
    // So, we need to set up I2C_SLV0 to do the ten byte reading. The parameters passed to
    // i2cControllerConfigurePeripheral are: 0: use I2C_SLV0 MAG_AK09916_I2C_ADDR: the I2C address
    // of the AK09916 magnetometer (0x0C unshifted) AK09916_REG_RSV2: we start reading here (0x03).
    // Secret sauce... 10: we read 10 bytes each cycle true: set the I2C_SLV0_RNW ReadNotWrite bit
    // so we read the 10 bytes (not write them) true: set the I2C_SLV0_CTRL I2C_SLV0_EN bit to
    // enable reading from the peripheral at the sample rate false: clear the I2C_SLV0_CTRL
    // I2C_SLV0_REG_DIS (we want to write the register value) true: set the I2C_SLV0_CTRL
    // I2C_SLV0_GRP bit to show the register pairing starts at byte 1+2 (copied from
    // inv_icm20948_resume_akm) true: set the I2C_SLV0_CTRL I2C_SLV0_BYTE_SW to byte-swap the data
    // from the mag (copied from inv_icm20948_resume_akm)
    result = i2cControllerConfigurePeripheral(
        0, MAG_AK09916_I2C_ADDR, AK09916_REG_RSV2, 10, true, true, false, true, true);
    if (result > worstResult)
        worstResult = result;
    //
    // We also need to set up I2C_SLV1 to do the Single Measurement triggering:
    // 1: use I2C_SLV1
    // MAG_AK09916_I2C_ADDR: the I2C address of the AK09916 magnetometer (0x0C unshifted)
    // AK09916_REG_CNTL2: we start writing here (0x31)
    // 1: not sure why, but the write does not happen if this is set to zero
    // false: clear the I2C_SLV0_RNW ReadNotWrite bit so we write the dataOut byte
    // true: set the I2C_SLV0_CTRL I2C_SLV0_EN bit. Not sure why, but the write does not happen if
    // this is clear false: clear the I2C_SLV0_CTRL I2C_SLV0_REG_DIS (we want to write the register
    // value) false: clear the I2C_SLV0_CTRL I2C_SLV0_GRP bit false: clear the I2C_SLV0_CTRL
    // I2C_SLV0_BYTE_SW bit AK09916_mode_single: tell I2C_SLV1 to write the Single Measurement
    // command each sample
    result = i2cControllerConfigurePeripheral(1,
                                              MAG_AK09916_I2C_ADDR,
                                              AK09916_REG_CNTL2,
                                              1,
                                              false,
                                              true,
                                              false,
                                              false,
                                              false,
                                              AK09916_mode_single);
    if (result > worstResult)
        worstResult = result;

    // Set the I2C Master ODR configuration
    // It is not clear why we need to do this... But it appears to be essential! From the datasheet:
    // "I2C_MST_ODR_CONFIG[3:0]: ODR configuration for external sensor when gyroscope and
    // accelerometer are disabled.
    //  ODR is computed as follows: 1.1 kHz/(2^((odr_config[3:0])) )
    //  When gyroscope is enabled, all sensors (including I2C_MASTER) use the gyroscope ODR.
    //  If gyroscope is disabled, then all sensors (including I2C_MASTER) use the accelerometer
    //  ODR."
    // Since both gyro and accel are running, setting this register should have no effect. But it
    // does. Maybe because the Gyro and Accel are placed in Low Power Mode (cycled)? You can see by
    // monitoring the Aux I2C pins that the next three lines reduce the bus traffic (magnetometer
    // reads) from 1125Hz to the chosen rate: 68.75Hz in this case.
    result = setBank(3);
    if (result > worstResult)
        worstResult = result;    // Select Bank 3
    uint8_t mstODRconfig = 0x04; // Set the ODR configuration to 1100/2^4 = 68.75Hz
    result = write(AGB3_REG_I2C_MST_ODR_CONFIG, &mstODRconfig, 1);
    if (result > worstResult)
        worstResult = result; // Write one byte to the I2C_MST_ODR_CONFIG register

    // Configure clock source through PWR_MGMT_1
    // ICM_20948_Clock_Auto selects the best available clock source – PLL if ready, else use the
    // Internal oscillator
    result = setClockSource(ICM_20948_Clock_Auto);
    if (result > worstResult)
        worstResult =
            result; // This is shorthand: success will be set to false if setClockSource fails

    // Enable accel and gyro sensors through PWR_MGMT_2
    // Enable Accelerometer (all axes) and Gyroscope (all axes) by writing zero to PWR_MGMT_2
    result = setBank(0);
    if (result > worstResult)
        worstResult = result; // Select Bank 0
    uint8_t pwrMgmt2 = 0x40;  // Set the reserved bit 6 (pressure sensor disable?)
    result = write(AGB0_REG_PWR_MGMT_2, &pwrMgmt2, 1);
    if (result > worstResult)
        worstResult = result; // Write one byte to the PWR_MGMT_2 register

    // Place _only_ I2C_Master in Low Power Mode (cycled) via LP_CONFIG
    // The InvenSense Nucleo example initially puts the accel and gyro into low power mode too, but
    // then later updates LP_CONFIG so only the I2C_Master is in Low Power Mode
    result = setSampleMode(ICM_20948_Internal_Mst, ICM_20948_Sample_Mode_Cycled);
    if (result > worstResult)
        worstResult = result;

    // Disable the FIFO
    result = enableFIFO(false);
    if (result > worstResult)
        worstResult = result;

    // Disable the DMP
    result = enableDMP(false);
    if (result > worstResult)
        worstResult = result;

    // Set Gyro FSR (Full scale range) to 2000dps through GYRO_CONFIG_1
    // Set Accel FSR (Full scale range) to 4g through ACCEL_CONFIG
    ICM_20948_fss_t myFSS; // This uses a "Full Scale Settings" structure that can contain values
                           // for all configurable sensors
    myFSS.a = gpm4;        // (ICM_20948_ACCEL_CONFIG_FS_SEL_e)
                           // gpm2
                           // gpm4
                           // gpm8
                           // gpm16
    myFSS.g = dps2000;     // (ICM_20948_GYRO_CONFIG_1_FS_SEL_e)
                           // dps250
                           // dps500
                           // dps1000
                           // dps2000
    result = setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
    if (result > worstResult)
        worstResult = result;

    // The InvenSense Nucleo code also enables the gyro DLPF (but leaves GYRO_DLPFCFG set to zero =
    // 196.6Hz (3dB)) We found this by going through the SPI data generated by ZaneL's
    // Teensy-ICM-20948 library byte by byte... The gyro DLPF is enabled by default (GYRO_CONFIG_1 =
    // 0x01) so the following line should have no effect, but we'll include it anyway
    result = enableDLPF(ICM_20948_Internal_Gyr, true);
    if (result > worstResult)
        worstResult = result;

    // Enable interrupt for FIFO overflow from FIFOs through INT_ENABLE_2
    // If we see this interrupt, we'll need to reset the FIFO
    // result = intEnableOverflowFIFO( 0x1F ); if (result > worstResult) worstResult = result; //
    // Enable the interrupt on all FIFOs

    // Turn off what goes into the FIFO through FIFO_EN_1, FIFO_EN_2
    // Stop the peripheral data from being written to the FIFO by writing zero to FIFO_EN_1
    result = setBank(0);
    if (result > worstResult)
        worstResult = result; // Select Bank 0
    uint8_t zero = 0;
    result = write(AGB0_REG_FIFO_EN_1, &zero, 1);
    if (result > worstResult)
        worstResult = result;
    // Stop the accelerometer, gyro and temperature data from being written to the FIFO by writing
    // zero to FIFO_EN_2
    result = write(AGB0_REG_FIFO_EN_2, &zero, 1);
    if (result > worstResult)
        worstResult = result;

    // Turn off data ready interrupt through INT_ENABLE_1
    result = intEnableRawDataReady(false);
    if (result > worstResult)
        worstResult = result;

    // Reset FIFO through FIFO_RST
    result = resetFIFO();
    if (result > worstResult)
        worstResult = result;

    // Set gyro sample rate divider with GYRO_SMPLRT_DIV
    // Set accel sample rate divider with ACCEL_SMPLRT_DIV_2
    ICM_20948_smplrt_t mySmplrt;
    mySmplrt.g = 19; // ODR is computed as follows: 1.1 kHz/(1+GYRO_SMPLRT_DIV[7:0]). 19 = 55Hz.
                     // InvenSense Nucleo example uses 19 (0x13).
    mySmplrt.a = 19; // ODR is computed as follows: 1.125 kHz/(1+ACCEL_SMPLRT_DIV[11:0]). 19
                     // = 56.25Hz. InvenSense Nucleo example uses 19 (0x13).
    // mySmplrt.g = 4; // 225Hz
    // mySmplrt.a = 4; // 225Hz
    // mySmplrt.g = 8; // 112Hz
    // mySmplrt.a = 8; // 112Hz
    result = setSampleRate((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt);
    if (result > worstResult)
        worstResult = result;

    // Setup DMP start address through PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result = setDMPstartAddress();
    if (result > worstResult)
        worstResult = result; // Defaults to DMP_START_ADDRESS

    // Now load the DMP firmware
    result = loadDMPFirmware();
    if (result > worstResult)
        worstResult = result;

    // Write the 2 byte Firmware Start Value to ICM PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result = setDMPstartAddress();
    if (result > worstResult)
        worstResult = result; // Defaults to DMP_START_ADDRESS

    // Set the Hardware Fix Disable register to 0x48
    result = setBank(0);
    if (result > worstResult)
        worstResult = result; // Select Bank 0
    uint8_t fix = 0x48;
    result = write(AGB0_REG_HW_FIX_DISABLE, &fix, 1);
    if (result > worstResult)
        worstResult = result;

    // Set the Single FIFO Priority Select register to 0xE4
    result = setBank(0);
    if (result > worstResult)
        worstResult = result; // Select Bank 0
    uint8_t fifoPrio = 0xE4;
    result = write(AGB0_REG_SINGLE_FIFO_PRIORITY_SEL, &fifoPrio, 1);
    if (result > worstResult)
        worstResult = result;

    // Configure Accel scaling to DMP
    // The DMP scales accel raw data internally to align 1g as 2^25
    // In order to align internal accel raw data 2^25 = 1g write 0x04000000 when FSR is 4g
    const unsigned char accScale[4] = {0x04, 0x00, 0x00, 0x00};
    result = writeDMPmems(ACC_SCALE, 4, &accScale[0]);
    if (result > worstResult)
        worstResult = result; // Write accScale to ACC_SCALE DMP register
    // In order to output hardware unit data as configured FSR write 0x00040000 when FSR is 4g
    const unsigned char accScale2[4] = {0x00, 0x04, 0x00, 0x00};
    result = writeDMPmems(ACC_SCALE2, 4, &accScale2[0]);
    if (result > worstResult)
        worstResult = result; // Write accScale2 to ACC_SCALE2 DMP register

    // Configure Compass mount matrix and scale to DMP
    // The mount matrix write to DMP register is used to align the compass axes with accel/gyro.
    // This mechanism is also used to convert hardware unit to uT. The value is expressed as 1uT =
    // 2^30. Each compass axis will be converted as below: X = raw_x * CPASS_MTX_00 + raw_y *
    // CPASS_MTX_01 + raw_z * CPASS_MTX_02 Y = raw_x * CPASS_MTX_10 + raw_y * CPASS_MTX_11 + raw_z *
    // CPASS_MTX_12 Z = raw_x * CPASS_MTX_20 + raw_y * CPASS_MTX_21 + raw_z * CPASS_MTX_22 The
    // AK09916 produces a 16-bit signed output in the range +/-32752 corresponding to +/-4912uT. 1uT
    // = 6.66 ADU. 2^30 / 6.66666 = 161061273 = 0x9999999
    const unsigned char mountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char mountMultiplierPlus[4] = {
        0x09, 0x99, 0x99, 0x99}; // Value taken from InvenSense Nucleo example
    const unsigned char mountMultiplierMinus[4] = {
        0xF6, 0x66, 0x66, 0x67}; // Value taken from InvenSense Nucleo example
    result = writeDMPmems(CPASS_MTX_00, 4, &mountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_01, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_02, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_10, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_11, 4, &mountMultiplierMinus[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_12, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_20, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_21, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(CPASS_MTX_22, 4, &mountMultiplierMinus[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the B2S Mounting Matrix
    const unsigned char b2sMountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char b2sMountMultiplierPlus[4] = {
        0x40, 0x00, 0x00, 0x00}; // Value taken from InvenSense Nucleo example
    result = writeDMPmems(B2S_MTX_00, 4, &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_01, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_02, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_10, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_11, 4, &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_12, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_20, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_21, 4, &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = writeDMPmems(B2S_MTX_22, 4, &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the DMP Gyro Scaling Factor
    // @param[in] gyro_div Value written to GYRO_SMPLRT_DIV register, where
    //            0=1125Hz sample rate, 1=562.5Hz sample rate, ... 4=225Hz sample rate, ...
    //            10=102.2727Hz sample rate, ... etc.
    // @param[in] gyro_level 0=250 dps, 1=500 dps, 2=1000 dps, 3=2000 dps
    result = setGyroSF(19, 3);
    if (result > worstResult)
        worstResult = result; // 19 = 55Hz (see above), 3 = 2000dps (see above)

    // Configure the Gyro full scale
    // 2000dps : 2^28
    // 1000dps : 2^27
    //  500dps : 2^26
    //  250dps : 2^25
    const unsigned char gyroFullScale[4] = {0x10, 0x00, 0x00, 0x00}; // 2000dps : 2^28
    result = writeDMPmems(GYRO_FULLSCALE, 4, &gyroFullScale[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Only Gain: 15252014 (225Hz) 30504029 (112Hz) 61117001 (56Hz)
    const unsigned char accelOnlyGain[4] = {0x03, 0xA4, 0x92, 0x49}; // 56Hz
    // const unsigned char accelOnlyGain[4] = {0x00, 0xE8, 0xBA, 0x2E}; // 225Hz
    // const unsigned char accelOnlyGain[4] = {0x01, 0xD1, 0x74, 0x5D}; // 112Hz
    result = writeDMPmems(ACCEL_ONLY_GAIN, 4, &accelOnlyGain[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Alpha Var: 1026019965 (225Hz) 977872018 (112Hz) 882002213 (56Hz)
    const unsigned char accelAlphaVar[4] = {0x34, 0x92, 0x49, 0x25}; // 56Hz
    // const unsigned char accelAlphaVar[4] = {0x3D, 0x27, 0xD2, 0x7D}; // 225Hz
    // const unsigned char accelAlphaVar[4] = {0x3A, 0x49, 0x24, 0x92}; // 112Hz
    result = writeDMPmems(ACCEL_ALPHA_VAR, 4, &accelAlphaVar[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel A Var: 47721859 (225Hz) 95869806 (112Hz) 191739611 (56Hz)
    const unsigned char accelAVar[4] = {0x0B, 0x6D, 0xB6, 0xDB}; // 56Hz
    // const unsigned char accelAVar[4] = {0x02, 0xD8, 0x2D, 0x83}; // 225Hz
    // const unsigned char accelAVar[4] = {0x05, 0xB6, 0xDB, 0x6E}; // 112Hz
    result = writeDMPmems(ACCEL_A_VAR, 4, &accelAVar[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Cal Rate
    const unsigned char accelCalRate[4] = {0x00,
                                           0x00}; // Value taken from InvenSense Nucleo example
    result = writeDMPmems(ACCEL_CAL_RATE, 2, &accelCalRate[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Compass Time Buffer. The I2C Master ODR Configuration (see above) sets the
    // magnetometer read rate to 68.75Hz. Let's set the Compass Time Buffer to 69 (Hz).
    const unsigned char compassRate[2] = {0x00, 0x45}; // 69Hz
    result = writeDMPmems(CPASS_TIME_BUFFER, 2, &compassRate[0]);
    if (result > worstResult)
        worstResult = result;

    // Enable DMP interrupt
    // This would be the most efficient way of getting the DMP data, instead of polling the FIFO
    // result = intEnableDMP(true); if (result > worstResult) worstResult = result;

#endif

    return worstResult;
}
#endif // SF_PLATFORM