#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lcore/did.h>
#include <lcore/jose.h>

// Same test keys as functional test
static const uint8_t test_private_key[32] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
};

static const uint8_t test_public_key[32] = {
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0
};

void string_to_hex(const char* input, char* output) {
    const char* hex_chars = "0123456789abcdef";
    size_t len = strlen(input);
    
    strcpy(output, "0x");
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = input[i];
        output[2 + i*2] = hex_chars[c >> 4];
        output[2 + i*2 + 1] = hex_chars[c & 0x0f];
    }
    output[2 + len*2] = '\0';
}

void generate_device_registration_payload() {
    printf("=== Device Registration Payload ===\n");
    
    // Generate DID
    lcore_did_document_t* did_doc = lcore_did_create(test_public_key, sizeof(test_public_key));
    char did_string[256];
    size_t did_len = sizeof(did_string);
    lcore_did_to_string(did_doc, did_string, &did_len);
    
    // Create registration JSON
    char json_payload[1024];
    snprintf(json_payload, sizeof(json_payload),
        "{"
        "\"type\":\"register_device\","
        "\"device_id\":\"%s\","
        "\"did_document\":\"{\\\"id\\\":\\\"%s\\\"}\""
        "}",
        did_string, did_string);
    
    // Convert to hex
    char hex_payload[2048];
    string_to_hex(json_payload, hex_payload);
    
    printf("📋 JSON: %s\n", json_payload);
    printf("📦 HEX:  %s\n", hex_payload);
    printf("📏 Length: %zu characters\n", strlen(hex_payload));
    printf("\n🚀 Ready for submission to lcore-node!\n\n");
    
    lcore_did_free(did_doc);
}

void generate_sensor_data_payload() {
    printf("=== Sensor Data Payload ===\n");
    
    // Generate DID
    lcore_did_document_t* did_doc = lcore_did_create(test_public_key, sizeof(test_public_key));
    char did_string[256];
    size_t did_len = sizeof(did_string);
    lcore_did_to_string(did_doc, did_string, &did_len);
    
    // Generate JWS for sensor data
    const char* sensor_data = "{\"temperature\":25.1,\"humidity\":48,\"location\":\"test_lab\",\"device_type\":\"environmental_sensor\"}";
    char jws_buffer[2048];
    size_t jws_len = sizeof(jws_buffer);
    
    lcore_jose_sign(
        (const uint8_t*)sensor_data, strlen(sensor_data),
        test_private_key, sizeof(test_private_key),
        LCORE_JOSE_ALG_ES256,
        jws_buffer, &jws_len
    );
    
    // Create submission JSON
    char json_payload[4096];
    snprintf(json_payload, sizeof(json_payload),
        "{"
        "\"type\":\"submit_sensor_data\","
        "\"device_id\":\"%s\","
        "\"encrypted_payload\":\"%s\""
        "}",
        did_string, jws_buffer);
    
    // Convert to hex
    char hex_payload[8192];
    string_to_hex(json_payload, hex_payload);
    
    printf("📋 Sensor Data: %s\n", sensor_data);
    printf("📋 JWS Token: %.100s...\n", jws_buffer);
    printf("📋 JSON: %.200s...\n", json_payload);
    printf("📦 HEX:  %.200s...\n", hex_payload);
    printf("📏 Length: %zu characters\n", strlen(hex_payload));
    printf("\n🚀 Ready for submission to lcore-node!\n\n");
    
    lcore_did_free(did_doc);
}

void generate_test_script() {
    printf("=== Test Script Generation ===\n");
    
    printf("# Copy these commands to test with live lcore-node:\n\n");
    
    // Device registration
    lcore_did_document_t* did_doc = lcore_did_create(test_public_key, sizeof(test_public_key));
    char did_string[256];
    size_t did_len = sizeof(did_string);
    lcore_did_to_string(did_doc, did_string, &did_len);
    
    char reg_json[1024];
    snprintf(reg_json, sizeof(reg_json),
        "{\"type\":\"register_device\",\"device_id\":\"%s\",\"did_document\":\"{\\\"id\\\":\\\"%s\\\"}\"}",
        did_string, did_string);
    
    char reg_hex[2048];
    string_to_hex(reg_json, reg_hex);
    
    printf("# 1. Submit device registration\n");
    printf("curl -X POST 'https://lcore-iot-core.fly.dev/advance' \\\n");
    printf("  -H 'Content-Type: application/json' \\\n");
    printf("  -d '%s'\n\n", reg_hex);
    
    // Sensor data
    const char* sensor_data = "{\"temperature\":25.1,\"humidity\":48}";
    char jws_buffer[2048];
    size_t jws_len = sizeof(jws_buffer);
    
    lcore_jose_sign(
        (const uint8_t*)sensor_data, strlen(sensor_data),
        test_private_key, sizeof(test_private_key),
        LCORE_JOSE_ALG_ES256,
        jws_buffer, &jws_len
    );
    
    char sensor_json[4096];
    snprintf(sensor_json, sizeof(sensor_json),
        "{\"type\":\"submit_sensor_data\",\"device_id\":\"%s\",\"encrypted_payload\":\"%s\"}",
        did_string, jws_buffer);
    
    char sensor_hex[8192];
    string_to_hex(sensor_json, sensor_hex);
    
    printf("# 2. Submit sensor data\n");
    printf("curl -X POST 'https://lcore-iot-core.fly.dev/advance' \\\n");
    printf("  -H 'Content-Type: application/json' \\\n");
    printf("  -d '%.200s...'\n\n", sensor_hex);
    
    printf("# 3. Check results\n");
    printf("curl -X POST 'https://lcore-iot-core.fly.dev/graphql' \\\n");
    printf("  -H 'Content-Type: application/json' \\\n");
    printf("  -d '{\"query\":\"{ inputs { totalCount } }\"}'\n\n");
    
    lcore_did_free(did_doc);
}

int main(int argc, char* argv[]) {
    printf("🔧 Device SDK → lcore-node Payload Generator\n");
    printf("=============================================\n\n");
    
    if (argc > 1 && strcmp(argv[1], "script") == 0) {
        generate_test_script();
        return 0;
    }
    
    generate_device_registration_payload();
    generate_sensor_data_payload();
    
    printf("💡 Run with 'script' argument to generate curl commands:\n");
    printf("   ./tools/generate_test_payloads script\n\n");
    
    return 0;
} 