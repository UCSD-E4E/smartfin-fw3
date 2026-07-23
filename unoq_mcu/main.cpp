/**
 * @file main.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Entry point for the STM32U585 Uno Q peripheral-server firmware.
 * @date 2026-07-23
 *
 * Initialises the SPI transport and peripheral server, then services
 * msgpack-rpc calls from the QRB2210 Linux side forever. Contains no
 * application/business logic; see src/platform/unoq/ for the Linux-side
 * HAL client that talks to this firmware.
 */
#include "peripheral_server.hpp"
#include "transport.hpp"

/**
 * @brief Firmware entry point.
 *
 * @return Never returns.
 */
int main()
{
    sf_mcu::transport_init();
    sf_mcu::peripheral_server_init();

    for (;;)
    {
        sf_mcu::peripheral_server_run();
    }
}
