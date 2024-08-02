cmake_minimum_required(VERSION 3.14)
project(TestScheduler)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



if(NOT DEFINED SCRIPT_BUILD)
    set(SCRIPT_BUILD OFF CACHE BOOL "Build from script" FORCE)
endif()

add_compile_definitions(TEST_VERSION=1)
if(SCRIPT_BUILD)
    add_compile_definitions(SCRIPT_BUILD=1)
endif()

add_subdirectory(external/googletest )
include_directories(../external/googletest/googletest/include src/ tests)

set(GTEST_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    src/antara_scheduler.cpp
    src/charlie_scheduler.cpp
    tests/fixed_google_tests.cpp
    tests/file_google_tests.cpp
    #tests/overlap_google_tests.cpp
    tests/scheduler_test_flog.cpp
)
set(EXAMINE_BEHAVIOR_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    src/charlie_scheduler.cpp
    src/antara_scheduler.cpp
    tests/scheduler_test_flog.cpp
    tests/examine_behavior.cpp
)

add_executable(googletests ${GTEST_SOURCE_FILES})
target_link_libraries(googletests gtest gtest_main pthread)

add_executable(examine_behavior ${EXAMINE_BEHAVIOR_SOURCE_FILES})
