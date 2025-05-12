cmake_minimum_required(VERSION 3.14)
project(Smartfin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/googletest)
include_directories(
    src/ 
    tests/ 
    pc_hal/
    lib/tracker-edge/src/
    external/googletest/googletest/include
)

set(GTEST_SOURCE_FILES
    tests/test_endianness.cpp
    tests/test_coreFiles.cpp
)

set(DUMMY_HAL_SOURCES
    pc_hal/Particle_dummy.cpp
)

set(CORE_SOURCE_FILES
    src/system.cpp
    src/sys/NVRAM.cpp
    src/sys/led.cpp
    src/cellular/recorder.cpp
    src/watersensor/waterSensor.cpp
    src/temperature/tmpSensor.cpp
    lib/tracker-edge/src/location_service.cpp
    src/temperature/max317275_cpp.cpp
    src/cli/conio.cpp
    src/cli/flog.cpp
    lib//gps-ublox/src/ubloxGPS.cpp
)

add_executable(googletests
    ${GTEST_SOURCE_FILES}
    ${DUMMY_HAL_SOURCES}
    ${CORE_SOURCE_FILES}
)

target_compile_definitions(googletests PRIVATE SF_PLATFORM=SF_PLATFORM_GOOGLETEST)

target_link_libraries(googletests gtest gtest_main pthread)

option(BUILD_PC_HAL "Build the PC HAL executable" OFF)

if (BUILD_PC_HAL)
    add_subdirectory(pc_hal)
endif()

enable_testing()
add_test(NAME googletests COMMAND googletests)

add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/googletests
    DEPENDS googletests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Google Tests"
)
