# lcore-device-sdk

The `lcore-device-sdk` provides a robust, standards-based C/C++ library for IoT device authentication and secure communication. It is designed for portability and can be adapted to a wide range of hardware platforms, from constrained microcontrollers to more powerful edge devices.

## Features

-   **W3C Decentralized Identifiers (DIDs)**: A complete implementation of DID standards for decentralized, self-sovereign device identity.
-   **IETF JOSE & COSE**: Support for JSON Object Signing and Encryption (JOSE) and CBOR Object Signing and Encryption (COSE) for secure data exchange.
-   **Platform Abstraction Layer (PAL)**: A flexible PAL allows the SDK to be ported to different hardware (e.g., ESP32, Nordic, Linux).
-   **Cryptographic Primitives**: Leverages standard cryptographic libraries and provides hooks for hardware-accelerated crypto.
-   **Low Footprint**: Optimized for resource-constrained environments.

## Role in the IoT-L{CORE} MVP

In the current MVP, this SDK's primary role is to provide the standards and patterns for device identity and data packaging. While the MVP testing uses `curl` to simulate device interactions, a real device would use this SDK to:
1.  Generate and manage a W3C DID for self-sovereign identity.
2.  Create a JSON payload containing sensor data (e.g., `{"temperature":25.5,"humidity":60.1}`).
3.  Sign the data payload or a request token using JOSE standards.
4.  Send the final payload to the `lcore-node-mvp`'s `/device/data` endpoint via an HTTP POST request.

The `lcore-node-mvp` would then be responsible for validating the device's signature before processing the data. This SDK ensures that data arriving at the node is authentic and originates from a trusted, registered device.

## Project Structure

```
lcore-device-sdk/
├── core/             # Core library (DID, JOSE, etc.)
│   ├── src/
│   └── include/
├── pal/              # Platform Abstraction Layer (ESP32, Linux, etc.)
├── examples/         # Example applications
├── tests/            # Unit and integration tests
├── docs/             # Documentation
└── CMakeLists.txt    # Main CMake build file
```

## Getting Started

### Prerequisites

-   CMake (version 3.10 or higher)
-   A C/C++ compiler (e.g., GCC, Clang)
-   (Optional) A target toolchain for your specific hardware.

### Building the SDK

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd lcore-device-sdk
    ```

2.  **Configure the build:**
    ```bash
    cmake -B build
    ```
    You can customize the build with the following options:
    -   `-DLCORE_BUILD_EXAMPLES=ON` (or OFF)
    -   `-DLCORE_BUILD_TESTS=ON` (or OFF)

3.  **Build the libraries and examples:**
    ```bash
    cmake --build build
    ```

## Usage

Link the `lcore` library to your application and include the necessary headers from `core/include`.

```c
#include <lcore/did.h>

int main() {
    // Your device logic here...
    lcore_did_document_t* doc = lcore_did_create(...);
    // ...
    return 0;
}
```

## Contributing

Please see the main `CONTRIBUTING.md` in the root of the monorepo. Contributions to the SDK are welcome, especially new PAL implementations and bug fixes.

## License

MIT
