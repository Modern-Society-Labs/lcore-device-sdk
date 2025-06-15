#include <lcore/did.h>
#include <stdlib.h>
#include <string.h>

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

    // In a real implementation, we would generate the DID string based on the key.
    // For this placeholder, we'll use a fixed string.
    const char* placeholder_did = "did:lcore:123456789abcdef";
    doc->did_string = strdup(placeholder_did);
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