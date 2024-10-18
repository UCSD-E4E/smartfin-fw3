cmake_minimum_required(VERSION 3.14)
project(SmartfinTests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_compile_definitions(TEST_VERSION)

add_subdirectory(external/googletest )
include_directories(external/googletest/googletest/include src/ tests)

set(GTEST_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    tests/fixed_google_tests.cpp
    tests/file_google_tests.cpp
    src/scheduler.cpp
    tests/scheduler_test_flog.cpp
    tests/test_file_system.cpp
)
set(EXAMINE_BEHAVIOR_SOURCE_FILES
    tests/scheduler_test_system.cpp
    tests/test_ensembles.cpp
    src/scheduler.cpp
    tests/scheduler_test_flog.cpp
    tests/examine_behavior.cpp
    tests/test_file_system.cpp
)

add_executable(googletests ${GTEST_SOURCE_FILES})
target_link_libraries(googletests gtest gtest_main pthread)

add_executable(examine_behavior ${EXAMINE_BEHAVIOR_SOURCE_FILES})
