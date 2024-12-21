cmake_minimum_required(VERSION 3.14)
project(Smartfin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/googletest)
include_directories(external/googletest/googletest/include src/ tests)

set(GTEST_SOURCE_FILES
    tests/test_endianness.cpp
)

add_executable(googletests ${GTEST_SOURCE_FILES})
target_link_libraries(googletests gtest gtest_main pthread)

add_subdirectory(pc_hal)
