#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
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
        files = std::make_unique<FileWriter>(
                "/dev/null",
                "tests/no_check_outputs/actual_file_tests.log");

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
    //! filename for writing actual test output
    std::string actualFileName = "actual_file_tests.log";
    //! holds test name across functions
    std::string testName;


    //! for writing test output
    bool useCompareLogs;
    std::unique_ptr<Scheduler> scheduler;
    



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
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
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
        std::cout << testName << "\n";
        testName = testName.substr(pos + 1);
        std::cout << testName << "\n";
        std::string::size_type const p(testName.find_last_of('.'));
        std::cout << testName << "\n";
        std::cout << p << "\n";
        std::string baseName = testName.substr(0, p);
        std::cout << baseName << "\n";
        files->writeTest(baseName, actual,actual);
    }
    void runTestFile(std::string filename)
    {
        input = parseInputFile(filename);

        setTime(input.start);
        deploymentSchedule.clear();
        DeploymentSchedule_t e;
        for (size_t i = 0; i < input.ensembles.size(); i++)
        {
            e = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0,
                                            input.ensembles[i].interval,
                                            input.ensembles[i].duration,
                                            input.ensembles[i].delay,
                                            input.ensembles[i].taskName.c_str(),
                                            {0} };
            deploymentSchedule.emplace_back(e);
        }
        e = { nullptr, nullptr, 0, 0, 0, 0, 0, "",{0} };
        deploymentSchedule.emplace_back(e);
        
        scheduler->initializeScheduler();

        while (millis() < input.end)
        {
            scheduler->getNextTask(nextEvent, &nextEventTime,
                                    millis());
            uint32_t beforeDelay = 0;
            uint32_t afterDelay = 0;
            for (size_t i = 0; i < input.delays.size(); i++)
            {
                if (!strcmp(nextEvent->taskName,
                    input.delays[i].taskName.c_str()) &&
                        (nextEvent->state.measurementCount - 1 ==
                            input.delays[i].iteration))
                {
                    if (input.delays[i].isBefore)
                    {
                        beforeDelay = input.delays[i].delay;
                    }
                    else
                    {
                        afterDelay = input.delays[i].delay;
                    }
                    input.delays.erase(input.delays.begin() + i);
                    if ((beforeDelay != 0) && (afterDelay != 0))
                        break;
                }
            }
            runAndCheckEventWithDelays(beforeDelay, afterDelay);

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
                Delay d;
                std::string delayName;
                std::getline(iss, delayName, '|');
                d.taskName = delayName.c_str();
                iss >> d.iteration;
                iss.ignore(1, '|');
                iss >> d.delay;
                d.isBefore = false;
                char checkChar = iss.peek();
                if (checkChar == '|')
                {
                    iss.ignore(1, '|');
                    std::string position;
                    iss >> position;
                    if (position == "before") d.isBefore = true;
                }
                out.delays.emplace_back(d);
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





    void runAndCheckEventWithDelays(uint32_t preceedingDelay,
                                    uint32_t trailingDelay)
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
        DIR* dirp = opendir(directory.c_str());
        struct dirent* dp;
        while ((dp = readdir(dirp)) != nullptr) {
            if (dp->d_type == DT_REG) {  
                filesInDir.push_back(directory + "/" + std::string(dp->d_name));
            }
        }
        closedir(dirp);
        return filesInDir;
    }
    void runTests()
    {
        SetUpTestSuite();
        std::vector<std::string> filesInDir = GetFilesInDirectory("no_check_inputs/");
        for (const auto& file : filesInDir)
        {
            SetUp(file);
            runTestFile(file);
            TearDown();

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
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;


    
    return 0;
}
