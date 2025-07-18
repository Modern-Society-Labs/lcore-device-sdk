# L{CORE} Device SDK Deployment Guide

**Production deployment guide for IoT devices using the L{CORE} Device SDK in real-world environments.**

## Overview

This guide covers deployment of the L{CORE} Device SDK across different hardware platforms, from development environments to production IoT deployments at scale.

## Deployment Architecture

### Target Environments

```
┌─────────────────────────────────────────────────────────────────┐
│                    DEPLOYMENT ENVIRONMENTS                      │
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │DEVELOPMENT  │    │  STAGING    │    │ PRODUCTION  │         │
│  │             │    │             │    │             │         │
│  │• Local PC   │    │• Test Cloud │    │• Edge Nodes │         │
│  │• Emulators  │───▶│• Mock APIs  │───▶│• Real Devices│         │
│  │• Unit Tests │    │• Integration│    │• Fleet Mgmt │         │
│  └─────────────┘    └─────────────┘    └─────────────┘         │
│         │                   │                   │              │
│   Fast Iteration        Full Integration   High Availability   │
│   Debug Tools           Performance Test   Security Focus      │
│   No Encryption         Rate Limiting      Hardware Crypto     │
└─────────────────────────────────────────────────────────────────┘
```

### Platform Support Matrix

| Platform | Status | Memory | Flash | Crypto | Network | Notes |
|----------|--------|--------|-------|--------|---------|--------|
| **Linux x86_64** | Production | 512MB+ | 1GB+ | Software | Ethernet/WiFi | Development primary |
| **Linux ARM64** | Production | 256MB+ | 512MB+ | ARM Crypto | WiFi/Cellular | Raspberry Pi 4+ |
| **Linux ARM32** | Planned | 128MB+ | 256MB+ | Software | WiFi | Raspberry Pi 3 |
| **ESP32-S3** | Planned | 512KB | 8MB+ | Hardware | WiFi/BLE | Microcontroller |
| **Nordic nRF9160** | Future | 256KB | 1MB+ | ARM TrustZone | LTE-M/NB-IoT | Cellular IoT |
| **STM32 U5** | Future | 786KB | 4MB+ | Hardware | Custom | Ultra low power |

## Development Environment Setup

### Prerequisites

**System Requirements**
- CMake 3.18 or later
- C99 compatible compiler (GCC 9+, Clang 10+)
- Git for source control
- Internet connection for dependency download

**Platform-Specific Tools**

**Linux (Ubuntu/Debian)**
```bash
# Install build tools
sudo apt update
sudo apt install build-essential cmake git pkg-config

# Install additional dependencies
sudo apt install libssl-dev libcurl4-openssl-dev

# Clone and build
git clone <repository-url> lcore-device-sdk
cd lcore-device-sdk
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

**macOS**
```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew (if not present)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake pkg-config

# Build SDK
git clone <repository-url> lcore-device-sdk
cd lcore-device-sdk
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

**Windows (MSYS2/MinGW)**
```bash
# Install MSYS2 from https://www.msys2.org/
# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-pkg-config git

# Build SDK
git clone <repository-url> lcore-device-sdk
cd lcore-device-sdk
cmake -B build -G "MinGW Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

### Build Configuration Options

**CMake Configuration**
```cmake
# Development build (default)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLCORE_ENABLE_TESTS=ON \
  -DLCORE_ENABLE_TOOLS=ON \
  -DLCORE_ENABLE_EXAMPLES=ON

# Production build  
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLCORE_ENABLE_TESTS=OFF \
  -DLCORE_ENABLE_TOOLS=OFF \
  -DLCORE_STATIC_LINKING=ON

# Cross-compilation for ARM64
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-gnu.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DLCORE_TARGET_PLATFORM=ARM64
```

**Configuration Options**
| Option | Default | Description | Production |
|--------|---------|-------------|------------|
| `CMAKE_BUILD_TYPE` | Debug | Build configuration | Release |
| `LCORE_ENABLE_TESTS` | ON | Build test suite | OFF |
| `LCORE_ENABLE_TOOLS` | ON | Build development tools | OFF |
| `LCORE_ENABLE_EXAMPLES` | ON | Build example applications | OFF |
| `LCORE_STATIC_LINKING` | OFF | Static library linking | ON |
| `LCORE_HARDWARE_CRYPTO` | AUTO | Use hardware acceleration | ON (if available) |

## Production Deployment

### Security Configuration

**Key Management**
```c
// Production key storage (platform-specific)
#ifdef ARM_PSA_AVAILABLE
    // Use ARM PSA secure storage
    psa_status_t status = psa_crypto_init();
    psa_key_handle_t key_handle;
    
    // Import device key into secure storage
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);
    
    status = psa_import_key(&attributes, device_key_material, 32, &key_handle);
#else
    // Software fallback - encrypt and store
    uint8_t encrypted_key[64];
    encrypt_device_key(device_key_material, encrypted_key);
    secure_storage_write("device_key", encrypted_key, sizeof(encrypted_key));
#endif
```

**Network Security**
```c
// TLS 1.3 configuration for HTTP client
struct lcore_http_config {
    const char* ca_cert_path;           // "/etc/ssl/certs/ca-certificates.crt"
    const char* client_cert_path;       // NULL for no client cert
    const char* client_key_path;        // NULL for no client cert
    bool verify_peer;                   // true for production
    bool verify_hostname;               // true for production
    const char* cipher_list;            // "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:!aNULL:!MD5:!DSS"
    int timeout_seconds;                // 30
    int max_retries;                    // 3
};
```

### Hardware Platform Deployment

#### Raspberry Pi 4 (ARM64)

**System Preparation**
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install runtime dependencies
sudo apt install -y curl ca-certificates systemd

# Create application user
sudo useradd -r -s /bin/false lcore-device
sudo mkdir -p /opt/lcore-device
sudo chown lcore-device:lcore-device /opt/lcore-device
```

**Application Deployment**
```bash
# Copy application binary
sudo cp build/examples/sensor_client /opt/lcore-device/
sudo chmod +x /opt/lcore-device/sensor_client

# Copy configuration
sudo cp config/production.conf /opt/lcore-device/
sudo chown lcore-device:lcore-device /opt/lcore-device/production.conf
sudo chmod 600 /opt/lcore-device/production.conf

# Install systemd service
sudo cp scripts/lcore-device.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable lcore-device
sudo systemctl start lcore-device
```

**Systemd Service Configuration**
```ini
[Unit]
Description=L{CORE} IoT Device Client
After=network-online.target
Wants=network-online.target

[Service]
Type=exec
User=lcore-device
Group=lcore-device
WorkingDirectory=/opt/lcore-device
ExecStart=/opt/lcore-device/sensor_client --config production.conf
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

# Security hardening
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/opt/lcore-device/data

[Install]
WantedBy=multi-user.target
```

#### ESP32-S3 (Embedded)

**Platform Abstraction Layer**
```c
// esp32_pal.c - ESP32 platform implementation
#include "lcore/pal.h"
#include "esp_system.h"
#include "esp_random.h"
#include "nvs_flash.h"

// Secure key storage using NVS
int lcore_pal_key_store(const char* key_id, const uint8_t* key_data, size_t key_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("lcore_keys", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return -1;
    
    err = nvs_set_blob(nvs_handle, key_id, key_data, key_len);
    nvs_close(nvs_handle);
    
    return (err == ESP_OK) ? 0 : -1;
}

// Hardware random number generation
int lcore_pal_random_bytes(uint8_t* buffer, size_t length) {
    esp_fill_random(buffer, length);
    return 0;
}

// Network connectivity
int lcore_pal_http_post(const char* url, const char* data, size_t data_len) {
    // Use ESP32 HTTP client implementation
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 30000,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_post_field(client, data, data_len);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    
    return (err == ESP_OK) ? 0 : -1;
}
```

### Fleet Management

#### Device Provisioning

**Manufacturing Process**
```c
// Device factory provisioning
int provision_device(const char* device_serial) {
    // 1. Generate unique device key
    uint8_t device_key[32];
    if (lcore_pal_random_bytes(device_key, 32) != 0) {
        return -1;
    }
    
    // 2. Create device DID
    lcore_did_document_t* did_doc = lcore_did_create(device_key, 32);
    if (!did_doc) {
        return -1;
    }
    
    char did_string[256];
    size_t did_len = sizeof(did_string);
    lcore_did_to_string(did_doc, did_string, &did_len);
    
    // 3. Store in secure storage
    lcore_pal_key_store("device_key", device_key, 32);
    lcore_pal_config_store("device_id", did_string);
    lcore_pal_config_store("device_serial", device_serial);
    
    // 4. Register with provisioning server
    char registration_payload[1024];
    snprintf(registration_payload, sizeof(registration_payload),
        "{"
        "\"type\":\"provision_device\","
        "\"device_id\":\"%s\","
        "\"serial_number\":\"%s\","
        "\"timestamp\":\"%ld\""
        "}",
        did_string, device_serial, time(NULL));
    
    int result = submit_to_provisioning_server(registration_payload);
    
    lcore_did_free(did_doc);
    return result;
}
```

#### Over-the-Air Updates

**Update Mechanism**
```c
// OTA update handling
struct lcore_ota_update {
    char version[32];
    char download_url[256];
    uint8_t signature[64];
    size_t payload_size;
};

int handle_ota_update(const struct lcore_ota_update* update) {
    // 1. Verify update signature
    if (verify_ota_signature(update) != 0) {
        return -1;
    }
    
    // 2. Download update payload
    uint8_t* payload = download_update_payload(update->download_url);
    if (!payload) {
        return -1;
    }
    
    // 3. Apply update atomically
    if (apply_firmware_update(payload, update->payload_size) != 0) {
        free(payload);
        return -1;
    }
    
    // 4. Verify update and restart
    schedule_restart();
    free(payload);
    return 0;
}
```

### Monitoring and Diagnostics

#### Device Health Monitoring

**Health Metrics**
```c
// Device health reporting
struct lcore_device_health {
    uint32_t uptime_seconds;
    uint32_t free_memory_bytes;
    uint32_t total_memory_bytes;
    float cpu_usage_percent;
    float temperature_celsius;
    uint32_t network_packets_sent;
    uint32_t network_packets_failed;
    uint32_t crypto_operations_total;
    uint32_t crypto_operations_failed;
};

int collect_health_metrics(struct lcore_device_health* health) {
    health->uptime_seconds = get_uptime();
    health->free_memory_bytes = get_free_memory();
    health->total_memory_bytes = get_total_memory();
    health->cpu_usage_percent = get_cpu_usage();
    health->temperature_celsius = get_device_temperature();
    
    // Get network statistics
    get_network_stats(&health->network_packets_sent, &health->network_packets_failed);
    
    // Get crypto statistics  
    get_crypto_stats(&health->crypto_operations_total, &health->crypto_operations_failed);
    
    return 0;
}
```

#### Remote Diagnostics

**Diagnostic Interface**
```c
// Remote diagnostic commands
enum lcore_diagnostic_command {
    LCORE_DIAG_HEALTH_CHECK,
    LCORE_DIAG_CRYPTO_TEST,
    LCORE_DIAG_NETWORK_TEST,
    LCORE_DIAG_STORAGE_TEST,
    LCORE_DIAG_SENSOR_TEST
};

int handle_diagnostic_command(enum lcore_diagnostic_command cmd) {
    switch (cmd) {
        case LCORE_DIAG_HEALTH_CHECK: {
            struct lcore_device_health health;
            collect_health_metrics(&health);
            report_health_status(&health);
            break;
        }
        
        case LCORE_DIAG_CRYPTO_TEST: {
            // Test DID generation and JWS signing
            uint8_t test_key[32];
            lcore_pal_random_bytes(test_key, 32);
            
            lcore_did_document_t* did = lcore_did_create(test_key, 32);
            if (!did) {
                report_diagnostic_failure("DID creation failed");
                return -1;
            }
            
            char jws_token[1024];
            size_t jws_len = sizeof(jws_token);
            int result = lcore_jose_sign(
                (uint8_t*)"test", 4, test_key, 32,
                LCORE_JOSE_ALG_ES256, jws_token, &jws_len);
            
            lcore_did_free(did);
            
            if (result != 0) {
                report_diagnostic_failure("JWS signing failed");
                return -1;
            }
            
            report_diagnostic_success("Crypto test passed");
            break;
        }
        
        // Additional diagnostic commands...
    }
    
    return 0;
}
```

### Performance Optimization

#### Memory Management

**Memory Pool Optimization**
```c
// Pre-allocated memory pools for predictable performance
struct lcore_memory_pool {
    uint8_t did_pool[10][1024];         // 10 DID documents
    uint8_t jws_pool[20][2048];         // 20 JWS tokens
    uint8_t http_pool[5][4096];         // 5 HTTP buffers
    uint32_t did_allocation_mask;
    uint32_t jws_allocation_mask;
    uint32_t http_allocation_mask;
};

static struct lcore_memory_pool g_memory_pool;

void* allocate_from_pool(enum pool_type type) {
    switch (type) {
        case POOL_TYPE_DID:
            for (int i = 0; i < 10; i++) {
                if (!(g_memory_pool.did_allocation_mask & (1 << i))) {
                    g_memory_pool.did_allocation_mask |= (1 << i);
                    return g_memory_pool.did_pool[i];
                }
            }
            break;
        // Handle other pool types...
    }
    return NULL;
}
```

#### Power Management

**Battery Optimization**
```c
// Power-aware operation scheduling
enum lcore_power_mode {
    LCORE_POWER_ACTIVE,     // Full performance
    LCORE_POWER_REDUCED,    // Reduced frequency
    LCORE_POWER_SLEEP,      // Deep sleep with wakeup
    LCORE_POWER_HIBERNATE   // Minimal power with external wakeup
};

int optimize_power_usage(float battery_level) {
    if (battery_level > 0.8f) {
        set_power_mode(LCORE_POWER_ACTIVE);
        set_transmission_interval(60);  // 1 minute
    } else if (battery_level > 0.5f) {
        set_power_mode(LCORE_POWER_REDUCED);
        set_transmission_interval(300); // 5 minutes
    } else if (battery_level > 0.2f) {
        set_power_mode(LCORE_POWER_SLEEP);
        set_transmission_interval(900); // 15 minutes
    } else {
        set_power_mode(LCORE_POWER_HIBERNATE);
        set_transmission_interval(3600); // 1 hour
    }
    
    return 0;
}
```

## Production Checklist

### Pre-Deployment Validation

**Security Verification**
- [ ] Device keys generated with hardware RNG
- [ ] Keys stored in secure storage (ARM PSA or equivalent)
- [ ] TLS 1.3 configured with proper CA certificates
- [ ] Certificate pinning enabled for production endpoints
- [ ] No hardcoded credentials in firmware
- [ ] Debug interfaces disabled in production builds

**Functionality Testing**
- [ ] DID generation produces valid identifiers
- [ ] JWS signatures verify correctly
- [ ] HTTP communication succeeds with production endpoints
- [ ] Error handling gracefully recovers from failures
- [ ] Memory usage within acceptable limits
- [ ] Performance meets timing requirements

**Integration Testing**
- [ ] Device registration succeeds with L{CORE} node
- [ ] Sensor data submission processes correctly
- [ ] Data appears in encrypted form in SQLite database
- [ ] Vouchers emitted to Arbitrum Orbit contracts
- [ ] End-to-end data flow verified

### Production Monitoring

**Key Metrics**
| Metric | Threshold | Action |
|--------|-----------|--------|
| Device Registration Success Rate | >95% | Investigate network/auth issues |
| Data Submission Success Rate | >98% | Check L{CORE} node availability |
| Memory Usage | <80% of available | Optimize memory allocation |
| CPU Usage | <70% average | Optimize computation |
| Network Errors | <2% of requests | Check connectivity/endpoints |
| Crypto Failures | <0.1% of operations | Validate key material |

**Alerting Thresholds**
```c
// Monitoring thresholds
#define MAX_MEMORY_USAGE_PERCENT    80.0f
#define MAX_CPU_USAGE_PERCENT       70.0f
#define MAX_NETWORK_ERROR_RATE      0.02f
#define MAX_CRYPTO_ERROR_RATE       0.001f
#define MIN_BATTERY_LEVEL           0.1f

int check_health_thresholds(const struct lcore_device_health* health) {
    float memory_usage = (float)health->free_memory_bytes / health->total_memory_bytes;
    
    if ((1.0f - memory_usage) * 100.0f > MAX_MEMORY_USAGE_PERCENT) {
        send_alert("Memory usage critical", ALERT_CRITICAL);
    }
    
    if (health->cpu_usage_percent > MAX_CPU_USAGE_PERCENT) {
        send_alert("CPU usage high", ALERT_WARNING);
    }
    
    float network_error_rate = (float)health->network_packets_failed / 
                              (health->network_packets_sent + health->network_packets_failed);
    if (network_error_rate > MAX_NETWORK_ERROR_RATE) {
        send_alert("Network errors high", ALERT_WARNING);
    }
    
    return 0;
}
```

This deployment guide provides comprehensive coverage for production deployment of the L{CORE} Device SDK across different platforms and environments, with emphasis on security, reliability, and monitoring best practices. 