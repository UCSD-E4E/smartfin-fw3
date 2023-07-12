/*
 * Project smartfin-fw3
 * Description:
 * Author:
 * Date:
 */
#include "Particle.h"
// #include "states.hpp"
// #include "task.hpp"
// #include "product.hpp"
#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"

// int counter = 0;

// Statemachine for handeling task-switching
typedef struct StateMachine_
{
  STATES_e state;
  Task* task;
}StateMachine_t;

static CLI cliTask;

static retained uint32_t RESET_GOOD;
static retained uint32_t nRESET_GOOD;


// Holds the list of states and coresponding tasks
static StateMachine_t stateMachine[] = 
{
  {STATE_CLI, &cliTask},
  {STATE_NULL, NULL}
};

static STATES_e currentState;

static StateMachine_t* findState(STATES_e state);
static void initalizeTaskObjects(void);
void mainThread(void* args);

// setup() runs once, when the device is first turned on.
void setup() {
    Serial.begin(9600);

    currentState = STATE_NULL;


    FLOG_Initialize();
    FLOG_AddError(FLOG_SYS_START, 0); 

    initalizeTaskObjects();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    SF_OSAL_printf("OSAL: Test");
    // SF_OSAL_printf("Counter", counter++);
    // Serial.print("test-serial");
    // Serial.print("Serial: Test");
    // FLOG_DisplayLog();
    mainThread(NULL);
}

void mainThread(void* args) {
  StateMachine_t* pState;
  // Starting main thread
  while(1) 
  {
    SF_OSAL_printf("Starting state");
    pState = findState(currentState);
    if(NULL == pState)
    {
      pState = findState(STATE_NULL); 
    }
    // initalizing state
    pState->task->init();

    // running state
    currentState = pState->task->run();
    SF_OSAL_printf("Current State: ", currentState);

    // exit state
    pState->task->exit();
  }
}



static void initalizeTaskObjects(void) 
{
  currentState = STATE_CLI;
}

static StateMachine_t* findState(STATES_e state)
{
  StateMachine_t* pStates;
  for(pStates = stateMachine; pStates->task; pStates++)
  {
    if(pStates->state == state)
    {
      return pStates;
    }
  }
  SF_OSAL_printf("State not found!");
  return NULL;
}