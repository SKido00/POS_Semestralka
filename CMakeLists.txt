cmake_minimum_required(VERSION 3.18)
project(POS_Semestralka)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(POS_Semestralka
        Langton.cpp
        Langton.h
        makefile
        Server.cpp
        Server.h
        Server.cpp)
