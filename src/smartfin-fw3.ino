/*
 * Project smartfin-fw3
 * Description: 
 * Author:
 * Date:
 */
#include "Particle.h"

#include "states.hpp"
#include "task.hpp"
#include "product.hpp"
#include "consts.hpp"
#include "system.hpp"

#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"

#include "sleepTask.hpp"
#include "chargeTask.hpp"

SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

// Statemachine for handeling task-switching
typedef struct StateMachine_
{
    STATES_e state;
    Task* task;
}StateMachine_t;

static CLI cliTask;
static ChargeTask chargeTask;
static SleepTask sleepTask;

// Holds the list of states and coresponding tasks
static StateMachine_t stateMachine[] = 
{
    {STATE_CLI, &cliTask},
    {STATE_DEEP_SLEEP, &sleepTask},
    {STATE_CHARGE, &chargeTask},
    {STATE_NULL, NULL}
};

static STATES_e currentState;

static StateMachine_t* findState(STATES_e state);
static void initalizeTaskObjects(void);
void mainThread(void* args);
void printState(STATES_e state);

// setup() runs once, when the device is first turned on.
void setup() {
    Serial.begin(SF_SERIAL_SPEED);

    currentState = STATE_CLI;

    
    FLOG_Initialize();
    FLOG_AddError(FLOG_SYS_START, 0); 
    time32_t bootTime = Time.now();
    SF_OSAL_printf("Boot time: %" PRId32 __NL__, bootTime);

    SYS_initSys();

    initalizeTaskObjects();

    currentState = STATE_CHARGE;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    mainThread(NULL);
}

void mainThread(void* args) {

    StateMachine_t* pState;
    // Starting main thread

    pState = findState(currentState);
    SF_OSAL_printf(__NL__ "Starting state: ");

    FLOG_AddError(FLOG_SYS_STARTSTATE, currentState);
    printState(currentState);

    if (pState == NULL) {
        pState = findState(SF_DEFAULT_STATE);
    }

    pState->task->init();

    currentState = pState->task->run();

    pState->task->exit();
}

static void initalizeTaskObjects(void) 
{
    currentState = SF_DEFAULT_STATE;

    SleepTask::loadBootBehavior();

    bool no_upload_flag;
    pSystemDesc->pNvram->get(NVRAM::NO_UPLOAD_FLAG, no_upload_flag);

    switch(SleepTask::getBootBehavior())
    {
        default:
        case SleepTask::BOOT_BEHAVIOR_NORMAL:
              currentState = SF_DEFAULT_STATE;
            break;
    }

    FLOG_AddError(FLOG_SYS_STARTSTATE, (uint16_t) currentState);
}

static StateMachine_t* findState(STATES_e state)
{
    StateMachine_t* pStates;
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
    const char* pStateName;
    if (state >= STATE_N_STATES)
    {
        // Illegal state value
        return;
    }
    pStateName = STATES_NAME_TAB[state];
    SF_OSAL_printf("%s" __NL__, pStateName);
}