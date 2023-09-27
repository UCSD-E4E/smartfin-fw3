#ifndef __FLOG_H__
#define __FLOG_H__

#include <stdint.h>
#include <inttypes.h>

#define FLOG_NUM_ENTRIES    128
#define FLOG_VALUE_TYPE     uint32_t
#define FLOG_PARAM_FMT      PRIX32

typedef enum FLOG_CODE_
{
    FLOG_NULL             =0x0000,

    FLOG_SYS_START        =0x0100,
    FLOG_SYS_BADSRAM      =0x0101,
    FLOG_SYS_STARTSTATE   =0x0102,
    FLOG_SYS_INITSTATE    =0x0103,
    FLOG_SYS_EXECSTATE    =0x0104,
    FLOG_SYS_EXITSTATE    =0x0105,
    FLOG_SYS_UNKNOWNSTATE =0x0106,
    FLOG_RESET_REASON     =0x0107,
    FLOG_CHARGER_REMOVED  =0x0108,

    FLOG_CAL_BURST        =0x0200,
    FLOG_CAL_INIT         =0x0201,
    FLOG_CAL_START_RUN    =0x0202,
    FLOG_CAL_LIMIT        =0x0203,
    FLOG_CAL_DONE         =0x0204,
    FLOG_CAL_EXIT         =0x0205,
    FLOG_CAL_SLEEP        =0x0206,
    FLOG_CAL_TEMP         =0x0207,

    FLOG_MAG_ID_MISMATCH  =0x0301,
    FLOG_MAG_MEAS_TO      =0x0302,
    FLOG_MAG_TEST_FAIL    =0x0303,
    FLOG_MAG_MEAS_OVRFL   =0x0304,
    FLOG_MAG_I2C_FAIL     =0x0305,
    FLOG_MAG_MODE_FAIL    =0x0306,
    FLOG_ICM_FAIL         =0x0307,

    FLOG_RIDE_INIT_TIMEOUT=0x0401,

    FLOG_UPLOAD_NO_UPLOAD =0x0501,
    FLOG_UPL_BATT_LOW     =0x0502,
    FLOG_UPL_FOLDER_COUNT =0x0503,
    FLOG_UPL_CONNECT_FAIL =0x0504,

    FLOG_GPS_INIT_FAIL    =0x0601,
    FLOG_GPS_START_FAIL   =0x0602,

    FLOG_TEMP_FAIL        =0x0704,

    FLOG_FS_OPENDIR_FAIL  =0x0800,
    FLOG_FS_STAT_FAIL     =0x0801,
    FLOG_SYS_MOUNT_FAIL   =0x0802,

    FLOG_DEBUG            =0xFFFF,
}FLOG_CODE_e;

void FLOG_Initialize(void);
void FLOG_AddError(FLOG_CODE_e errorCode, FLOG_VALUE_TYPE parameter);
void FLOG_DisplayLog(void);
void FLOG_ClearLog(void);

#endif