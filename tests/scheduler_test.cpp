#include "scheduler_test_system.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>

static void SS_ensembleAInit(DeploymentSchedule_t* pDeployment) {return;}
static void SS_ensembleAFunc(DeploymentSchedule_t* pDeployment) {return;}


static void SS_ensembleBInit(DeploymentSchedule_t* pDeployment) {return;}
static void SS_ensembleBFunc(DeploymentSchedule_t* pDeployment) {return;}

static void SS_ensembleCInit(DeploymentSchedule_t* pDeployment) {return;}
static void SS_ensembleCFunc(DeploymentSchedule_t* pDeployment) {return;}


int main(int argc, char* argv[])
{
    int cycles = 15;

    if (argc >= 2)

    {
        cycles = std::stoi(argv[1]);
    }

    std::vector<uint32_t> delays(cycles, 0);

    for (int i = 2; i < argc - 1; i += 2)
    {
        delays[std::stoi(argv[i])] = delays[std::stoi(argv[i + 1])];
        int index = std::stoi(argv[i]);
        int value = std::stoi(argv[i + 1]);
        if (index >= 0 && index < delays.size())
        {
            delays[index] = value;
        }
        else
        {
            std::cerr << "Index out of bounds: " << index << "\n";
        }
    }


    setTime(0);
    DeploymentSchedule_t deploymentSchedule[] =
    {{SS_ensembleAFunc, SS_ensembleAInit, 1, millis(), 2000, UINT32_MAX,
                                        0, 0, 0, nullptr,400,(char)65},
    {SS_ensembleBFunc, SS_ensembleBInit, 1, millis(), 2000, UINT32_MAX, 
                                        0, 0, 0, nullptr,200,(char)66},
    {SS_ensembleCFunc, SS_ensembleCInit, 1, millis(), 2000, UINT32_MAX, 
                                        0, 0, 0, nullptr,600,(char)67},
    {nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr}};

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
            
            std::cout << "|" << millis();

            //currentTime += ;
            addTime(nextEvent->maxDuration);
            if (delays[counter] != 0)
            {
                //currentTime += delays[counter];
                addTime(delays[counter]);
            }

            std::cout << "|" << millis() << "\n";

            if (counter < 3)
            {
                //currentTime += 100;
                addTime(100);
            }
        }
        counter++;
    }

    return 0;
}
