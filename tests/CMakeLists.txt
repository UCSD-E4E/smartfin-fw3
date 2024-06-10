cmake_minimum_required(VERSION 3.14)
project(TestScheduler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(TEST_VERSION=1 GTESTING=1)

add_subdirectory(../external/googletest ./binary_dir)
include_directories(../external/googletest/include  ../src/ ../src/cli/ ../test)

set(GTEST_SOURCE_FILES
    scheduler_test_system.cpp
    ../src/scheduler.cpp
    scheduler_functions.cpp
    gtest.cpp)

set(TEST_SOURCE_FILES
    scheduler_test_system.cpp
    ../src/scheduler.cpp
    ../src/ensembles.cpp
    scheduler_test.cpp
)

add_executable(googletests ${GTEST_SOURCE_FILES})
target_link_libraries(googletests gtest gtest_main pthread)

add_executable(test ${TEST_SOURCE_FILES})
