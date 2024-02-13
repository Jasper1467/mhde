cmake_minimum_required(VERSION 3.28)
project(mhde)

set(CMAKE_CXX_STANDARD 26)

add_library(hdepp STATIC
        mhde32.cpp
        mhde32.h
        mhde64.cpp
        mhde64.h
        pstdint.h
        table32.h
        table64.h
)
