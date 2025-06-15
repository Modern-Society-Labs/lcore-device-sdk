cmake_minimum_required(VERSION 3.10)

project(lcore-device-sdk VERSION 0.1.0 LANGUAGES C CXX)

# Set C and CXX standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Subdirectories ---
# Core library containing DID, JOSE, and other core functionalities.
add_subdirectory(core)

# Platform Abstraction Layer (PAL) for different hardware.
add_subdirectory(pal)

# Examples demonstrating the SDK usage.
add_subdirectory(examples)

# --- Global Options ---
option(LCORE_BUILD_EXAMPLES "Build the example applications" ON)
option(LCORE_BUILD_TESTS "Build tests" ON)

# --- Include Directories ---
include_directories(
    core/include
)

# --- Testing ---
if(LCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Installation ---
# Placeholder for installation rules.
# install(TARGETS ...)
