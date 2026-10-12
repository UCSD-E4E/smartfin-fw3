#include <iostream>
#include <unistd.h>

// Smartfin Core Headers
#include "cellular/dataUpload.hpp"
#include "chargeTask.hpp"
#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "platform/hal.hpp"
#include "platform/unoq/rpc_client.hpp"
#include "rideTask.hpp"
#include "sleepTask.hpp"
#include "states.hpp"
#include "system.hpp"

typedef struct StateMachine_
{
    STATES_e state;
    Task *task;
} StateMachine_t;

static CLI cliTask;
static ChargeTask chargeTask;
static SleepTask sleepTask;
static DataUpload uploadTask;
static RideTask rideTask;

static StateMachine_t stateMachine[] = {
    {STATE_CLI, &cliTask},
    {STATE_DEEP_SLEEP, &sleepTask},
    {STATE_CHARGE, &chargeTask},
    {STATE_UPLOAD, &uploadTask},
    {STATE_DEPLOYED, &rideTask},
    {STATE_NULL, NULL}
};

static StateMachine_t *findState(STATES_e state)
{
    for (StateMachine_t *pStates = stateMachine; pStates->task; pStates++)
    {
        if (pStates->state == state)
        {
            return pStates;
        }
    }
    return NULL;
}

int main()
{
    std::cout << "Starting Smartfin Uno Q HAL..." << std::endl;

    // 1. Establish the SPI bridge to the STM32
    std::cout << "Starting RPC Client..." << std::endl;
    sf_unoq::rpc_client_init();

    // 2. Run the primary system initialization sequence
    std::cout << "Executing SYS_initSys()..." << std::endl;
    SYS_initSys();

    // 3. Run the delayed initialization for the IMU and secondary sensors
    std::cout << "Executing SYS_delayedInitSys()..." << std::endl;
    SYS_delayedInitSys();

    // Request live hardware data from the STM32
    std::cout << "\n--- FETCHING STM32 HARDWARE STATE OVER SPI ---" << std::endl;
    SYS_displaySys();
    SYS_dumpSys(0);
    std::cout << "----------------------------------------------\n" << std::endl;

    // 4. Run the Smartfin State Machine
    std::cout << "Initialization complete. Starting Smartfin State Machine..." << std::endl;

    currentState = STATE_CLI;
    CLI_nextState = STATE_CHARGE;

    while (true)
    {
        StateMachine_t *pState = findState(currentState);
        if (pState == NULL)
        {
            std::cout << "State not found (" << currentState << "), defaulting to CLI" << std::endl;
            currentState = STATE_CLI;
            pState = findState(currentState);
        }

        std::cout << "\nStarting state: "
                  << ((currentState < STATE_N_STATES && currentState >= 0) ? STATES_NAME_TAB[currentState] : "UNKNOWN")
                  << " (" << currentState << ")" << std::endl;

        pState->task->init();
        currentState = pState->task->run();
        pState->task->exit();
    }

    return 0;
}