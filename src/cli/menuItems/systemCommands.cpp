/**
 * Project smartfin-fw3
 * Description: System CLI commands
 * @file debugCommands.cpp
 * @author Emily Thorpe <ethorpe@macalester.edu>
 * @date Jul 20 2023
 *
 */

#include "systemCommands.hpp"

#include "cli/cli.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "cellular/dataCollection.hpp"
#include "cellular/sf_cloud.hpp"
#include "consts.hpp"
#include "states.hpp"
#include "system.hpp"
#include "vers.hpp"

#include "Particle.h"

void CLI_connect(void)
{

    sf::cloud::wait_connect(30000);
    SF_OSAL_printf("Connected" __NL__);
}

void CLI_disconnect(void)
{
    sf::cloud::wait_disconnect(30000);
    SF_OSAL_printf("Disconnected" __NL__);
}

void CLI_doSleep(void)
{
    CLI_nextState = STATE_DEEP_SLEEP;
    SF_OSAL_printf("Quit the menu to set the next state" __NL__);
}

void CLI_doUpload(void)
{
    char integer_string[64];
    int integer = Time.now();

    char other_string[64] = "Session: ";
    sprintf(integer_string, "%d", integer);

    strcat(other_string, integer_string);

    int success = sf::cloud::publish_blob(other_string, "Particle was here!");
    SF_OSAL_printf("Particle publish: %d" __NL__, success);
}

void CLI_self_identify(void)
{
    SF_OSAL_printf("Smartfin ID: %s\n", pSystemDesc->deviceID);
    VERS_printBanner();
}

