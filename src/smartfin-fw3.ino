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
#include "cli/conio.hpp"
#include "cli/flog.hpp"

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
    currentState = STATE_NULL;

    // TODO: add flog error protection here
    FLOG_Initialize();
    FLOG_AddError(FLOG_SYS_START, 0);

    if(RESET_GOOD != ~nRESET_GOOD)
    {
      FLOG_AddError(FLOG_SYS_BADSRAM, 0);
      // if in boot loop, that's 15 seconds to save everything
      if(System.resetReason() == RESET_REASON_POWER_DOWN)
      {

      }
      else
      {
        SF_OSAL_printf("Waiting ");
        for(int i = 0; i < 15; i++)
        {
          SF_OSAL_printf("%d  ", i);
          delay(1000);
        }
        SF_OSAL_printf("\n");
      }
    }

    #ifdef SF_ENABLE_DEBUG_DELAY
    #warning Initialization delay enabled!
    SF_OSAL_printf("Waiting ");
    for(i = 0; i < SF_ENABLE_DEBUG_DELAY; i++)
    {
      SF_OSAL_printf("%d  ", i);
      delay(1000);
    }
    SF_OSAL_printf("\n");
    #endif

    // Put initialization like pinMode and begin functions here.
    Serial.begin(9500);

    RESET_GOOD = 0;
    nRESET_GOOD = 0;

    initalizeTaskObjects();
    SF_OSAL_printf("Reset Reason: %d\n", System.resetReason());
    // SF_OSAL_printf("Starting Device: %s\n", pSystemDesc->deviceID);

    // SF_OSAL_printf("Battery = %lf %%\n", pSystemDesc->pBattery->getSoC());
    // SF_OSAL_printf("Battery = %lf V\n", pSystemDesc->pBattery->getVCell());

    SF_OSAL_printf("Time.now() = %ld\n", Time.now());

    RESET_GOOD = 0xAA55;
    nRESET_GOOD = ~RESET_GOOD;
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
  return NULL;
}