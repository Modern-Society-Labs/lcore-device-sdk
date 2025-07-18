cmake_minimum_required(VERSION 3.10)

project(lcore-device-sdk VERSION 0.1.0 LANGUAGES C CXX)

# Set C and CXX standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---
# ARM PSA (MbedTLS) for IoTeX-compatible cryptography
find_package(MbedTLS QUIET)
if(NOT MbedTLS_FOUND)
    # Download and build MbedTLS if not found
    include(FetchContent)
    FetchContent_Declare(
        mbedtls
        GIT_REPOSITORY https://github.com/ARMmbed/mbedtls.git
        GIT_TAG        v3.4.0
    )
    set(ENABLE_TESTING OFF CACHE BOOL "Disable MbedTLS tests")
    set(ENABLE_PROGRAMS OFF CACHE BOOL "Disable MbedTLS programs")
    FetchContent_MakeAvailable(mbedtls)
    
    # Fix: Disable documentation warnings for MbedTLS libraries
    # This prevents build failures due to MbedTLS documentation formatting issues
    if(TARGET mbedcrypto)
        target_compile_options(mbedcrypto PRIVATE 
            $<$<COMPILE_LANGUAGE:C>:-Wno-documentation>
        )
    endif()
    if(TARGET mbedtls)
        target_compile_options(mbedtls PRIVATE 
            $<$<COMPILE_LANGUAGE:C>:-Wno-documentation>
        )
    endif()
    if(TARGET mbedx509)
        target_compile_options(mbedx509 PRIVATE 
            $<$<COMPILE_LANGUAGE:C>:-Wno-documentation>
        )
    endif()
endif()

# --- Subdirectories ---
# Core library containing DID, JOSE, and other core functionalities.
add_subdirectory(core)

# Platform Abstraction Layer
# add_subdirectory(pal)

# Examples
# add_subdirectory(examples)

# Functional tests
add_subdirectory(tests/functional)

# Tools
add_subdirectory(tools)

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
