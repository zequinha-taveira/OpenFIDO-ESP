#ifndef CRYPTO_HAL_H
#define CRYPTO_HAL_H

#include <stdint.h>
#include <stddef.h>

// RNG
int hal_rng_generate(uint8_t *buf, size_t len);

// SHA-256
int hal_sha256(const uint8_t *input, size_t len, uint8_t output[32]);

// ECC P-256
int hal_ecc_generate_keypair(uint8_t *private_key, uint8_t *public_key);
int hal_ecc_sign(const uint8_t *private_key, const uint8_t *hash, uint8_t *signature);

// AES-256-GCM (To be implemented)
int hal_aes_gcm_encrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *input, size_t length,
                        uint8_t *output, uint8_t *tag, size_t tag_len);

int hal_aes_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *input, size_t length,
                        uint8_t *output, const uint8_t *tag, size_t tag_len);

#endif // CRYPTO_HAL_H
