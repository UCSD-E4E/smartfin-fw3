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

#include "product.hpp"

void setup();
void loop();

#include "smartfin-fw3.ino"

#if SF_PLATFORM == SF_PLATFORM_GLIBC
int main(int argc, char const *argv[])
{
    setup();
    while (true)
    {
        loop();
    }
    return 0;
}

#endif