#ifndef LCORE_DID_H
#define LCORE_DID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque structure representing a DID document.
 */
typedef struct lcore_did_document lcore_did_document_t;

/**
 * @brief Creates a new DID document.
 *
 * @param[in] key_material The public key material to include.
 * @param[in] key_material_len The length of the key material.
 * @return A pointer to the new DID document, or NULL on failure.
 */
lcore_did_document_t* lcore_did_create(const uint8_t* key_material, size_t key_material_len);

/**
 * @brief Frees a DID document.
 *
 * @param[in] doc The DID document to free.
 */
void lcore_did_free(lcore_did_document_t* doc);

/**
 * @brief Serializes a DID document to its string representation.
 *
 * @param[in] doc The DID document to serialize.
 * @param[out] buffer The buffer to write the string to.
 * @param[in,out] len The size of the buffer, updated with the actual size.
 * @return 0 on success, non-zero on failure.
 */
int lcore_did_to_string(const lcore_did_document_t* doc, char* buffer, size_t* len);

#ifdef __cplusplus
}
#endif

#endif // LCORE_DID_H 