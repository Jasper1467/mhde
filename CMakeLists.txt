cmake_minimum_required(VERSION 3.21)
project(mhde VERSION 1.0)
set(PROJECT_DESCRIPTION "A modern C++ port of the HDE (Hacker Disassembler Engine).")

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH 32)
endif()

# Source files
if (${ARCH} EQUAL 32)
    set(SOURCES
        src/mhde32.cpp
    )
    set(HEADERS
        include/mhde32.hpp
        include/table32.hpp
        include/wrapper.hpp
    )
elseif (${ARCH} EQUAL 64)
    set(SOURCES
        src/mhde64.cpp
    )
    set(HEADERS
        include/mhde64.hpp
        include/table64.hpp
        include/wrapper.hpp
    )
else()
    message(FATAL_ERROR "Unsupported architecture")
endif()

message(STATUS "Building in architecture: ${ARCH} bit!")

# Create library
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

# Define an option to enable/disable building tests
option(BUILD_TESTS "Build tests" OFF) # Set to OFF by default

if (BUILD_TESTS)
    # Add an executable target for tests/tests.cpp
    add_executable(tests tests/tests.cpp)

    # Include directories for tests
    target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # Link your library to the tests executable
    target_link_libraries(tests PRIVATE mhde_lib)
endif()
