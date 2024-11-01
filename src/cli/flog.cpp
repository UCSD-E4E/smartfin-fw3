/**
 * @file flog.cpp
 * @author @emilybthorpe
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Fault Log (FLOG) with persistent memory
 * @version 0.1
 * @date 2024-10-31
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "flog.hpp"

#include "conio.hpp"
#include "consts.hpp"
#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include <Particle.h>
#elif SF_PLATFORM == SF_PLATFORM_GCC
#include "scheduler_test_system.hpp"

#include <ctime>
#include <stdlib.h>
#include <string.h>
#endif

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

struct FLOG_Printer
{
    FLOG_CODE_e code;
    void (*printer)(const FLOG_Entry_t &);
};

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
retained FLOG_Data_t flogData;
#elif SF_PLATFORM == SF_PLATFORM_GCC
FLOG_Data_t flogData;
#endif
static char FLOG_unknownMessage[256];

static const char* FLOG_FindMessage(FLOG_CODE_e code);
static int FLOG_IsInitialized(void);
static void FLOG_fmt_sys_start(const FLOG_Entry_t &entry);
static void FLOG_fmt_reset_reason(const FLOG_Entry_t &entry);
static void FLOG_fmt_default(const FLOG_Entry_t &entry);

const FLOG_Message_t FLOG_Message[] = {{FLOG_SYS_START, "System Start"},
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
                                       {FLOG_SCHEDULER_DELAY_EXCEEDED, "Ensemble skipped"},

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
                                       {FLOG_NULL, NULL}};

const struct FLOG_Printer formatter_table[] = {{FLOG_RESET_REASON, FLOG_fmt_reset_reason},
                                               {FLOG_SYS_START, FLOG_fmt_sys_start},
                                               {FLOG_NULL, FLOG_fmt_default}};

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
    const FLOG_Entry_t *pEntry;
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
        pEntry = &flogData.flogEntries[i & (FLOG_NUM_ENTRIES - 1)];
        const struct FLOG_Printer *ptr;
        for (ptr = formatter_table; ptr->code != FLOG_NULL; ptr++)
        {
            if (ptr->code == pEntry->errorCode)
            {
                break;
            }
        }
        ptr->printer(*pEntry);
    }
    SF_OSAL_printf(__NL__);
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

static void FLOG_fmt_sys_start(const FLOG_Entry_t &entry)
{
    const char *time_str;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    time_str = Time.format((time32_t)entry.param).c_str();
#elif SF_PLATFORM == SF_PLATFORM_GCC
    time_t timestamp;
    timestamp = entry.param;
    time_str = ctime(&timestamp);
#endif
    SF_OSAL_printf("%8d System Started at %s" __NL__, entry.timestamp_ms, time_str);
}

static void FLOG_fmt_reset_reason(const FLOG_Entry_t &entry)
{
#if SF_PLATFORM == SF_PLATFORM_GCC
    SF_OSAL_printf("Reset reason not available" __NL__);
#elif SF_PLATFORM == SF_PLATFORM_PARTICLE
    struct reason_mapping
    {
        System_Reset_Reason code;
        const char *text;
    };
    struct reason_mapping reason_map[] = {
        {RESET_REASON_UNKNOWN, "Unspecified reason"},
        {RESET_REASON_PIN_RESET, "Reset pin assert"},
        {RESET_REASON_POWER_MANAGEMENT, "Low-power management reset"},
        {RESET_REASON_POWER_DOWN, "Power-down reset"},
        {RESET_REASON_POWER_BROWNOUT, "Brownout reset"},
        {RESET_REASON_WATCHDOG, "Watchdog reset"},
        {RESET_REASON_UPDATE, "Reset to apply firmware update"},
        {RESET_REASON_UPDATE_ERROR, "Generic firmware update error"},
        {RESET_REASON_UPDATE_TIMEOUT, "Firmware update timeout"},
        {RESET_REASON_FACTORY_RESET, "Factory reset requested"},
        {RESET_REASON_SAFE_MODE, "Safe mode requested"},
        {RESET_REASON_DFU_MODE, "DFU mode requested"},
        {RESET_REASON_PANIC, "System panic"},
        {RESET_REASON_USER, "User reset"},
        {RESET_REASON_CONFIG_UPDATE, "Config change update"},
        {RESET_REASON_NONE, "Invalid reason code"},
    };
    struct reason_mapping *ptr;
    for (ptr = reason_map; ptr->code != RESET_REASON_NONE; ptr++)
    {
        if (ptr->code == (System_Reset_Reason)entry.param)
        {
            break;
        }
    }
    SF_OSAL_printf("%8d Reset due to %s" __NL__, entry.timestamp_ms, ptr->text);

#endif
}

static void FLOG_fmt_default(const FLOG_Entry_t &entry)
{
    SF_OSAL_printf("%8d %32s, parameter: 0x%08" FLOG_PARAM_FMT __NL__,
                   entry.timestamp_ms,
                   FLOG_FindMessage((FLOG_CODE_e)entry.errorCode),
                   entry.param);
}