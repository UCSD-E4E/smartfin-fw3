cmake_minimum_required(VERSION 3.14)
project(smartfin_unoq)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(
    .
    ../
    ../src/
    ../pc_hal/
    ../lib/tracker-edge/src
    ../lib/gps-ublox/src
    ../lib/gps-nmea-parser/src
    ../lib/gps-quectel/src
)

add_definitions(-DSF_PLATFORM=SF_PLATFORM_UNOQ)

set(SMARTFIN_SOURCE_FILES
    main.cpp
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
    ../src/cli/cliDebug.cpp
    ../src/cli/menuItems/gpsCommands.cpp
    ../src/debug/recorder_debug.cpp
    ../src/debug/session_debug.cpp
    ../src/fileCLI/fileCLI.cpp
    ../src/rideTask.cpp
    ../src/deploy/scheduler.cpp
    ../src/deploy/ensembles.cpp
    ../src/deploy/ensembleTypes.cpp
    ../src/deploy/ensemble_commit.cpp
    ../src/ble/ble_live_stream.cpp
    ../src/ble/ble_packet_builder.cpp
    ../src/ble/sf_ble.cpp
    ../src/ble/high_rate_stream.cpp
    ../src/platform/unoq/gpio.cpp
    ../src/platform/unoq/i2c.cpp
    ../src/platform/unoq/timing.cpp
    ../src/platform/unoq/serial.cpp
    ../src/platform/unoq/nvm.cpp
    ../src/platform/unoq/cloud.cpp
    ../src/platform/unoq/ble.cpp
    ../src/platform/unoq/system.cpp
    ../src/platform/unoq/thread.cpp
    ../src/platform/unoq/timer.cpp
    ../src/platform/unoq/mutex.cpp
    ../src/platform/unoq/led.cpp
    ../src/platform/unoq/utils.cpp
)
add_executable(smartfin_unoq ${SMARTFIN_SOURCE_FILES})
target_link_libraries(smartfin_unoq
    Threads::Threads
)
