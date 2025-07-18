#include <lcore/did.h>
#include <mbedtls/sha256.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Internal struct definition for the opaque type.
struct lcore_did_document {
    uint8_t* key_material;
    size_t key_material_len;
    char* did_string;
};

lcore_did_document_t* lcore_did_create(const uint8_t* key_material, size_t key_material_len) {
    if (!key_material || key_material_len == 0) {
        return NULL;
    }

    lcore_did_document_t* doc = calloc(1, sizeof(lcore_did_document_t));
    if (!doc) {
        return NULL;
    }

    doc->key_material = malloc(key_material_len);
    if (!doc->key_material) {
        free(doc);
        return NULL;
    }

    memcpy(doc->key_material, key_material, key_material_len);
    doc->key_material_len = key_material_len;

    // Generate real DID from public key material
    // 1. Hash the public key with SHA-256 using MbedTLS
    uint8_t hash[32]; // SHA-256 digest length is 32 bytes
    mbedtls_sha256(key_material, key_material_len, hash, 0); // 0 = SHA-256 (not SHA-224)
    
    // 2. Take first 16 bytes and encode as hex
    char key_id[33]; // 32 hex chars + null terminator
    for (int i = 0; i < 16; i++) {
        sprintf(&key_id[i*2], "%02x", hash[i]);
    }
    
    // 3. Create DID string: did:lcore:<key-id>
    char did_string[256];
    snprintf(did_string, sizeof(did_string), "did:lcore:%s", key_id);
    
    doc->did_string = strdup(did_string);
    if (!doc->did_string) {
        free(doc->key_material);
        free(doc);
        return NULL;
    }

    return doc;
}

void lcore_did_free(lcore_did_document_t* doc) {
    if (doc) {
        free(doc->key_material);
        free(doc->did_string);
        free(doc);
    }
}

int lcore_did_to_string(const lcore_did_document_t* doc, char* buffer, size_t* len) {
    if (!doc || !buffer || !len) {
        return -1;
    }

    size_t did_len = strlen(doc->did_string);
    if (*len < did_len + 1) {
        *len = did_len + 1;
        return -2; // Buffer too small
    }

    strcpy(buffer, doc->did_string);
    *len = did_len;
    return 0;
} 