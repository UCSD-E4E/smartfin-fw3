#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
#include "cli/conio.hpp"
#include "scheduler.hpp"
#include "test_file_system.hpp"

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
    //! output file for actual values
    std::ofstream actualFile;
    //! holds all data input from file
    TestInput input;
    
    //! holds test name across functions
    std::string testName;

    //! for writing test output
    bool useCompareLogs;
    std::unique_ptr<Scheduler> scheduler;
    std::vector<std::pair<std::string,std::uint32_t>> exceededDelays;
    std::unordered_map<std::string,std::vector<std::string>> shifts;


    static void SetUpTestSuite()
    {
    
        files = std::make_unique<FileWriter>(
                "/dev/null",
                "tests/no_check_outputs/consolodated_actual_file_tests.log");

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
    std::uint32_t nextEventTime;
    //! holds actual values for testing
    std::vector<TestLog> actual;
    


    

    
    /**
     * @brief Add actual log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendActualFile(std::string task,
                                    std::uint32_t start,
                                    std::uint32_t end)
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
       
        files->writeTest(baseName, actual, actual);
    }
    void runTestFile(std::string filename)
    {

        std::cout << "running " << filename << "\n";
        input.clear();
        input.parseInputFile(filename);

        setTime(input.start);
        deploymentSchedule.clear();
        DeploymentSchedule_t ensemble;
        for (size_t i = 0; i < input.ensembles.size(); i++)
        {
            ensemble = { SS_ensembleAFunc, SS_ensembleAInit, 1,
                                            input.ensembles[i].interval,
                                            input.ensembles[i].duration,
                                            input.ensembles[i].delay,
                                            input.ensembles[i].taskName.c_str(),
                                            {0} };
            deploymentSchedule.emplace_back(ensemble);
        }
        ensemble = { nullptr, nullptr, 0, 0, 0, 0, "", {0}};
        deploymentSchedule.emplace_back(ensemble);
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        

     
    
    
        scheduler->initializeScheduler();
    
        while (millis() < input.end)
        {
        
            scheduler->getNextTask(&nextEvent, &nextEventTime, millis());

        
            std::uint32_t afterDelay = input.getDelay(nextEvent->taskName,
                                    nextEvent->state.measurementCount - 1);
        
            runEventWithDelays(afterDelay);
        }

        std::string delimiter = "|";
        std::ofstream sch_log("scheduler.log");
        for (const auto& log = scheduler->log.front(); 
                scheduler->log.size() > 0; scheduler->log.pop())
        {
            sch_log << std::get<0>(log) << "|" << std::get<1>(log) << "\n";
        }
        
        
        
        
        
    }
    
    





    void runEventWithDelays(std::uint32_t trailingDelay)
    {


        setTime(nextEventTime);

        std::uint32_t actualStart = millis();


        addTime(nextEvent->maxDuration);
        addTime(trailingDelay);
        std::uint32_t actualEnd = millis();


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
            SF_OSAL_printf("Completed %d/%d\t\t\t\t\r",++i,filesInDir.size());
        }
        TearDownTestSuite();
    }

};
std::unique_ptr<FileWriter> ExamineBehavior::files = nullptr;


int main(int argc, char const* argv[])
{

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


    ExamineBehavior examine;
    examine.runTests();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time to run = " 
    << 
    std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
    << "[µs]" << std::endl;


    
    return 0;
}
