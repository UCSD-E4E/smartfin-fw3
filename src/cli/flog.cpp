/**
 * @file flog.cpp
 * @author @emilybthorpe
 * @brief Fault Log (FLOG) with persistent memory
 * @version 0.1
 * @date 2023-08-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "flog.hpp"

#include "conio.hpp"
#include "consts.hpp"

#include <Particle.h>


typedef struct FLOG_Entry_
{
    uint32_t timestamp_ms;
    uint16_t errorCode;
    FLOG_VALUE_TYPE param;
}__attribute__((packed)) FLOG_Entry_t;

typedef struct FLOG_Data_
{
    uint32_t numEntries;
    uint32_t nNumEntries;
    FLOG_Entry_t flogEntries[FLOG_NUM_ENTRIES];
}FLOG_Data_t;

typedef struct FLOG_Message_
{
    FLOG_CODE_e code;
    const char* message;
}FLOG_Message_t;

retained FLOG_Data_t flogData;
static char FLOG_unknownMessage[256];

static const char* FLOG_FindMessage(FLOG_CODE_e code);
static int FLOG_IsInitialized(void);

const FLOG_Message_t FLOG_Message[] = {
    {FLOG_SYS_START, "System Start"},
    {FLOG_SYS_BADSRAM, "Bad SRAM"},
    {FLOG_SYS_STARTSTATE, "Starting State"},
    {FLOG_SYS_INITSTATE, "Initializing State"},
    {FLOG_SYS_EXECSTATE, "Executing State Body"},
    {FLOG_SYS_EXITSTATE, "Exiting State"},
    {FLOG_SYS_UNKNOWNSTATE, "Unknown State"},
    {FLOG_RESET_REASON, "Reset Reason"},
    {FLOG_CHARGER_REMOVED, "Charger removed"},

    {FLOG_CAL_BURST, "Calibrate Burst"},
    {FLOG_CAL_INIT, "Calibrate Initialization"},
    {FLOG_CAL_START_RUN, "Calibrate Start RUN"},
    {FLOG_CAL_LIMIT, "Calibrate Limit of Cycles"},
    {FLOG_CAL_DONE, "Calibration complete"},
    {FLOG_CAL_EXIT, "Calbiration Exit"},
    {FLOG_CAL_SLEEP, "Calibration Sleep"},
    {FLOG_CAL_TEMP, "Calibration Temp Measurement"},

    {FLOG_MAG_ID_MISMATCH, "Compass ID Mismatch"},
    {FLOG_MAG_MEAS_TO, "Compass Measurement Timeout"},
    {FLOG_MAG_TEST_FAIL, "Compass Self-Test Failure"},
    {FLOG_MAG_MEAS_OVRFL, "Compass Measurement Overflow"},
    {FLOG_MAG_I2C_FAIL, "Compass I2C Failure"},
    {FLOG_MAG_MODE_FAIL, "Compass Mode Set Fail"},
    {FLOG_ICM_FAIL, "ICM Fail"},

    {FLOG_RIDE_INIT_TIMEOUT, "Ride init Timeout"},
    {FLOG_SCHEDULER_FAILED, "Scheduler failed"},
    {FLOG_SCHEDULER_DELAY_EXCEEDED,"Ensemble skipped"},

    {FLOG_UPLOAD_NO_UPLOAD, "Upload - No Upload Flag set"},
    {FLOG_UPL_BATT_LOW, "Upload Battery low"},
    {FLOG_UPL_FOLDER_COUNT, "Upload file count"},
    {FLOG_UPL_CONNECT_FAIL, "Upload connect fail"},
    {FLOG_UPL_OPEN_FAIL, "Upload open last session fail"},
    {FLOG_UPL_PUB_FAIL, "Upload Publish fail"},

    {FLOG_GPS_INIT_FAIL, "GPS Init Fail"},
    {FLOG_GPS_START_FAIL, "GPS Start Fail"},

    {FLOG_TEMP_FAIL, "Temp Start Fail"},

    {FLOG_FS_OPENDIR_FAIL, "opendir fail"},
    {FLOG_FS_STAT_FAIL, "stat fail"},
    {FLOG_SYS_MOUNT_FAIL, "Mounting fail"},
    {FLOG_FS_MKDIR_FAIL, "mkdir fail"},
    {FLOG_FS_CREAT_FAIL, "file create fail"},
    {FLOG_FS_OPEN_FAIL, "file open fail"},
    {FLOG_FS_WRITE_FAIL, "file write fail"},
    {FLOG_FS_CLOSE_FAIL, "file close fail"},
    {FLOG_FS_FTRUNC_FAIL, "file ftrunc fail"},
    {FLOG_FS_READ_FAIL, "file ftrunc fail"},
    {FLOG_REC_SETUP_FAIL, "Recorder setup failed"},
    {FLOG_REC_SESSION_CLOSED, "Write to Closed Session"},

    {FLOG_CELL_DISCONN_FAIL, "Cellular failed to disconnect"},

    {FLOG_SW_NULLPTR, "Software Null Pointer"},
    {FLOG_DEBUG, "debug point"},
    {FLOG_NULL, NULL}
};

void FLOG_Initialize(void)
{
    if (flogData.nNumEntries != ~flogData.numEntries)
    {
        FLOG_ClearLog();
    }
}
void FLOG_AddError(FLOG_CODE_e errorCode, FLOG_VALUE_TYPE parameter)
{
    FLOG_Entry_t* pEntry;

    if (!FLOG_IsInitialized())
    {
        FLOG_Initialize();
    }

    pEntry = &flogData.flogEntries[(flogData.numEntries) & (FLOG_NUM_ENTRIES - 1)];
    pEntry->timestamp_ms = millis();
    pEntry->errorCode = errorCode;
    pEntry->param = parameter;
    flogData.numEntries++;
    flogData.nNumEntries = ~flogData.numEntries;
}

void FLOG_DisplayLog(void)
{
    uint32_t i;
    if (!FLOG_IsInitialized())
    {
        SF_OSAL_printf("Fault Log not initialized!"  __NL__);
        return;
    }

    i = 0;
    if (flogData.numEntries > FLOG_NUM_ENTRIES)
    {
        SF_OSAL_printf("Fault Log overrun!"  __NL__);
        i = flogData.numEntries - FLOG_NUM_ENTRIES;
    }

    for (; i < flogData.numEntries; i++)
    {
        SF_OSAL_printf("%8d %32s, parameter: 0x%08" FLOG_PARAM_FMT __NL__,
            flogData.flogEntries[i & (FLOG_NUM_ENTRIES - 1)].timestamp_ms,
            FLOG_FindMessage((FLOG_CODE_e)flogData.flogEntries[i & (FLOG_NUM_ENTRIES - 1)].errorCode),
            flogData.flogEntries[i & (FLOG_NUM_ENTRIES - 1)].param);
    }
    SF_OSAL_printf(""  __NL__);
}
void FLOG_ClearLog(void)
{
    memset(&flogData, 0, sizeof(FLOG_Data_t));
    flogData.nNumEntries = ~flogData.numEntries;
}

static const char* FLOG_FindMessage(FLOG_CODE_e code)
{
    const FLOG_Message_t* pEntry;
    for (pEntry = FLOG_Message; pEntry->code; pEntry++)
    {
        if (pEntry->code == code)
        {
            return pEntry->message;
        }
    }
    sprintf(FLOG_unknownMessage, "Unknown FLOG Code: 0x%04X", code);
    return FLOG_unknownMessage;
}

static int FLOG_IsInitialized(void)
{
    return flogData.nNumEntries == ~flogData.numEntries;
}