cmake_minimum_required(VERSION 2.8.0)
project(main)

SET(CMAKE_SYSTEM_NAME Linux)

SET(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
SET(CMAKE_SYSTEM_PROCESSOR "arm")
set(CMAKE_CXX_FLAGS "-pthread -g -Wall")
# add_compile_options(-pthread)

set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp)