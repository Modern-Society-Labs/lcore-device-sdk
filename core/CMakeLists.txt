# Define the core library
add_library(lcore_core)

# Source files for the core library
target_sources(lcore_core
    PRIVATE
        src/did/did.c
        src/jose/jose.c
        # Add other source files here
)

# Public include directories
target_include_directories(lcore_core
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src
)

# Link to other libraries if needed
# target_link_libraries(lcore_core PRIVATE mbedtls)

# Installation rules
install(TARGETS lcore_core
    EXPORT lcore-device-sdk-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/lcore/
    DESTINATION include/lcore
) 