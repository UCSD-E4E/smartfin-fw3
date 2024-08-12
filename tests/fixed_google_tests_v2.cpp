#include <gtest/gtest.h>
#include <iostream>
#include <vector>
/**
 * name of test class: ClassName 
*/

class ClassName : public ::testing::Test
{
//required
protected:
    std::vector<int> test;

    ClassName()
    {
        test.reserve(5);
        test.emplace_back(5);
    }
    void SetUp() override{
        test.emplace_back(10);
    }
    void TearDown() override{
        for (int i = 0; i < get_size(); i++)
        {
            std::cout << test[i] << "\t";
        }
        std::cout << "\n";

    }


    int get_size()
    {
        return test.size();
    }

    
};

TEST_F(ClassName, Test1)
{
    test.emplace_back(20);
    ASSERT_EQ(test[1], 20);
    
};
