cmake_minimum_required(VERSION 3.14)
project(smartfin_pc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
)
add_executable(smartfin_pc ${SMARTFIN_SOURCE_FILES})
