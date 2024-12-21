/**
 * @file main.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Smartfin CMake Main File
 * @version 0.1
 * @date 2024-11-08
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "cli/conio.hpp"
#include "product.hpp"
#include "signal.h"

void setup();
void loop();

#include "smartfin-fw3.ino"

#if SF_PLATFORM == SF_PLATFORM_GLIBC
#include "ncurses.h"
void sigint_handler(int _)
{
    (void)_;
    SF_OSAL_deinit_conio();
    exit(1);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, sigint_handler);
    setup();
    while (true)
    {
        loop();
    }
    SF_OSAL_deinit_conio();
    return 0;
}

#endif