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

# Platform Abstraction Layer
# add_subdirectory(pal)

# Examples
# add_subdirectory(examples)

# --- Global Options ---
option(LCORE_BUILD_EXAMPLES "Build the example applications" ON)
option(LCORE_BUILD_TESTS "Build tests" ON)

# --- Include Directories ---
include_directories(
    core/include
)

# --- Testing ---
if(LCORE_BUILD_TESTS)
    # Enable testing
    enable_testing()

    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # add_subdirectory(tests)
endif()

# --- Installation ---
# Placeholder for installation rules.
# install(TARGETS ...)
