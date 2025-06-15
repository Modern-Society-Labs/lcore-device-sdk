#ifndef LCORE_JOSE_H
#define LCORE_JOSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Supported JOSE signing algorithms.
 */
typedef enum {
    LCORE_JOSE_ALG_ES256,
    LCORE_JOSE_ALG_ES512,
} lcore_jose_alg_t;

/**
 * @brief Signs a payload using the specified algorithm and key.
 *
 * @param[in] payload The data to sign.
 * @param[in] payload_len The length of the data.
 * @param[in] private_key The private key to sign with.
 * @param[in] key_len The length of the private key.
 * @param[in] alg The signing algorithm to use.
 * @param[out] buffer The buffer to write the JWS to.
 * @param[in,out] buffer_len The size of the buffer, updated with the actual size.
 * @return 0 on success, non-zero on failure.
 */
int lcore_jose_sign(
    const uint8_t* payload,
    size_t payload_len,
    const uint8_t* private_key,
    size_t key_len,
    lcore_jose_alg_t alg,
    char* buffer,
    size_t* buffer_len
);

/**
 * @brief Verifies a JWS signature.
 *
 * @param[in] jws The JWS string to verify.
 * @param[in] jws_len The length of the JWS string.
 * @param[in] public_key The public key to verify with.
 * @param[in] key_len The length of the public key.
 * @param[out] payload_buffer Buffer to store the extracted payload.
 * @param[in,out] payload_len The size of the payload buffer, updated with the actual size.
 * @return 0 on success (signature is valid), non-zero on failure.
 */
int lcore_jose_verify(
    const char* jws,
    size_t jws_len,
    const uint8_t* public_key,
    size_t key_len,
    uint8_t* payload_buffer,
    size_t* payload_len
);

#ifdef __cplusplus
}
#endif

#endif // LCORE_JOSE_H 