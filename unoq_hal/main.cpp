#include <iostream>
#include <unistd.h>

// Smartfin Core Headers
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

    std::cout << "Initialization complete. Entering main OS loop..." << std::endl;

    // 4. Keep the Linux process alive indefinitely so background threads execute
    std::cout << "Executing SYS_delayedInitSys()..." << std::endl;
    SYS_delayedInitSys();

    // Add these two lines to request live hardware data from the STM32
    std::cout << "\n--- FETCHING STM32 HARDWARE STATE OVER SPI ---" << std::endl;
    SYS_displaySys();
    SYS_dumpSys(0);
    std::cout << "----------------------------------------------\n" << std::endl;

    std::cout << "Initialization complete. Entering main OS loop..." << std::endl;

    while (true)
    {
        sleep(1);
    }

    return 0;
}