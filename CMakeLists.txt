cmake_minimum_required(VERSION 3.14)
project(TestScheduler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT DEFINED SCRIPT_BUILD)
    set(SCRIPT_BUILD OFF CACHE BOOL "Build from script" FORCE)
endif()


add_compile_definitions(TEST_VERSION=1 GTESTING=1)

if(SCRIPT_BUILD)
    add_compile_definitions(SCRIPT_BUILD=1)
endif()

add_subdirectory(external/googletest tests/binary_dir)
include_directories(external/googletest/include src/ src/cli/ tests)

set(GTEST_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    src/scheduler.cpp
    tests/gtest.cpp)

set(TEST_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    src/scheduler.cpp
    src/ensembles.cpp
    tests/scheduler_test.cpp
)

add_executable(googletests ${GTEST_SOURCE_FILES})
target_link_libraries(googletests gtest gtest_main pthread)

add_executable(test ${TEST_SOURCE_FILES})
