/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/home/emily-thorpe/E4E/smartfin-fw3/src/smartfin-fw3.ino"
/*
 * Project smartfin-fw3
 * Description:
 * Author:
 * Date:
 */
#include "Particle.h"
#include "states.hpp"
#include "task.hpp"


// Statemachine for handeling task-switching
void setup();
void loop();
#line 13 "/home/emily-thorpe/E4E/smartfin-fw3/src/smartfin-fw3.ino"
typedef struct StateMachine_
{
  STATES_e state;
  Task* task;
}StateMachine_t;

// static CLI cliTask; // TODO


// Holds the list of states and coresponding tasks
static StateMachine_t stateMachine[] = 
{
  // {STATE_CLI, &cliTask}, // TODO
  {STATE_NULL, NULL}
};

static STATES_e currentState;

static StateMachine_t* findState(STATES_e state);
void mainThread(void* args);

// setup() runs once, when the device is first turned on.
void setup() {
    currentState = STATE_NULL;

    // TODO: add flog error protection here

    // Put initialization like pinMode and begin functions here.
    Serial.begin(9500);
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