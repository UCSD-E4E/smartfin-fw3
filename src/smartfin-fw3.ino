/*
 * Project smartfin-fw3
 * Description:
 * Author:
 * Date:
 */
#include "Particle.h"
#include "cellular/dataUpload.hpp"
#include "cellular/sf_cloud.hpp"
#include "chargeTask.hpp"
#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "mfgTest/mfgTest.hpp"
#include "product.hpp"
#include "rideTask.hpp"
#include "sleepTask.hpp"
#include "states.hpp"
#include "system.hpp"
#include "task.hpp"

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

/**
 * @brief State machine for handling task switching
 *
 * Associates a device state with a corresponding task
 */
typedef struct StateMachine_
{
    /**
     * @brief Device state associated with a task
     */
    STATES_e state;
    /**
     * @brief Task for the given device state
     */
    Task *task;
} StateMachine_t;

static CLI cliTask;
static ChargeTask chargeTask;
static SleepTask sleepTask;
static DataUpload uploadTask;
static MfgTest mfgTask;
static RideTask rideTask;

// Holds the list of states and coresponding tasks
static StateMachine_t stateMachine[] = {{STATE_CLI, &cliTask},
                                        {STATE_DEEP_SLEEP, &sleepTask},
                                        {STATE_CHARGE, &chargeTask},
                                        {STATE_UPLOAD, &uploadTask},
                                        {STATE_MFG_TEST, &mfgTask},
                                        {STATE_DEPLOYED, &rideTask},
                                        {STATE_NULL, NULL}};

static STATES_e currentState;

static StateMachine_t *findState(STATES_e state);
static void initalizeTaskObjects(void);
void mainFunc(void);
void mainLoop(void *args);
static void printState(STATES_e state);

/**
 * @brief Main thread object
 *
 */
Thread __sf_main_thread;

// setup() runs once, when the device is first turned on.
void setup()
{
    System.enableFeature(FEATURE_RESET_INFO);
    SF_OSAL_init_conio();

    FLOG_Initialize();
    time32_t bootTime = Time.now();
    FLOG_AddError(FLOG_SYS_START, bootTime);
    SF_OSAL_printf("Boot time: %" PRId32 __NL__, bootTime);

    FLOG_AddError(FLOG_RESET_REASON, System.resetReason());
    FLOG_AddError(FLOG_RESET_REASON_DATA, System.resetReasonData());

    SYS_initSys();

    initalizeTaskObjects();
    __sf_main_thread = Thread(
        "SF_main", mainLoop, NULL, OS_THREAD_PRIORITY_DEFAULT, OS_THREAD_STACK_SIZE_DEFAULT_HIGH);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
    // mainThread(NULL);
    delay(1);
}

/**
 * @brief Main loop
 *
 * Core thread loop
 *
 * @param args
 */
void mainLoop(void *args)
{
    while (1)
    {
        mainFunc();
    }
}

/**
 * @brief Main loop function
 *
 */
void mainFunc(void)
{

    StateMachine_t *pState;
    // Starting main thread

    pState = findState(currentState);
    SF_OSAL_printf(__NL__ "Starting state: ");

    FLOG_AddError(FLOG_SYS_STARTSTATE, currentState);
    printState(currentState);

    if (pState == NULL)
    {
        pState = findState(SF_DEFAULT_STATE);
    }

    pState->task->init();

    currentState = pState->task->run();

    pState->task->exit();
}

static void initalizeTaskObjects(void)
{

    SleepTask::loadBootBehavior();

    bool no_upload_flag;
    pSystemDesc->pNvram->get(NVRAM::NO_UPLOAD_FLAG, no_upload_flag);

    switch (SleepTask::getBootBehavior())
    {
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_UPLOAD_REATTEMPT:
        FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0001);
        currentState = STATE_UPLOAD;
        break;
    default:
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_NOT_SET:
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_TMP_CAL_CONTINUE:
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_TMP_CAL_END:
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_TMP_CAL_START:
    case SleepTask::BOOT_BEHAVIOR_e::BOOT_BEHAVIOR_NORMAL:
        if (pSystemDesc->pWaterSensor->getCurrentReading())
        {
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0002);
            currentState = STATE_DEPLOYED;
        }
        else if (pSystemDesc->pRecorder->hasData())
        {
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0003);
            currentState = STATE_UPLOAD;
        }
        else if (pSystemDesc->flags->hasCharger)
        {
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0004);
            currentState = SF_DEFAULT_STATE;
        }
        else
        {
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0005);
            currentState = STATE_DEEP_SLEEP;
        }
        break;
    }
}

static StateMachine_t *findState(STATES_e state)
{
    StateMachine_t *pStates;
    // SF_OSAL_printf("Searching for %d" __NL__, state);
    for (pStates = stateMachine; pStates->task; pStates++)
    {
        // SF_OSAL_printf("Checking index %d, state: %d, task: 0x%08x" __NL__,
        //                pStates - stateMachine,
        //                pStates->state,
        //                pStates->task);
        if (pStates->state == state)
        {
            return pStates;
        }
    }
    SF_OSAL_printf("State not found!");
    return NULL;
}

static void printState(STATES_e state)
{
    const char *pStateName;
    if (state >= STATE_N_STATES)
    {
        // Illegal state value
        return;
    }
    pStateName = STATES_NAME_TAB[state];
    SF_OSAL_printf("%s" __NL__, pStateName);
}
