# Functional test executable
add_executable(test_sdk_basic test_sdk_basic.c)

# Link against our core library
target_link_libraries(test_sdk_basic 
    PRIVATE 
        lcore_core
)

# Include directories for headers
target_include_directories(test_sdk_basic
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
) 