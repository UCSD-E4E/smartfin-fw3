cmake_minimum_required(VERSION 3.14)
project(TestScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(TEST_VERSION=1 GTESTING=1)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../external/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/googletest/include)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/scheduler_test_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler_functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest.cpp)


add_executable(test_scheduler ${SOURCE_FILES})


target_link_libraries(test_scheduler gtest gtest_main pthread)