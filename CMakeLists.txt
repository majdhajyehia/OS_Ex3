cmake_minimum_required(VERSION 3.16)
project(OS_Ex3)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

set(CMAKE_C_FLAGS -pthread)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(OS_Ex3
        Job.cpp
        Job.h
        MapReduceClient.h
        MapReduceFramework.cpp
        MapReduceFramework.h
        main.cpp)
