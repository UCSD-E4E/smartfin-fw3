cmake_minimum_required(VERSION 3.14)
project(smartfin_pc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(
    .    
    ../src/
    ../lib/tracker-edge/src
    ../lib/gps-ublox/src
    ../lib/gps-nmea-parser/src
    ../lib/gps-quectel/src
)

set(SMARTFIN_SOURCE_FILES
    main.cpp
    Particle_dummy.cpp
    ../src/cli/flog.cpp
    ../src/cli/conio.cpp
    ../src/system.cpp
    ../src/cellular/sf_cloud.cpp
    ../src/sleepTask.cpp
    ../src/cli/cli.cpp
    ../src/states.cpp
    ../src/mfgTest/mfgTest.cpp
    ../src/chargeTask.cpp
    ../src/cellular/dataUpload.cpp
    ../src/sys/NVRAM.cpp
    ../src/cellular/recorder.cpp
    ../src/sys/led.cpp
    ../src/watersensor/waterSensor.cpp
    ../src/temperature/max317275_cpp.cpp
    ../src/temperature/tmpSensor.cpp
    ../src/vers.cpp
    ../src/cellular/encoding/base64.c
    ../src/cli/menu.cpp
    ../src/cellular/deploy.cpp
    ../src/i2c/i2c.cpp
    ../src/util.cpp
    ../src/cli/menuItems/systemCommands.cpp
    ../src/cli/menuItems/debugCommands.cpp
    ../src/imu/imu.cpp
    ../src/cli/cliDebug.cpp
    ../src/cli/menuItems/gpsCommands.cpp
    ../src/debug/recorder_debug.cpp
    ../src/debug/session_debug.cpp
    ../src/fileCLI/fileCLI.cpp
)
add_executable(smartfin_pc ${SMARTFIN_SOURCE_FILES})
target_link_libraries(smartfin_pc Threads::Threads)
