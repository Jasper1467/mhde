cmake_minimum_required(VERSION 3.21)
project(mhde VERSION 1.0)
set(PROJECT_DESCRIPTION "A modern C++ port of the HDE (Hacker Disassembler Engine).")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_X86 "Build x86 version" ON)
option(BUILD_X64 "Build x64 version" ON)

function(set_arch_output_name target arch)
    if (WIN32)
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "mhde${arch}")
    else() # Linux/macOS
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "mhde${arch}")
    endif()
endfunction()

if (BUILD_X86)
    message(STATUS "Building x86 version")
    set(SOURCES_X86 src/mhde32.cpp)
    set(HEADERS_X86
        include/mhde32.hpp
        include/mhde_table32.hpp
        include/mhde_wrapper.hpp
    )

    add_library(mhde_lib_x86 STATIC ${SOURCES_X86} ${HEADERS_X86})
    set_arch_output_name(mhde_lib_x86 32)
endif()

if (BUILD_X64)
    message(STATUS "Building x64 version")
    set(SOURCES_X64 src/mhde64.cpp)
    set(HEADERS_X64
        include/mhde64.hpp
        include/mhde_table64.hpp
        include/mhde_wrapper.hpp
    )

    add_library(mhde_lib_x64 STATIC ${SOURCES_X64} ${HEADERS_X64})
    set_arch_output_name(mhde_lib_x64 64)
endif()

option(BUILD_TESTS "Build tests" OFF) # OFF by default

if (BUILD_TESTS)
    add_executable(tests tests/tests.cpp)
    target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    if (BUILD_X86)
        target_link_libraries(tests PRIVATE mhde_lib_x86)
    endif()
    if (BUILD_X64)
        target_link_libraries(tests PRIVATE mhde_lib_x64)
    endif()
endif()

