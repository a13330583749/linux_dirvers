cmake_minimum_required(VERSION 2.8.0)
project(main)

SET(CMAKE_SYSTEM_NAME Linux)

SET(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
SET(CMAKE_SYSTEM_PROCESSOR "arm")
set(CMAKE_CXX_FLAGS "-pthread -g -Wall")

set(CMAKE_CXX_STANDARD 14)
add_executable(select-ser select-ser.cpp)

SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_SYSTEM_PROCESSOR "x86_64")
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 14)
add_executable(select-cli select-cli.cpp)