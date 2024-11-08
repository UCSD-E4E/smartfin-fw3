cmake_minimum_required(VERSION 3.14)
project(smartfin_pc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../src/ .)

set(SMARTFIN_SOURCE_FILES
    main.cpp
)
add_executable(smartfin_pc ${SMARTFIN_SOURCE_FILES})
