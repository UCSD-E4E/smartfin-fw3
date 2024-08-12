#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
#include "cli/conio.hpp"
#include "scheduler.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <memory>
#include <dirent.h>
#include <algorithm>
#include <utility>
#include <string.h>
#include <chrono>


class ExamineBehavior
{
    public:
    
    static std::unique_ptr<FileWriter> files;
    static void SetUpTestSuite()
    {
    
        #if SCHEDULER_VERSION == CHARLIE_VERSION 
        files = std::make_unique<FileWriter>(
                "/dev/null",
                "tests/no_check_outputs/charlie_actual_file_tests.log");
        SF_OSAL_printf("Using Charlie's Version\n");
        #elif SCHEDULER_VERSION == ANTARA_VERSION
        files = std::make_unique<FileWriter>(
                "/dev/null",
                "tests/no_check_outputs/antara_actual_file_tests.log");
                SF_OSAL_printf("Using Antara's Version\n");
        #else
        files = std::make_unique<FileWriter>(
                "/dev/null",
                "tests/no_check_outputs/consolodated_actual_file_tests.log");
                SF_OSAL_printf("Using consolodated version\n");
        #endif

    }
    static void TearDownTestSuite()
    {
        files->closeFiles();
    }

    //! holds the deployment schedule created in @ref SchedulerTest()
    std::vector<DeploymentSchedule_t> deploymentSchedule;
    //! pointer for next event in deploymentSchedule
    DeploymentSchedule_t* nextEvent;
    //! next time any event will be run
    uint32_t nextEventTime;
    //! holds actual values for testing
    std::vector<TestLog> actual;
    
    
    
    //! output file for actual values
    std::ofstream actualFile;

    TestInput input;
    
    
    //! holds test name across functions
    std::string testName;


    //! for writing test output
    bool useCompareLogs;
    std::unique_ptr<Scheduler> scheduler;
    std::vector<std::pair<std::string,uint32_t>> exceededDelays;



    /**
    * @brief Construct a new Scheduler Test object
    *
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 2000ms with a max duration of 400ms
    * Ensemble B runs every 2000ms with a max duration of 200ms
    * Ensemble C runs every 2000ms with a max duration of 600ms
    */

    
    /**
     * @brief Add actual log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendActualFile(std::string task,
                                    uint32_t start,
                                    uint32_t end)
    {
        actual.emplace_back(task, start, end);
    }
    /**
     * @brief Sets up the test envirnoment before each test
     *
     */
    void SetUp(std::string filename)
    {
        actual.clear();
        

        
        testName = filename;
        
        
        nextEvent = nullptr; // ensures that first call to scheduler is correct
        nextEventTime = 0; // time handling

        setTime(0);

    }

    /**
     * @brief Cleans the test envirnoment after each test
     *
     */
    void TearDown()
    {
        
        size_t pos = testName.find_last_of("/");
        
        testName = testName.substr(pos + 1);
        
        std::string::size_type const p(testName.find_last_of('.'));
        
        
        std::string baseName = testName.substr(0, p);
       
        files->writeTest(baseName, actual,actual);
    }
    void runTestFile(std::string filename)
    {
        input.clear();
        input = parseInputFile(filename);

        setTime(input.start);
        deploymentSchedule.clear();
        DeploymentSchedule_t e;
        for (size_t i = 0; i < input.ensembles.size(); i++)
        {
            #if SCHEDULER_VERSION == CHARLIE_VERSION
            e = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0,
                                            input.ensembles[i].interval,
                                            input.ensembles[i].duration,
                                            input.ensembles[i].delay,
                                            input.ensembles[i].taskName.c_str(),
                                            {0} };
            #elif SCHEDULER_VERSION == ANTARA_VERSION
            e = {SS_ensembleAFunc, SS_ensembleAInit, 0, 
                        input.ensembles[i].interval, 
                        input.ensembles[i].duration, 
                        input.ensembles[i].taskName.c_str(),
                        UINT32_MAX, 0,  0};
            #else
            e = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0,
                                            input.ensembles[i].interval,
                                            input.ensembles[i].duration,
                                            input.ensembles[i].delay,
                                            input.ensembles[i].taskName.c_str(),
                                            {0} };
            #endif 
            deploymentSchedule.emplace_back(e);
        }
        #if SCHEDULER_VERSION == CHARLIE_VERSION
        e = { nullptr, nullptr, 0, 0, 0, 0, 0, "",{0} };
        deploymentSchedule.emplace_back(e);
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        #elif SCHEDULER_VERSION == ANTARA_VERSION
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data(),
            deploymentSchedule.size());
        #else
        e = { nullptr, nullptr, 0, 0, 0, 0, 0, "",{0} };
        deploymentSchedule.emplace_back(e);
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        #endif
        
        
        scheduler->initializeScheduler();

        while (millis() < input.end)
        {
            scheduler->getNextTask(&nextEvent, &nextEventTime,
                                    millis());
            
            uint32_t afterDelay = input.getDelay(nextEvent->taskName,
                                        nextEvent->state.measurementCount - 1);
            
            
            runAndCheckEventWithDelays(afterDelay);

        }

    }



    TestInput parseInputFile(std::string filename)
    {
        std::ifstream inputFile(filename);
        std::ifstream file(filename);
        std::string line;
        std::string currentSection;
        int start = 0;
        int end;




        TestInput out;
        while (getline(file, line))
        {

            size_t commentPos = line.find('#');
            if (commentPos != std::string::npos)
            {
                line = line.substr(0, commentPos);
            }

            // Remove leading and trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty()) continue;

            if (line == "START")
            {
                currentSection = "START";
                continue;
            }
            else if (line == "END")
            {
                currentSection = "END";
                continue;
            }
            else if (line == "ENSEMBLES")
            {
                currentSection = "ENSEMBLES";
                continue;
            }
            else if (line == "DELAYS")
            {
                currentSection = "DELAYS";
                continue;
            }
            
            else if (line == "RESETS")
            {
                currentSection = "RESETS";
                continue;
            }

            std::istringstream iss(line);
            if (currentSection == "START")
            {
                iss >> out.start;
            }
            else if (currentSection == "END")
            {
                iss >> out.end;
            }
            else if (currentSection == "ENSEMBLES")
            {

                std::string name;

                uint32_t interval, duration, maxDelay;
                std::getline(iss, name, '|');

                iss >> interval;


                iss.ignore(1, '|');
                iss >> duration;

                char checkChar = iss.peek();
                if (checkChar == '|')
                {
                    iss.ignore(1, '|');
                    iss >> maxDelay;
                }
                else
                {
                    maxDelay = interval * 2;
                }
                EnsembleInput ensembleInput(name, interval, duration, maxDelay);
                out.ensembles.emplace_back(ensembleInput);

            }
            else if (currentSection == "DELAYS")
            {
                
                std::string delayName;
                std::getline(iss, delayName, '|');
                std::string taskName = delayName.c_str();
                uint32_t iteration, delay;
                iss >> iteration;
                iss.ignore(1, '|');
                iss >> delay;
                
                if (out.delays.find(taskName) != out.delays.end())
                {
                    out.delays[taskName] = std::unordered_map<uint32_t,
                                                                uint32_t>();
                }
                out.delays[taskName][iteration] = delay;
                
            }
            else if (currentSection == "RESETS")
            {
                std::string resetName;
                uint32_t iteration;
                std::getline(iss, resetName, '|');
                iss >> iteration;
                out.resets.emplace_back(std::make_pair(resetName, iteration));
            }
        }


        return out;
    }





    void runAndCheckEventWithDelays(uint32_t trailingDelay)
    {


        setTime(nextEventTime);

        uint32_t actualStart = millis();


        addTime(nextEvent->maxDuration);
        addTime(trailingDelay);
        uint32_t actualEnd = millis();


        appendActualFile(nextEvent->taskName, actualStart, actualEnd);

    }
    std::vector<std::string> GetFilesInDirectory(const std::string& directory) {
        std::vector<std::string> filesInDir;
        struct dirent *dp; 
        DIR* dirp = opendir(directory.c_str());
        
       
        std::cout << "directory: " <<  directory << "\n";
    
        while ((dp = readdir(dirp)) != NULL) {
            if (dp->d_type == DT_REG) {
                std::string file = directory + std::string(dp->d_name);
                
                filesInDir.push_back(file);
            }
        }
        std::cout << "closing " <<  directory << "\n";
        closedir(dirp);
        std::cout << directory << " closed" << "\n";
        return filesInDir;
    }
    void runTests()
    {
        SetUpTestSuite();
        std::vector<std::string> filesInDir = GetFilesInDirectory(
                "./tests/no_check_inputs/");
        int i = 0;
        for (const auto& file : filesInDir)
        {
            SetUp(file);
            runTestFile(file);
            TearDown();
            SF_OSAL_printf("\rCompleted %d/%d\t\t\t\t",++i,filesInDir.size());
        }
        TearDownTestSuite();
    }

};
std::unique_ptr<FileWriter> ExamineBehavior::files = nullptr;


int main(int argc, char const* argv[])
{

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


    ExamineBehavior e;
    e.runTests();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time to run = " 
    << 
    std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
    << "[µs]" << std::endl;


    
    return 0;
}
