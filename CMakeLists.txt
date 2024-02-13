cmake_minimum_required(VERSION 3.28)
project(hdepp)

set(CMAKE_CXX_STANDARD 26)

add_library(hdepp STATIC
        hdepp32.cpp
        hdepp32.h
        hdepp64.cpp
        hdepp64.h
        hdepp64.h
        hdepp64.h
        pstdint.h
        table32.h
        table64.h)
