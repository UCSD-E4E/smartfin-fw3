/** @todo Add documentation*/
#include "scheduler.hpp"
#include "consts.hpp"
#include "conio.hpp"
#include "scheduler_test_system.hpp"
#include <unistd.h>
#include <vector>
#include <inttypes.h>
#include <cstdlib>
#include "test_ensembles.hpp"




int main(int argc, char* argv[])
{
    int cycles = 15;

    if (argc >= 2)

    {
        cycles = atoi(argv[1]);
    }

    std::vector<uint32_t> delays(cycles, 0);

    for (int i = 2; i < argc - 1; i += 2)
    {
        delays[atoi(argv[i])] = delays[atoi(argv[i + 1])];
        int index = atoi(argv[i]);
        int value = atoi(argv[i + 1]);
        if (index >= 0 && index < delays.size())
        {
            delays[index] = value;
        }
    }


    setTime(0);
    DeploymentSchedule_t deploymentSchedule2[] =
    { {SS_ensembleAFunc, SS_ensembleAInit, 1, millis(), 2000, UINT32_MAX,
                                        0, 0, 0, nullptr,400,(char)65},
    {SS_ensembleBFunc, SS_ensembleBInit, 1, millis(), 2000, UINT32_MAX,
                                        0, 0, 0, nullptr,200,(char)66},
    {SS_ensembleCFunc, SS_ensembleCInit, 1, millis(), 2000, UINT32_MAX,
                                        0, 0, 0, nullptr,600,(char)67},
    {nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr} };

    DeploymentSchedule_t deploymentSchedule[] =
    { 
    {SS_ensembleAFunc, SS_ensembleAInit, 1, millis(), 500, UINT32_MAX, 0, 0, 0, nullptr, 200, 'A'},
    {SS_ensembleBFunc, SS_ensembleBInit, 1, millis(), 200, UINT32_MAX, 0, 0, 0, nullptr, 100, 'B'},
    {SS_ensembleCFunc, SS_ensembleCInit, 1, millis(), 800, UINT32_MAX, 0, 0, 0, nullptr, 250, 'C'},
    {nullptr,          nullptr,          0, 0,        0,   0,          0, 0, 0, nullptr, 0,   '\0'} };

    SCH_initializeSchedule(deploymentSchedule, millis());

    DeploymentSchedule_t* nextEvent = nullptr;
    uint32_t nextEventTime = 0;
    uint32_t counter = 0;

    while (counter < cycles)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        if (nextEvent != nullptr)
        {

            setTime(nextEventTime);

            SF_OSAL_printf("|%" PRIu32"|", millis());


            addTime(nextEvent->maxDuration);

            SF_OSAL_printf("%" PRIu32"|" __NL__, millis());



        }
        counter++;
    }

    return 0;
}
