#include <lcore/jose.h>
#include <psa/crypto.h>
#include <mbedtls/base64.h>
#include <string.h>
#include <stdlib.h>

// ARM PSA approach (IoTeX pattern) - RISC-V compatible

// Base64URL encoding helper (without padding)
static int base64url_encode(const uint8_t* input, size_t input_len, char* output, size_t output_size) {
    size_t olen = 0;
    int ret = mbedtls_base64_encode((unsigned char*)output, output_size, &olen, input, input_len);
    if (ret != 0) {
        return -1;
    }
    
    // Convert to base64url (replace + with -, / with _, remove padding)
    for (size_t i = 0; i < olen; i++) {
        if (output[i] == '+') output[i] = '-';
        else if (output[i] == '/') output[i] = '_';
        else if (output[i] == '=') {
            output[i] = '\0';
            break;
        }
    }
    
    return 0;
}

// Base64URL decoding helper
static int base64url_decode(const char* input, uint8_t* output, size_t* output_len) {
    size_t input_len = strlen(input);
    char* padded_input = malloc(input_len + 4); // Add space for padding
    if (!padded_input) {
        return -1;
    }
    
    strcpy(padded_input, input);
    
    // Convert base64url to base64
    for (size_t i = 0; i < input_len; i++) {
        if (padded_input[i] == '-') padded_input[i] = '+';
        else if (padded_input[i] == '_') padded_input[i] = '/';
    }
    
    // Add padding if needed
    size_t padding = (4 - (input_len % 4)) % 4;
    for (size_t i = 0; i < padding; i++) {
        strcat(padded_input, "=");
    }
    
    size_t decoded_len = *output_len;
    int ret = mbedtls_base64_decode(output, *output_len, &decoded_len, 
                                   (const unsigned char*)padded_input, strlen(padded_input));
    
    free(padded_input);
    
    if (ret != 0) {
        return -1;
    }
    
    *output_len = decoded_len;
    return 0;
}

int lcore_jose_sign(
    const uint8_t* payload,
    size_t payload_len,
    const uint8_t* private_key,
    size_t key_len,
    lcore_jose_alg_t alg,
    char* buffer,
    size_t* buffer_len
) {
    if (!payload || !private_key || !buffer || !buffer_len) {
        return -1;
    }

    // Initialize PSA crypto (IoTeX pattern)
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        return -1;
    }

    // Create JWS header for ES256
    const char* header = "{\"alg\":\"ES256\",\"typ\":\"JWT\"}";
    
    // Base64URL encode header
    char header_b64[256];
    if (base64url_encode((const uint8_t*)header, strlen(header), header_b64, sizeof(header_b64)) != 0) {
        return -1;
    }
    
    // Base64URL encode payload
    char payload_b64[1024];
    if (base64url_encode(payload, payload_len, payload_b64, sizeof(payload_b64)) != 0) {
        return -1;
    }
    
    // Create signing input: header.payload
    char signing_input[2048];
    snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);
    
    // Import private key for PSA (IoTeX pattern)
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256); // P-256
    
    psa_key_id_t key_id;
    status = psa_import_key(&attributes, private_key, key_len, &key_id);
    if (status != PSA_SUCCESS) {
        psa_reset_key_attributes(&attributes);
        return -1;
    }
    
    // Generate ECDSA signature using ARM PSA (IoTeX pattern)
    uint8_t signature[64]; // P-256 signature is typically 64 bytes
    size_t signature_length;
    
    status = psa_sign_message(
        key_id,
        PSA_ALG_ECDSA(PSA_ALG_SHA_256),
        (const uint8_t*)signing_input, strlen(signing_input),
        signature, sizeof(signature), &signature_length
    );
    
    // Clean up key
    psa_destroy_key(key_id);
    psa_reset_key_attributes(&attributes);
    
    if (status != PSA_SUCCESS) {
        return -1;
    }
    
    // Base64URL encode signature
    char sig_b64[128];
    if (base64url_encode(signature, signature_length, sig_b64, sizeof(sig_b64)) != 0) {
        return -1;
    }
    
    // Create final JWS: header.payload.signature
    int jws_len = snprintf(buffer, *buffer_len, "%s.%s.%s", header_b64, payload_b64, sig_b64);
    if (jws_len >= (int)*buffer_len) {
        *buffer_len = jws_len + 1;
        return -2; // Buffer too small
    }

    *buffer_len = jws_len;
    return 0;
}

int lcore_jose_verify(
    const char* jws,
    size_t jws_len,
    const uint8_t* public_key,
    size_t key_len,
    uint8_t* payload_buffer,
    size_t* payload_len
) {
    if (!jws || !public_key || !payload_buffer || !payload_len) {
        return -1;
    }

    // Initialize PSA crypto
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        return -1;
    }

    // Split JWS into header.payload.signature
    char *jws_copy = strndup(jws, jws_len);
    if (!jws_copy) {
        return -1;
    }
    
    char *header_b64 = strtok(jws_copy, ".");
    char *payload_b64 = strtok(NULL, ".");
    char *sig_b64 = strtok(NULL, ".");
    
    if (!header_b64 || !payload_b64 || !sig_b64) {
        free(jws_copy);
        return -1; // Invalid JWS format
    }
    
    // Decode signature
    uint8_t signature[128];
    size_t sig_len = sizeof(signature);
    if (base64url_decode(sig_b64, signature, &sig_len) != 0) {
        free(jws_copy);
        return -1;
    }
    
    // Create signing input for verification
    char signing_input[2048];
    snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);
    
    // Import public key for PSA (IoTeX pattern)
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256); // P-256
    
    psa_key_id_t key_id;
    status = psa_import_key(&attributes, public_key, key_len, &key_id);
    if (status != PSA_SUCCESS) {
        psa_reset_key_attributes(&attributes);
        free(jws_copy);
        return -1;
    }
    
    // Verify signature using ARM PSA
    status = psa_verify_message(
        key_id,
        PSA_ALG_ECDSA(PSA_ALG_SHA_256),
        (const uint8_t*)signing_input, strlen(signing_input),
        signature, sig_len
    );
    
    // Clean up key
    psa_destroy_key(key_id);
    psa_reset_key_attributes(&attributes);
    
    if (status == PSA_SUCCESS) {
        // Signature is valid, decode payload
        size_t decoded_len = *payload_len;
        if (base64url_decode(payload_b64, payload_buffer, &decoded_len) != 0) {
            free(jws_copy);
            return -1;
        }
        *payload_len = decoded_len;
    }
    
    free(jws_copy);
    return (status == PSA_SUCCESS) ? 0 : -1;
} 