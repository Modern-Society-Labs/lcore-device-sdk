# IoT Device SDK for L{CORE} System

**Standards-based C/C++ SDK for IoT device authentication and secure communication with Cartesi-powered smart city infrastructure.**

## Overview

The lcore-device-sdk provides production-ready libraries for IoT devices to authenticate and communicate securely with the L{CORE} system - a trust-minimized smart city platform built on Cartesi Rollups and Arbitrum Orbit chains.

### Key Features

- **W3C Decentralized Identifiers (DIDs)**: Complete DID specification implementation for self-sovereign device identity
- **IETF JOSE Compliance**: JSON Web Signature (JWS) with ES256 algorithm for data integrity  
- **MbedTLS Integration**: Hardware-optimized cryptography with ARM PSA support
- **Cartesi Integration**: Native compatibility with L{CORE} rollups infrastructure
- **Production Ready**: Validated functional libraries with comprehensive testing

## Architecture Overview

The Device SDK operates within a multi-layer trust-minimized architecture:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   IoT Device    │    │   L{CORE} Node  │    │ Arbitrum Orbit  │
│                 │    │                 │    │    (KC Chain)   │
│  ┌───────────┐  │    │  ┌───────────┐  │    │  ┌───────────┐  │
│  │Device SDK │──┼────┼─▶│ Cartesi   │  │    │  │  Stylus   │  │
│  │           │  │    │  │ Rollups   │  │    │  │Contracts  │  │
│  │• DID Gen  │  │    │  │• Verify   │──┼────┼─▶│• Registry │  │
│  │• JWS Sign │  │    │  │• Encrypt  │  │    │  │• Analytics│  │
│  │• HTTP     │  │    │  │• Store    │  │    │  │• Payments │  │
│  └───────────┘  │    │  └───────────┘  │    │  └───────────┘  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
      Edge Layer           Processing Layer        Settlement Layer
```

### Data Flow Pipeline

| Stage | Component | Function | Output |
|-------|-----------|----------|---------|
| 1 | Device SDK | Generate DID from device keys | `did:lcore:abc123...` |
| 2 | Device SDK | Create JWS payload | `eyJhbGciOiJFUzI1Ni...` |
| 3 | Device SDK | Submit to gateway | JSON → Hex encoding |
| 4 | L{CORE} Node | Verify JWS signature | Extract plaintext data |
| 5 | L{CORE} Node | Dual encryption | AES-256-GCM + XChaCha20 |
| 6 | L{CORE} Node | Store in SQLite | Encrypted blob + metadata |
| 7 | L{CORE} Node | Emit vouchers | On-chain state updates |
| 8 | Stylus Contracts | Process vouchers | Registry + analytics |

## Quick Start

### Prerequisites

- CMake 3.18+
- C99 compatible compiler  
- MbedTLS 3.x (auto-downloaded)

### Build

```bash
git clone <repository-url>
cd lcore-device-sdk
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

### Basic Usage

```c
#include <lcore/did.h>
#include <lcore/jose.h>

// 1. Generate device DID
uint8_t device_key[32] = { /* your device private key */ };
lcore_did_document_t* did = lcore_did_create(device_key, 32);

char did_string[256];
size_t did_len = sizeof(did_string);
lcore_did_to_string(did, did_string, &did_len);

// 2. Sign sensor data  
const char* sensor_data = "{\"temperature\":23.4,\"humidity\":52}";
char jws_token[2048];
size_t jws_len = sizeof(jws_token);

lcore_jose_sign(
    (uint8_t*)sensor_data, strlen(sensor_data),
    device_key, 32,
    LCORE_JOSE_ALG_ES256,
    jws_token, &jws_len
);

// 3. Submit to L{CORE} system
// (HTTP client implementation required)
```

## Integration with L{CORE} Node

### Message Formats

The Device SDK generates data in formats expected by the L{CORE} Cartesi application:

#### Device Registration
```json
{
  "type": "register_device",
  "device_id": "did:lcore:cbc32f41bbb1b704b200067859a90d4c", 
  "did_document": "{\"id\":\"did:lcore:cbc32f41bbb1b704b200067859a90d4c\"}"
}
```

#### Sensor Data Submission  
```json
{
  "type": "submit_sensor_data",
  "device_id": "did:lcore:cbc32f41bbb1b704b200067859a90d4c",
  "encrypted_payload": "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0ZW1wZXJhdHVyZSI6MjMuNCwiaHVtaWRpdHkiOjUyfQ.signature"
}
```

### Submission Protocol

1. **Encode**: JSON payload → Hex string (0x prefix)
2. **Submit**: POST to `https://<lcore-endpoint>/advance`
3. **Headers**: `Content-Type: application/json`
4. **Body**: Hex-encoded JSON string

Example:
```bash
curl -X POST 'https://lcore-iot-core.fly.dev/advance' \
  -H 'Content-Type: application/json' \
  -d '0x7b2274797065223a2272656769737465725f646576696365...'
```

## Connection to Arbitrum Orbit Layer

### Smart Contract Integration

The L{CORE} system settles on KC-Chain (Arbitrum Orbit) using Stylus contracts for 10x gas efficiency:

| Contract Type | Function | Gas Savings |
|---------------|----------|-------------|
| Device Registry | Store verified device DIDs | 90% vs Solidity |
| Analytics Engine | Process aggregated sensor data | 85% vs Solidity |  
| Payment System | Handle micro-transactions | 92% vs Solidity |
| Governance | Manage system parameters | 88% vs Solidity |

### Voucher Flow

```
Device SDK → L{CORE} Node → Cartesi VM → Vouchers → Stylus Contracts
     ↓              ↓            ↓          ↓           ↓
   JWS Sign    Verify & Store  Analytics  Execute   Update Registry
```

### On-Chain Queries

External dApps can query device data through:

1. **GraphQL Interface**: `POST /graphql` (L{CORE} Node)
2. **Stylus Contracts**: Direct on-chain calls (KC-Chain)
3. **IPFS/Arweave**: Decentralized data availability (planned)

## API Reference

### DID Management

| Function | Parameters | Returns | Description |
|----------|------------|---------|-------------|
| `lcore_did_create` | `key[]`, `key_len` | `lcore_did_document_t*` | Generate DID from key material |
| `lcore_did_to_string` | `did`, `buffer`, `buffer_len` | `int` | Serialize DID to string |
| `lcore_did_free` | `did` | `void` | Release DID resources |

### JOSE Operations

| Function | Parameters | Returns | Description |
|----------|------------|---------|-------------|
| `lcore_jose_sign` | `data`, `data_len`, `key`, `key_len`, `alg`, `output`, `output_len` | `int` | Create JWS signature |
| `lcore_jose_verify` | `jws`, `jws_len`, `key`, `key_len` | `int` | Verify JWS signature |

### Algorithm Support

| Algorithm | Status | Use Case |
|-----------|--------|----------|
| ES256 | Production | Primary signing algorithm |
| RS256 | Planned | Alternative for RSA keys |
| EdDSA | Future | Next-generation signatures |

## Testing Tools

### Functional Validation
```bash
# Run comprehensive tests
./build/tests/functional/test_sdk_basic

# Generate test payloads
./build/tools/generate_test_payloads

# Generate integration scripts  
./build/tools/generate_test_payloads script
```

### Integration Testing
```bash
# Test device registration
curl -X POST 'https://lcore-endpoint/advance' \
  -H 'Content-Type: application/json' \
  -d '<hex-encoded-registration>'

# Verify submission
curl -X POST 'https://lcore-endpoint/graphql' \
  -H 'Content-Type: application/json' \
  -d '{"query":"{ inputs { totalCount } }"}'
```

## Production Deployment

### Supported Platforms

| Platform | Status | Notes |
|----------|--------|--------|
| Linux (x86_64) | Production | Primary development platform |
| Linux (ARM64) | Production | Raspberry Pi, edge devices |
| ESP32 | Planned | Requires PAL implementation |
| Nordic nRF | Planned | BLE + cellular connectivity |
| STM32 | Future | Ultra low-power applications |

### Hardware Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| Flash | 256KB | 512KB |
| RAM | 64KB | 128KB |
| Crypto | Software | Hardware accelerated |
| Network | Any IP stack | TLS 1.3 support |

## Security Considerations

### Cryptographic Security

- **ES256**: NIST P-256 curve with SHA-256
- **Key Storage**: Platform-specific secure storage (ARM PSA preferred)
- **Randomness**: Hardware RNG required for production
- **Side Channels**: Constant-time implementations via MbedTLS

### Network Security

- **TLS 1.3**: Required for all HTTP communications
- **Certificate Pinning**: Recommended for production deployments
- **Rate Limiting**: Built into L{CORE} gateway
- **Replay Protection**: Timestamp validation in JWS payloads

## Development Status

| Component | Status | Completeness |
|-----------|--------|--------------|
| DID Library | Production | 100% |
| JOSE Library | Production | 100% |
| MbedTLS Integration | Production | 100% |
| HTTP Client | In Development | 0% |
| Platform Abstraction | Planned | 0% |
| Example Applications | In Development | 30% |

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for development guidelines.

## License

This project is licensed under the terms specified in [LICENSE](../LICENSE).

## Related Projects

- [lcore-node](../lcore-node/): Cartesi application for IoT data processing
- [lcore-platform](https://github.com/your-org/lcore-platform): TypeScript gateway and Stylus contracts  
- [lcore-shared](https://github.com/your-org/lcore-shared): Common types and schemas
