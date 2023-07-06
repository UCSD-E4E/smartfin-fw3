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
#include "cli/cli.hpp"


// Statemachine for handeling task-switching
typedef struct StateMachine_
{
  STATES_e state;
  Task* task;
}StateMachine_t;

static CLI cliTask;


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
    currentState = STATE_NULL;

    // TODO: add flog error protection here

    // Put initialization like pinMode and begin functions here.
    Serial.begin(9500);

    initalizeTaskObjects();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    mainThread(NULL);
}

void mainThread(void* args) {
  StateMachine_t* pState;
  // Starting main thread
  while(1) 
  {
    pState = findState(currentState);
    if(NULL == pState)
    {
      pState = findState(STATE_NULL); 
    }
    // initalizing state
    pState->task->init();

    // running state
    currentState = pState->task->run();

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
  // FLOG_AddError(FLOG_SYS_UNKNOWNSTATE, state);
  return NULL;
}