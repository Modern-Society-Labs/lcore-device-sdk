# Tools executable
add_executable(generate_test_payloads generate_test_payloads.c)

# Link against our core library
target_link_libraries(generate_test_payloads 
    PRIVATE 
        lcore_core
)

# Include directories for headers
target_include_directories(generate_test_payloads
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
) 