#include <iostream>
#include <unistd.h>

// Smartfin Core Headers
#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "platform/hal.hpp"
#include "platform/unoq/rpc_client.hpp"
#include "system.hpp"

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

    // 4. Run the interactive CLI
    std::cout << "Initialization complete. Starting Smartfin CLI..." << std::endl;

    static CLI cliTask;

    while (true)
    {
        cliTask.run();
    }

    return 0;
}