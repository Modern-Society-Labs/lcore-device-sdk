# L{CORE} Device SDK Project Structure

**Comprehensive guide to the Device SDK codebase organization, build system, and development workflow.**

## Repository Overview

The L{CORE} Device SDK is organized as a modular C/C++ library with clear separation of concerns, platform abstraction, and comprehensive testing infrastructure.

```
lcore-device-sdk/
├── core/                           # Core SDK implementation
│   ├── include/lcore/              # Public headers
│   │   ├── did.h                   # W3C DID management API
│   │   └── jose.h                  # IETF JOSE operations API
│   ├── src/                        # Implementation files
│   │   ├── did/                    # DID implementation
│   │   │   ├── did.c               # Core DID functions
│   │   │   └── did_utils.c         # Helper utilities
│   │   ├── jose/                   # JOSE implementation
│   │   │   ├── jose.c              # Core JOSE functions
│   │   │   ├── base64url.c         # Base64URL encoding
│   │   │   └── crypto_mbedtls.c    # MbedTLS integration
│   │   └── common/                 # Shared utilities
│   │       ├── memory.c            # Memory management
│   │       └── utils.c             # Common utilities
│   └── CMakeLists.txt              # Core library build config
├── tests/                          # Test infrastructure
│   ├── functional/                 # Integration tests
│   │   ├── test_sdk_basic.c        # Core functionality tests
│   │   └── CMakeLists.txt          # Test build config
│   └── unit/                       # Unit tests (planned)
├── tools/                          # Development tools
│   ├── generate_test_payloads.c    # Payload generation tool
│   └── CMakeLists.txt              # Tools build config
├── docs/                           # Documentation
│   ├── README.md                   # Existing overview docs
│   ├── INTEGRATION_GUIDE.md        # Technical integration guide
│   ├── API_REFERENCE.md            # Complete API documentation
│   ├── DEPLOYMENT_GUIDE.md         # Production deployment guide
│   ├── PROJECT_STRUCTURE.md        # This file
│   └── cartesi-integration-plan.md # Historical integration docs
├── examples/                       # Example applications (planned)
├── pal/                           # Platform Abstraction Layer (planned)
├── build/                         # Build artifacts (generated)
├── CMakeLists.txt                 # Root build configuration
├── compile_commands.json          # IDE integration (generated)
├── .gitignore                     # Git ignore rules
└── README.md                      # Main project documentation
```

## Core Library Architecture

### Module Organization

The core library follows a layered architecture with clear dependencies and interfaces:

```
┌─────────────────────────────────────────────────────────────────┐
│                    L{CORE} DEVICE SDK CORE                      │
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │PUBLIC API   │    │ PLATFORM    │    │  EXTERNAL   │         │
│  │             │    │ABSTRACTION  │    │DEPENDENCIES │         │
│  │• lcore/did  │    │             │    │             │         │
│  │• lcore/jose │────┼▶ Platform   │────┼▶ MbedTLS    │         │
│  │             │    │ Specific    │    │ (Crypto)    │         │
│  └─────────────┘    │ (Future)    │    │             │         │
│         │            └─────────────┘    └─────────────┘         │
│         ▼                                                       │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │CORE IMPL    │    │  UTILITIES  │    │   CRYPTO    │         │
│  │             │    │             │    │  BACKEND    │         │
│  │• DID Mgmt   │    │• Memory     │    │             │         │
│  │• JOSE Ops   │────┼▶ Base64URL  │────┼▶ ES256      │         │
│  │• Key Mgmt   │    │• JSON       │    │ P-256       │         │
│  └─────────────┘    │• Strings    │    │ SHA-256     │         │
│                     └─────────────┘    └─────────────┘         │
└─────────────────────────────────────────────────────────────────┘
```

### File Organization

#### Core Headers (`core/include/lcore/`)

| File | Purpose | Public API | Status |
|------|---------|------------|--------|
| `did.h` | W3C DID document management | 3 functions | Production |
| `jose.h` | IETF JOSE signing and verification | 2 functions | Production |

#### Implementation (`core/src/`)

| Directory | Purpose | Key Files | Dependencies |
|-----------|---------|-----------|--------------|
| `did/` | DID implementation | `did.c`, `did_utils.c` | SHA-256, JSON |
| `jose/` | JOSE implementation | `jose.c`, `crypto_mbedtls.c` | MbedTLS, Base64URL |
| `common/` | Shared utilities | `memory.c`, `utils.c` | Standard library |

### Build System

#### CMake Structure

The build system uses modern CMake (3.18+) with hierarchical configuration:

**Root CMakeLists.txt**
```cmake
# Project configuration
cmake_minimum_required(VERSION 3.18)
project(lcore-device-sdk VERSION 1.0.0 LANGUAGES C)

# Global settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(LCORE_ENABLE_TESTS "Build test suite" ON)
option(LCORE_ENABLE_TOOLS "Build development tools" ON)
option(LCORE_ENABLE_EXAMPLES "Build example applications" OFF)
option(LCORE_STATIC_LINKING "Use static linking" OFF)

# Dependencies
find_package(PkgConfig REQUIRED)
include(FetchContent)

# MbedTLS integration
FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/ARMmbed/mbedtls.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(mbedtls)

# Subdirectories
add_subdirectory(core)
if(LCORE_ENABLE_TESTS)
    add_subdirectory(tests/functional)
endif()
if(LCORE_ENABLE_TOOLS)
    add_subdirectory(tools)
endif()
```

**Core Library CMakeLists.txt**
```cmake
# Core library target
add_library(lcore_core STATIC
    src/did/did.c
    src/did/did_utils.c
    src/jose/jose.c
    src/jose/base64url.c
    src/jose/crypto_mbedtls.c
    src/common/memory.c
    src/common/utils.c
)

# Include directories
target_include_directories(lcore_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(lcore_core
    PRIVATE
        mbedtls
        mbedx509
        mbedcrypto
)

# Compiler flags
target_compile_options(lcore_core
    PRIVATE
        -Wall -Wextra -Werror
        -Wno-documentation  # Suppress MbedTLS doc warnings
)
```

#### Build Targets

| Target | Type | Purpose | Dependencies |
|--------|------|---------|--------------|
| `lcore_core` | Static Library | Main SDK library | MbedTLS |
| `test_sdk_basic` | Executable | Functional tests | lcore_core |
| `generate_test_payloads` | Executable | Development tool | lcore_core |

#### Dependency Management

**External Dependencies**
- **MbedTLS 3.4.0**: Cryptographic operations (auto-downloaded)
- **Google Test**: Unit testing framework (planned, auto-downloaded)

**Platform Dependencies**
- **POSIX**: Memory management, system calls
- **C99 Standard Library**: Basic functions and types

## Testing Infrastructure

### Test Organization

```
tests/
├── functional/                     # Integration tests
│   ├── test_sdk_basic.c            # Core functionality validation
│   │   ├── test_did_generation()   # DID creation and serialization
│   │   ├── test_jose_signing()     # JWS creation and format
│   │   └── test_lcore_node_format() # Integration format validation
│   └── CMakeLists.txt              # Test build configuration
└── unit/                          # Unit tests (planned)
    ├── test_did_unit.c             # DID unit tests
    ├── test_jose_unit.c            # JOSE unit tests
    └── CMakeLists.txt              # Unit test configuration
```

### Test Categories

| Test Type | Purpose | Coverage | Execution |
|-----------|---------|----------|-----------|
| **Functional** | End-to-end validation | DID + JOSE + Integration | `./build/tests/functional/test_sdk_basic` |
| **Unit** | Component isolation | Individual functions | Planned |
| **Integration** | System interop | SDK ↔ lcore-node | Manual with tools |
| **Performance** | Resource usage | Memory, CPU, timing | Planned |

### Test Execution

**Running Tests**
```bash
# Build all tests
cmake --build build

# Run functional tests
./build/tests/functional/test_sdk_basic

# Expected output:
# ================================
# DID Generation: SUCCESS
# JOSE Signing: SUCCESS  
# lcore-node Format: COMPATIBLE
# ALL TESTS PASSED
# ================================
```

## Development Tools

### Payload Generation

**Tool: `generate_test_payloads`**
- **Purpose**: Create hex-encoded payloads for lcore-node testing
- **Usage**: `./build/tools/generate_test_payloads [script]`
- **Output**: Ready-to-use curl commands and hex payloads

**Generated Outputs**
1. Device registration payloads (JSON → Hex)
2. Sensor data payloads (Data → JWS → JSON → Hex)
3. Integration test scripts for live testing

### IDE Integration

**Compilation Database**
- **File**: `compile_commands.json` (generated)
- **Purpose**: IDE integration for IntelliSense, error checking
- **Generation**: Automatic with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

**Editor Support**
| Editor | Features | Configuration |
|--------|----------|---------------|
| **VSCode** | IntelliSense, debugging | Auto-detects compile_commands.json |
| **CLion** | Full IDE support | CMake project import |
| **Vim/Neovim** | LSP integration | clangd with compile_commands.json |

## Platform Abstraction (Planned)

### PAL Architecture

```
pal/
├── include/lcore/pal.h             # Platform abstraction interface
├── linux/                         # Linux implementation
│   ├── pal_linux.c                 # POSIX-based implementation
│   └── CMakeLists.txt              # Linux-specific build
├── esp32/                         # ESP32 implementation
│   ├── pal_esp32.c                 # ESP-IDF integration
│   └── CMakeLists.txt              # ESP32-specific build
├── nordic/                        # Nordic nRF implementation
│   ├── pal_nordic.c                # nRF SDK integration
│   └── CMakeLists.txt              # Nordic-specific build
└── CMakeLists.txt                  # PAL build configuration
```

### PAL Interface

**Key Storage**
```c
int lcore_pal_key_store(const char* key_id, const uint8_t* key_data, size_t key_len);
int lcore_pal_key_load(const char* key_id, uint8_t* key_data, size_t* key_len);
int lcore_pal_key_delete(const char* key_id);
```

**Network Operations**
```c
int lcore_pal_http_post(const char* url, const char* data, size_t data_len);
int lcore_pal_http_get(const char* url, char* response, size_t* response_len);
```

**System Functions**
```c
int lcore_pal_random_bytes(uint8_t* buffer, size_t length);
uint64_t lcore_pal_get_timestamp(void);
void lcore_pal_delay_ms(uint32_t milliseconds);
```

## Code Quality Standards

### Coding Standards

**Style Guidelines**
- **C Standard**: C99 compliance required
- **Naming**: Snake_case for functions, variables
- **Prefixes**: `lcore_` for public API, `_lcore_` for internal
- **Headers**: Include guards, comprehensive documentation

**Example Function**
```c
/**
 * @brief Creates a W3C DID document from key material
 * 
 * @param key_material  Private key or seed (must be 32 bytes)
 * @param key_len       Length of key material (must be 32)
 * @return              Pointer to DID document, or NULL on failure
 * 
 * @note Caller must call lcore_did_free() to release memory
 * @see lcore_did_free(), lcore_did_to_string()
 */
lcore_did_document_t* lcore_did_create(
    const uint8_t* key_material,
    size_t key_len
);
```

### Static Analysis

**Tools Integration**
- **Clang Static Analyzer**: Memory leaks, null pointer dereference
- **Cppcheck**: Additional static analysis
- **Valgrind**: Runtime memory checking (development)

**Analysis Commands**
```bash
# Static analysis with clang
scan-build cmake --build build

# Memory checking with valgrind  
valgrind --leak-check=full ./build/tests/functional/test_sdk_basic

# Additional static analysis
cppcheck --enable=all core/src/
```

## Integration with L{CORE} System

### Repository Relationships

```
IoT-SDK/                           # Monorepo root
├── lcore-node/                    # Cartesi application
│   ├── src/cartesi_rollup.rs      # JWS verification integration
│   └── src/database.rs            # Device registry storage
├── lcore-device-sdk/              # This repository
│   ├── core/                      # SDK implementation
│   └── tools/                     # Integration tools
└── deploy/                        # Deployment infrastructure
    ├── contracts/                 # Stylus contracts
    └── scripts/                   # Deployment scripts
```

### Interface Compatibility

**Data Flow Integration**
1. **Device SDK** → JSON payloads → Hex encoding
2. **HTTP Gateway** → Cartesi rollups input
3. **lcore-node** → JWS verification → Encryption → Storage
4. **Vouchers** → Stylus contracts → On-chain state

**Message Format Compatibility**
- Device registration: `{"type": "register_device", ...}`
- Sensor data: `{"type": "submit_sensor_data", ...}`
- JWS format: RFC 7515 compact serialization
- Hex encoding: `0x` prefix for rollups submission

## Development Workflow

### Local Development

**Setup Process**
```bash
# 1. Clone repository
git clone <repository-url> lcore-device-sdk
cd lcore-device-sdk

# 2. Configure build
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 3. Build and test
cmake --build build
./build/tests/functional/test_sdk_basic

# 4. Generate integration payloads
./build/tools/generate_test_payloads
```

**Development Cycle**
1. **Edit** → Code changes in `core/src/`
2. **Build** → `cmake --build build`
3. **Test** → Run functional tests
4. **Validate** → Generate and test payloads
5. **Commit** → Git commit with descriptive message

### Continuous Integration (Planned)

**CI Pipeline**
```yaml
# .github/workflows/ci.yml
stages:
  - build:
      - cmake configure
      - cmake build
      - static analysis
  - test:
      - unit tests
      - functional tests
      - integration tests
  - deploy:
      - package artifacts
      - update documentation
```

This project structure provides a comprehensive foundation for IoT device integration with the L{CORE} trust-minimized smart city platform, emphasizing modularity, testability, and production readiness. 