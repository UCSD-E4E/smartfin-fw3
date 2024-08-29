#include "test_file_system.hpp"

#include <vector>

/**
 * @brief comparator for google tests EXPECT_TRUE
 *
 * @param rhs other TestLog to compare to
 * @return true if the same
 * @return false otherwise
 */
bool TestLog::operator==(const TestLog& rhs)
{
    return  (name == rhs.name) &&
        (start == rhs.start) &&
        (end == rhs.end);
};
std::ostream& operator<<(std::ostream& strm, const TestLog& value) {
    return strm << "\"" << value.name << "|"
        << value.start << "|" << value.end << "\"";
}

std::uint32_t TestInput::getDelay(std::string name, std::uint32_t iteration)
{
    std::uint32_t delay = 0;
    if (delays.find(name) != delays.end())
    {
        if (delays[name].find(iteration) != delays[name].end())
        {
            delay = delays[name][iteration];
        }
    }
    return delay;
}
FileWriter::FileWriter(std::string expectedFileName, std::string actualFileName)
{
    this->firstTest = true;
    this->expectedFileName = expectedFileName;
    this->actualFileName = actualFileName;
    std::ofstream expectedFile(expectedFileName, std::ios::out |
    std::ios::trunc);

    std::ofstream actualFile(actualFileName, std::ios::out |
        std::ios::trunc);

    if (actualFile.is_open() && expectedFile.is_open())
    {
        actualFile << "{";
        expectedFile << "{";
    }
    actualFile.close();
    expectedFile.close();
}
void FileWriter::writeTest(std::string testName,
    std::vector<TestLog> expected, std::vector<TestLog> actual)
{
    std::ofstream actualFile(actualFileName, std::ios::out |
                                std::ios::app);
    std::ofstream expectedFile(expectedFileName, std::ios::out |
        std::ios::app);

    if (firstTest == false)
    {
        expectedFile << ",";
        actualFile << ",";
    }
    else
    {
        firstTest = false;
    }
    expectedFile << "\n\t\"" << testName << "\": [";
    if (!expected.empty())
    {


        for (int i = 0; i < expected.size() - 1; i++)
        {
            expectedFile << expected[i] << ", ";
        }
        expectedFile << expected.back();
    }
    expectedFile << "]";

    expectedFile.close();


    actualFile << "\n\t\"" << testName << "\": [";
    if (!actual.empty())
    {


        for (int i = 0; i < actual.size() - 1; i++)
        {
            actualFile << actual[i] << ", ";
        }
        actualFile << actual.back();
    }
    actualFile << "]";
    actualFile.close();
}
void FileWriter::closeFiles()
{
    std::ofstream actualFile(actualFileName, std::ios::out |
                                std::ios::app);
    std::ofstream expectedFile(expectedFileName, std::ios::out |
        std::ios::app);

    if (actualFile.is_open() && expectedFile.is_open())
    {
        actualFile << "\n}";
        expectedFile << "\n}";
    }

    actualFile.close();
    expectedFile.close();
}
EnsembleInput::EnsembleInput(std::string taskName, std::uint32_t interval, std::uint32_t duration, std::uint32_t delay) : taskName(taskName), interval(interval), duration(duration), delay(delay) {};

void TestInput::clear()
{
    ensembles.clear();
    expectedValues.clear();
    delays.clear();
    resets.clear();

}



void TestInput::parseInputFile(std::string filename)
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
        else if (line == "EXPECTED")
        {
            currentSection = "EXPECTED";
            continue;
        }
        

        std::istringstream iss(line);
        if (currentSection == "START")
        {
            iss >> this->start;
        }
        else if (currentSection == "END")
        {
            iss >> this->end;
        }
        else if (currentSection == "ENSEMBLES")
        {

            std::string name;

            std::uint32_t interval, duration, maxDelay;
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
            this->ensembles.emplace_back(ensembleInput);

        }
        else if (currentSection == "DELAYS")
        {
            
            std::string delayName;
            std::getline(iss, delayName, '|');
            std::string taskName = delayName.c_str();
            std::uint32_t iteration, delay;
            iss >> iteration;
            iss.ignore(1, '|');
            iss >> delay;
            
            if (this->delays.find(taskName) != this->delays.end())
            {
                this->delays[taskName] = std::unordered_map<std::uint32_t,
                                                            std::uint32_t>();
            }
            this->delays[taskName][iteration] = delay;
            
        }
        else if (currentSection == "RESETS")
        {
            std::string resetName;
            std::uint32_t iteration;
            std::getline(iss, resetName, '|');
            iss >> iteration;
            resets.emplace_back(std::make_pair(resetName, iteration));
        }
        else if (currentSection == "EXPECTED" )
        {
            std::string name;
            std::uint32_t start;
            std::uint32_t end;
            std::getline(iss, name, '|');
            iss >> start;
            iss.ignore(1, '|');
            iss >> end;
            this->expectedValues.emplace_back(name,start,end);

        }
    }


    
}


void write_shift(std::string ensemble, std::uint32_t idx)
{

    std::ofstream shiftFile("shifts.txt", std::ios::app);
    if (shiftFile.is_open())
    {
        shiftFile << ensemble << "|" << idx << "\n";
        shiftFile.close();
    }
}
Log::Log(DeploymentSchedule_ *task, std::uint32_t time) {
    DeploymentSchedule_ *table = task;
    this->taskName = table->taskName;
    this->runTime = time;
    
}

Log::Log(const char* name, std::uint32_t time) {
    this->taskName = name;
    this->runTime = time;
    
}

const char* Log::getName(void)
{
    return taskName;
}
std::uint32_t Log::getRunTime()
{
    return runTime;
}

bool operator==(const Log& a, const Log& b) {
    return ((a.taskName == b.taskName) && (a.runTime == b.runTime));
}