cmake_minimum_required(VERSION 3.28)
project(mhde VERSION 1.0 DESCRIPTION "A modern C++ port of the HDE (Hacker Disassembler Engine).")

# C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    mhde32.cpp
    mhde64.cpp
)

set(HEADERS
    mhde32.hpp
    mhde64.hpp
    pstdint.hpp
    table32.hpp
    table64.hpp
    wrapper.hpp
)

add_library(mhde_lib STATIC ${SOURCES} ${HEADERS})

# Include directories (if needed)
# target_include_directories(hmde_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler options
if (MSVC)
    target_compile_options(mhde_lib PRIVATE /W4 /WX)
else()
    target_compile_options(mhde_lib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Optional: If you want to install the library
# install(TARGETS mhde_lib DESTINATION lib)
# install(FILES ${HEADERS} DESTINATION include)
