cmake_minimum_required(VERSION 3.14)
project(TestScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DTEST_VERSION=1 -DGTESTING=1)
include_directories($ENV{HOME}/local/include)
link_directories($ENV{HOME}/local/lib)

set(SOURCE_FILES
    ../tests/scheduler_test_system.cpp
    ../src/scheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler_functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest.cpp)

add_executable(test_scheduler.o ${SOURCE_FILES})
target_link_libraries(test_scheduler.o gtest gtest_main pthread)