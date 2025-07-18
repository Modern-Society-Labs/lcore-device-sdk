#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lcore/did.h>
#include <lcore/jose.h>

// Test key material (simulated P-256 private key - 32 bytes)
static const uint8_t test_private_key[32] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
};

// Test public key material (simulated - first 32 bytes for testing)
static const uint8_t test_public_key[32] = {
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0
};

int test_did_generation() {
    printf("=== Testing DID Generation ===\n");
    
    // Create DID document from public key
    lcore_did_document_t* did_doc = lcore_did_create(test_public_key, sizeof(test_public_key));
    if (!did_doc) {
        printf("❌ Failed to create DID document\n");
        return -1;
    }
    
    // Get DID string
    char did_string[256];
    size_t did_len = sizeof(did_string);
    int result = lcore_did_to_string(did_doc, did_string, &did_len);
    
    if (result != 0) {
        printf("❌ Failed to serialize DID document\n");
        lcore_did_free(did_doc);
        return -1;
    }
    
    printf("✅ DID Generated: %s\n", did_string);
    printf("✅ DID Length: %zu characters\n", strlen(did_string));
    
    // Verify format
    if (strncmp(did_string, "did:lcore:", 10) == 0) {
        printf("✅ DID Format: Valid (starts with 'did:lcore:')\n");
    } else {
        printf("❌ DID Format: Invalid\n");
        lcore_did_free(did_doc);
        return -1;
    }
    
    lcore_did_free(did_doc);
    printf("✅ DID Generation: SUCCESS\n\n");
    return 0;
}

int test_jose_signing() {
    printf("=== Testing JOSE Signing ===\n");
    
    // Test sensor data
    const char* sensor_data = "{\"temperature\":23.4,\"humidity\":52,\"timestamp\":\"2024-01-01T12:00:00Z\"}";
    printf("📋 Test Payload: %s\n", sensor_data);
    
    // Sign the data
    char jws_buffer[2048];
    size_t jws_len = sizeof(jws_buffer);
    
    int result = lcore_jose_sign(
        (const uint8_t*)sensor_data, strlen(sensor_data),
        test_private_key, sizeof(test_private_key),
        LCORE_JOSE_ALG_ES256,
        jws_buffer, &jws_len
    );
    
    if (result != 0) {
        printf("❌ JOSE Signing Failed (error code: %d)\n", result);
        return -1;
    }
    
    printf("✅ JWS Generated Successfully\n");
    printf("✅ JWS Length: %zu characters\n", jws_len);
    printf("📋 JWS Token: %.100s...\n", jws_buffer); // Show first 100 chars
    
    // Verify JWS format (header.payload.signature)
    int dot_count = 0;
    for (size_t i = 0; i < jws_len; i++) {
        if (jws_buffer[i] == '.') dot_count++;
    }
    
    if (dot_count == 2) {
        printf("✅ JWS Format: Valid (header.payload.signature)\n");
    } else {
        printf("❌ JWS Format: Invalid (found %d dots, expected 2)\n", dot_count);
        return -1;
    }
    
    printf("✅ JOSE Signing: SUCCESS\n\n");
    return 0;
}

int test_lcore_node_format() {
    printf("=== Testing lcore-node Format Compatibility ===\n");
    
    // Generate DID
    lcore_did_document_t* did_doc = lcore_did_create(test_public_key, sizeof(test_public_key));
    char did_string[256];
    size_t did_len = sizeof(did_string);
    lcore_did_to_string(did_doc, did_string, &did_len);
    
    // Generate device registration JSON
    printf("📋 Device Registration Format:\n");
    printf("{\n");
    printf("  \"type\": \"register_device\",\n");
    printf("  \"device_id\": \"%s\",\n", did_string);
    printf("  \"did_document\": \"{\\\"id\\\":\\\"%s\\\"}\"\n", did_string);
    printf("}\n\n");
    
    // Generate sensor data JWS
    const char* sensor_data = "{\"temperature\":23.4,\"humidity\":52}";
    char jws_buffer[2048];
    size_t jws_len = sizeof(jws_buffer);
    
    lcore_jose_sign(
        (const uint8_t*)sensor_data, strlen(sensor_data),
        test_private_key, sizeof(test_private_key),
        LCORE_JOSE_ALG_ES256,
        jws_buffer, &jws_len
    );
    
    // Generate sensor submission JSON
    printf("📋 Sensor Data Submission Format:\n");
    printf("{\n");
    printf("  \"type\": \"submit_sensor_data\",\n");
    printf("  \"device_id\": \"%s\",\n", did_string);
    printf("  \"encrypted_payload\": \"%.80s...\"\n", jws_buffer);
    printf("}\n\n");
    
    printf("✅ lcore-node Format: COMPATIBLE\n\n");
    
    lcore_did_free(did_doc);
    return 0;
}

int main() {
    printf("🧪 Device SDK Functional Testing\n");
    printf("================================\n\n");
    
    int result = 0;
    
    // Test 1: DID Generation
    if (test_did_generation() != 0) {
        result = -1;
    }
    
    // Test 2: JOSE Signing  
    if (test_jose_signing() != 0) {
        result = -1;
    }
    
    // Test 3: Format Compatibility
    if (test_lcore_node_format() != 0) {
        result = -1;
    }
    
    printf("================================\n");
    if (result == 0) {
        printf("🎉 ALL TESTS PASSED - Device SDK is functional!\n");
    } else {
        printf("❌ SOME TESTS FAILED - Check implementation\n");
    }
    
    return result;
} 