# L{CORE} Device SDK API Reference

**Complete API documentation for the L{CORE} Device SDK C/C++ library.**

## Overview

The L{CORE} Device SDK provides two primary modules for IoT device integration:

- **DID Module** (`lcore/did.h`): W3C Decentralized Identifiers for device identity
- **JOSE Module** (`lcore/jose.h`): IETF JSON Object Signing and Encryption for data integrity

## DID Management API

### Core Functions

#### `lcore_did_create`

**Signature**
```c
lcore_did_document_t* lcore_did_create(
    const uint8_t* key_material, 
    size_t key_len
);
```

**Parameters**
| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `key_material` | `const uint8_t*` | Private key or seed material | Must be 32 bytes for P-256 |
| `key_len` | `size_t` | Length of key material | Must equal 32 |

**Returns**
- **Success**: Pointer to `lcore_did_document_t` structure
- **Failure**: `NULL` (insufficient memory or invalid parameters)

**Description**  
Generates a W3C DID document from provided key material. Creates a deterministic DID identifier using SHA-256 hash of the public key derived from the private key.

**Example**
```c
uint8_t private_key[32] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
};

lcore_did_document_t* did_doc = lcore_did_create(private_key, 32);
if (!did_doc) {
    // Handle error: memory allocation failed or invalid key
    return -1;
}
```

**Security Notes**
- Key material should have at least 256 bits of entropy
- Private key should be stored securely (ARM PSA preferred)
- Generated DID is deterministic for same key material

---

#### `lcore_did_to_string`

**Signature**
```c
int lcore_did_to_string(
    const lcore_did_document_t* did_doc,
    char* buffer,
    size_t* buffer_len
);
```

**Parameters**
| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `did_doc` | `const lcore_did_document_t*` | DID document to serialize | Must be valid document |
| `buffer` | `char*` | Output buffer for DID string | Minimum 256 bytes recommended |
| `buffer_len` | `size_t*` | Input: buffer size, Output: actual length | Must be sufficient |

**Returns**
| Value | Meaning | Action |
|-------|---------|---------|
| `0` | Success | DID string written to buffer |
| `-1` | Invalid parameter | Check inputs |
| `-2` | Buffer too small | Increase buffer size |

**Description**  
Serializes a DID document to its string representation following the `did:lcore:` method specification.

**Example**
```c
char did_string[256];
size_t did_len = sizeof(did_string);

int result = lcore_did_to_string(did_doc, did_string, &did_len);
if (result == 0) {
    printf("DID: %s\n", did_string);
    printf("Length: %zu bytes\n", did_len);
    // Output: "did:lcore:cbc32f41bbb1b704b200067859a90d4c"
} else {
    // Handle serialization error
}
```

**Output Format**
- Prefix: `did:lcore:`
- Identifier: 32-character hexadecimal string
- Total length: 42 characters
- Example: `did:lcore:cbc32f41bbb1b704b200067859a90d4c`

---

#### `lcore_did_free`

**Signature**
```c
void lcore_did_free(lcore_did_document_t* did_doc);
```

**Parameters**
| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `did_doc` | `lcore_did_document_t*` | DID document to deallocate | Can be NULL (no-op) |

**Returns**
- `void` (no return value)

**Description**  
Releases all memory associated with a DID document. Safe to call with NULL pointer.

**Example**
```c
lcore_did_document_t* did_doc = lcore_did_create(key, 32);
// ... use DID document ...
lcore_did_free(did_doc);
did_doc = NULL; // Good practice
```

**Memory Management**
- Always call `lcore_did_free()` for documents created with `lcore_did_create()`
- Safe to call multiple times (idempotent)
- NULL-safe (will not crash if passed NULL)

---

### Data Structures

#### `lcore_did_document_t`

**Description**  
Opaque structure representing a W3C DID document. Internal structure is implementation-defined and should not be accessed directly.

**Usage Pattern**
```c
// Creation
lcore_did_document_t* did = lcore_did_create(key, key_len);

// Serialization  
char buffer[256];
size_t len = sizeof(buffer);
lcore_did_to_string(did, buffer, &len);

// Cleanup
lcore_did_free(did);
```

## JOSE Operations API

### Core Functions

#### `lcore_jose_sign`

**Signature**
```c
int lcore_jose_sign(
    const uint8_t* payload,
    size_t payload_len,
    const uint8_t* private_key,
    size_t key_len,
    lcore_jose_algorithm_t algorithm,
    char* jws_output,
    size_t* jws_len
);
```

**Parameters**
| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `payload` | `const uint8_t*` | Data to be signed | Any binary data |
| `payload_len` | `size_t` | Length of payload | Maximum 64KB recommended |
| `private_key` | `const uint8_t*` | Signing key material | 32 bytes for P-256 |
| `key_len` | `size_t` | Length of private key | Must equal 32 |
| `algorithm` | `lcore_jose_algorithm_t` | Signing algorithm | `LCORE_JOSE_ALG_ES256` |
| `jws_output` | `char*` | Output buffer for JWS | Minimum 2048 bytes |
| `jws_len` | `size_t*` | Input: buffer size, Output: JWS length | Must be sufficient |

**Returns**
| Value | Meaning | Action |
|-------|---------|---------|
| `0` | Success | JWS token created |
| `-1` | Invalid parameter | Check inputs |
| `-2` | Cryptographic failure | Retry or check key |
| `-3` | Buffer too small | Increase output buffer |
| `-4` | Unsupported algorithm | Use ES256 |

**Description**  
Creates a JSON Web Signature (JWS) token using the specified algorithm and private key. Implements RFC 7515 with compact serialization.

**Example**
```c
const char* sensor_data = "{\"temperature\":23.4,\"humidity\":52}";
uint8_t private_key[32] = { /* key material */ };
char jws_token[2048];
size_t jws_len = sizeof(jws_token);

int result = lcore_jose_sign(
    (const uint8_t*)sensor_data,
    strlen(sensor_data),
    private_key,
    32,
    LCORE_JOSE_ALG_ES256,
    jws_token,
    &jws_len
);

if (result == 0) {
    printf("JWS: %s\n", jws_token);
    // Output: "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0ZW1wZXJhdHVyZSI6MjMuNCwiaHVtaWRpdHkiOjUyfQ.signature"
}
```

**JWS Structure**
```
Header (Base64URL):     eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9
Payload (Base64URL):    eyJ0ZW1wZXJhdHVyZSI6MjMuNCwiaHVtaWRpdHkiOjUyfQ  
Signature (Base64URL):  [64-byte-ECDSA-signature]
```

---

#### `lcore_jose_verify`

**Signature**
```c
int lcore_jose_verify(
    const char* jws_token,
    size_t jws_len,
    const uint8_t* public_key,
    size_t key_len
);
```

**Parameters**
| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `jws_token` | `const char*` | JWS token to verify | Valid JWS format |
| `jws_len` | `size_t` | Length of JWS token | Actual string length |
| `public_key` | `const uint8_t*` | Verification key | 64 bytes for P-256 uncompressed |
| `key_len` | `size_t` | Length of public key | Must equal 64 |

**Returns**
| Value | Meaning | Action |
|-------|---------|---------|
| `0` | Valid signature | Accept data |
| `-1` | Invalid parameter | Check inputs |
| `-2` | Invalid JWS format | Reject token |
| `-3` | Signature verification failed | Reject token |
| `-4` | Unsupported algorithm | Check header |

**Description**  
Verifies a JWS token signature using the provided public key. Implements RFC 7515 verification process.

**Example**
```c
const char* jws_token = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0ZW1wZXJhdHVyZSI6MjMuNCwiaHVtaWRpdHkiOjUyfQ.signature";
uint8_t public_key[64] = { /* uncompressed P-256 public key */ };

int result = lcore_jose_verify(jws_token, strlen(jws_token), public_key, 64);
if (result == 0) {
    printf("Signature valid - data trusted\n");
} else {
    printf("Signature invalid - reject data\n");
}
```

---

### Algorithm Support

#### `lcore_jose_algorithm_t`

**Enumeration Values**
| Value | Algorithm | Status | Description |
|-------|-----------|--------|-------------|
| `LCORE_JOSE_ALG_ES256` | ECDSA P-256 + SHA-256 | Production | Primary algorithm |
| `LCORE_JOSE_ALG_RS256` | RSA-2048 + SHA-256 | Planned | Alternative for RSA keys |
| `LCORE_JOSE_ALG_EDDSA` | Ed25519 | Future | Next-generation signatures |

**Current Support**
- **ES256**: Full implementation with MbedTLS backend
- **RS256**: Planned for next release
- **EdDSA**: Future consideration

**Example Usage**
```c
// Currently supported
int result = lcore_jose_sign(data, len, key, 32, LCORE_JOSE_ALG_ES256, output, &out_len);

// Planned support  
// int result = lcore_jose_sign(data, len, rsa_key, 256, LCORE_JOSE_ALG_RS256, output, &out_len);
```

## Error Handling

### Error Codes

**Global Error Constants**
| Code | Constant | Description | Recovery Strategy |
|------|----------|-------------|-------------------|
| `0` | `LCORE_SUCCESS` | Operation completed successfully | Continue operation |
| `-1` | `LCORE_ERROR_INVALID_PARAM` | Invalid function parameter | Fix parameter values |
| `-2` | `LCORE_ERROR_CRYPTO_FAIL` | Cryptographic operation failed | Retry or use different key |
| `-3` | `LCORE_ERROR_MEMORY` | Memory allocation failed | Reduce memory usage |
| `-4` | `LCORE_ERROR_NETWORK` | Network operation failed | Retry with backoff |
| `-5` | `LCORE_ERROR_BUFFER_TOO_SMALL` | Output buffer insufficient | Increase buffer size |
| `-6` | `LCORE_ERROR_UNSUPPORTED` | Unsupported algorithm/feature | Use supported alternative |

### Error Handling Patterns

**Parameter Validation**
```c
int safe_did_create_example(const uint8_t* key, size_t key_len) {
    // Validate inputs
    if (!key || key_len != 32) {
        return LCORE_ERROR_INVALID_PARAM;
    }
    
    lcore_did_document_t* did = lcore_did_create(key, key_len);
    if (!did) {
        return LCORE_ERROR_MEMORY;
    }
    
    // Use DID...
    lcore_did_free(did);
    return LCORE_SUCCESS;
}
```

**Buffer Size Management**
```c
int safe_jose_sign_example(const char* data) {
    char jws_buffer[2048];
    size_t jws_len = sizeof(jws_buffer);
    
    int result = lcore_jose_sign(
        (uint8_t*)data, strlen(data),
        private_key, 32,
        LCORE_JOSE_ALG_ES256,
        jws_buffer, &jws_len
    );
    
    switch (result) {
        case LCORE_SUCCESS:
            printf("JWS created: %zu bytes\n", jws_len);
            break;
        case LCORE_ERROR_BUFFER_TOO_SMALL:
            printf("Buffer too small, need %zu bytes\n", jws_len);
            break;
        case LCORE_ERROR_CRYPTO_FAIL:
            printf("Cryptographic operation failed\n");
            break;
        default:
            printf("Unknown error: %d\n", result);
    }
    
    return result;
}
```

## Memory Management

### Allocation Patterns

**Stack vs Heap Usage**
| Component | Memory Type | Size | Lifetime |
|-----------|-------------|------|----------|
| DID strings | Stack | 256 bytes | Function scope |
| JWS tokens | Stack | 2048 bytes | Function scope |
| DID documents | Heap | ~1KB | User-managed |
| Crypto contexts | Stack | ~4KB | Function scope |

**Recommended Buffer Sizes**
```c
// DID operations
char did_string[256];           // Sufficient for any DID
char did_document[1024];        // Full DID document JSON

// JOSE operations  
char jws_token[2048];           // Sufficient for typical payloads
char jws_header[256];           // Header only
char jws_payload[1024];         // Payload only (depends on data)

// HTTP operations
char http_request[4096];        // Complete HTTP request
char http_response[2048];       // HTTP response headers
```

### Memory Safety

**Best Practices**
1. Always check return values for NULL/error
2. Call `lcore_did_free()` for every `lcore_did_create()`
3. Initialize buffers before use
4. Validate buffer sizes before calling functions
5. Use stack allocation for temporary data

**Common Pitfalls**
```c
// BAD: No error checking
lcore_did_document_t* did = lcore_did_create(key, 32);
lcore_did_to_string(did, buffer, &len); // Could crash if did is NULL

// GOOD: Proper error checking
lcore_did_document_t* did = lcore_did_create(key, 32);
if (!did) {
    return LCORE_ERROR_MEMORY;
}

int result = lcore_did_to_string(did, buffer, &len);
if (result != LCORE_SUCCESS) {
    lcore_did_free(did);
    return result;
}

lcore_did_free(did);
```

## Performance Characteristics

### Computational Complexity

| Operation | Complexity | Typical Time | Notes |
|-----------|------------|--------------|--------|
| DID generation | O(1) | 10ms | Includes key derivation |
| DID serialization | O(1) | <1ms | String formatting only |
| JWS signing | O(1) | 50ms | ECDSA signature creation |
| JWS verification | O(1) | 30ms | ECDSA signature verification |

### Memory Usage

**Runtime Memory**
| Function | Stack Usage | Heap Usage | Total |
|----------|-------------|------------|--------|
| `lcore_did_create` | 2KB | 1KB | 3KB |
| `lcore_did_to_string` | 512B | 0B | 512B |
| `lcore_jose_sign` | 4KB | 0B | 4KB |
| `lcore_jose_verify` | 3KB | 0B | 3KB |

**Optimization Tips**
- Reuse buffers across multiple operations
- Use stack allocation for temporary data
- Free DID documents promptly after use
- Consider batch operations for multiple signatures

## Thread Safety

### Concurrency Model

**Thread Safety Guarantees**
| Function | Thread Safety | Notes |
|----------|---------------|--------|
| `lcore_did_create` | Thread-safe | No shared state |
| `lcore_did_to_string` | Thread-safe | Read-only operations |
| `lcore_did_free` | Thread-safe | Atomic deallocation |
| `lcore_jose_sign` | Thread-safe | Independent crypto contexts |
| `lcore_jose_verify` | Thread-safe | Read-only verification |

**Usage in Multithreaded Applications**
```c
// Safe: Each thread uses independent resources
void* worker_thread(void* arg) {
    uint8_t thread_key[32];
    // ... initialize thread-specific key ...
    
    lcore_did_document_t* did = lcore_did_create(thread_key, 32);
    // ... use DID in this thread only ...
    lcore_did_free(did);
    
    return NULL;
}
```

**Shared Resource Management**
- DID documents are not shared between threads
- Each thread should manage its own crypto contexts
- Global configuration (if any) requires external synchronization

## Integration Examples

### Complete Device Registration

```c
#include <lcore/did.h>
#include <lcore/jose.h>
#include <stdio.h>
#include <string.h>

int register_device_complete_example() {
    // 1. Initialize device key (from secure storage)
    uint8_t device_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    
    // 2. Generate DID
    lcore_did_document_t* did_doc = lcore_did_create(device_key, 32);
    if (!did_doc) {
        printf("Failed to create DID\n");
        return -1;
    }
    
    // 3. Get DID string
    char did_string[256];
    size_t did_len = sizeof(did_string);
    int result = lcore_did_to_string(did_doc, did_string, &did_len);
    if (result != 0) {
        printf("Failed to serialize DID\n");
        lcore_did_free(did_doc);
        return -1;
    }
    
    // 4. Create registration payload
    char registration_json[1024];
    snprintf(registration_json, sizeof(registration_json),
        "{"
        "\"type\":\"register_device\","
        "\"device_id\":\"%s\","
        "\"did_document\":\"{\\\"id\\\":\\\"%s\\\"}\""
        "}",
        did_string, did_string);
    
    // 5. Convert to hex for submission
    printf("Registration payload: %s\n", registration_json);
    
    // 6. Cleanup
    lcore_did_free(did_doc);
    
    return 0;
}
```

### Complete Sensor Data Submission

```c
int submit_sensor_data_complete_example() {
    // Device key and DID (from previous registration)
    uint8_t device_key[32] = { /* same key as registration */ };
    const char* device_id = "did:lcore:cbc32f41bbb1b704b200067859a90d4c";
    
    // 1. Collect sensor data
    const char* sensor_data = "{"
        "\"temperature\":23.4,"
        "\"humidity\":52,"
        "\"timestamp\":\"2024-07-18T10:30:00Z\","
        "\"location\":\"sensor_node_001\""
        "}";
    
    // 2. Sign sensor data with JWS
    char jws_token[2048];
    size_t jws_len = sizeof(jws_token);
    
    int result = lcore_jose_sign(
        (const uint8_t*)sensor_data,
        strlen(sensor_data),
        device_key,
        32,
        LCORE_JOSE_ALG_ES256,
        jws_token,
        &jws_len
    );
    
    if (result != 0) {
        printf("Failed to sign sensor data: %d\n", result);
        return -1;
    }
    
    // 3. Create submission payload
    char submission_json[4096];
    snprintf(submission_json, sizeof(submission_json),
        "{"
        "\"type\":\"submit_sensor_data\","
        "\"device_id\":\"%s\","
        "\"encrypted_payload\":\"%s\""
        "}",
        device_id, jws_token);
    
    printf("Submission payload: %s\n", submission_json);
    
    // 4. In real implementation: convert to hex and submit via HTTP
    // hex_encode(submission_json, hex_payload);
    // http_post("/advance", hex_payload);
    
    return 0;
}
```

This API reference provides complete technical documentation for integrating with the L{CORE} Device SDK. All functions are production-ready and validated through comprehensive testing. 