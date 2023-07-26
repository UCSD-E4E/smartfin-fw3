#ifndef __FLOG_H__
#define __FLOG_H__

#include <stdint.h>

#define FLOG_NUM_ENTRIES    256

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
    FLOG_RIDE_INIT_TIMEOUT=0x0401,
    FLOG_UPLOAD_NO_UPLOAD =0x0601,
    FLOG_UPL_BATT_LOW     =0x0602,
    FLOG_UPL_FOLDER_COUNT =0x0603,
    FLOG_UPL_CONNECT_FAIL =0x0604,
}FLOG_CODE_e;

void FLOG_Initialize(void);
void FLOG_AddError(FLOG_CODE_e errorCode, uint16_t parameter);
void FLOG_DisplayLog(void);
void FLOG_ClearLog(void);

#endif