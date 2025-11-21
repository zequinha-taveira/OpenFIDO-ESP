#include "esp_system.h"
#include "esp_log.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/gcm.h"

static const char *TAG = "CRYPTO_HAL";

// Hardware Random Number Generator Wrapper
int hal_rng_generate(uint8_t *buf, size_t len) {
    // ESP32-S2 has a hardware TRNG enabled by default in the Wi-Fi/BT stack or bootloader.
    // esp_fill_random() uses this hardware RNG.
    esp_fill_random(buf, len);
    return 0; // Success
}

// SHA-256 Wrapper
int hal_sha256(const uint8_t *input, size_t len, uint8_t output[32]) {
    int ret = mbedtls_sha256(input, len, output, 0); // 0 = SHA-256 (not 224)
    if (ret != 0) {
        ESP_LOGE(TAG, "SHA256 Failed: -0x%04X", -ret);
    }
    return ret;
}

// ECC P-256 Key Generation
int hal_ecc_generate_keypair(uint8_t *private_key, uint8_t *public_key) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context pk;
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_pk_init(&pk);

    // Seed RNG
    // Note: In ESP32, mbedtls_entropy_func uses hardware RNG automatically
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    if (ret != 0) goto exit;

    // Generate Keypair (SECP256R1)
    ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) goto exit;

    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(pk), 
                              mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) goto exit;

    // Export Private Key (32 bytes)
    ret = mbedtls_mpi_write_binary(&mbedtls_pk_ec(pk)->d, private_key, 32);
    if (ret != 0) goto exit;

    // Export Public Key (65 bytes: 0x04 + X + Y)
    size_t olen;
    ret = mbedtls_ecp_point_write_binary(&mbedtls_pk_ec(pk)->grp, &mbedtls_pk_ec(pk)->Q,
                                         MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, public_key, 65);
    if (ret != 0) goto exit;

exit:
    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    
    if (ret != 0) {
        ESP_LOGE(TAG, "ECC Gen Failed: -0x%04X", -ret);
    }
    return ret;
}

// ECC P-256 Sign
int hal_ecc_sign(const uint8_t *private_key, const uint8_t *hash, uint8_t *signature) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context pk;
    mbedtls_mpi r, s;
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_pk_init(&pk);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    if (ret != 0) goto exit;

    // Import Private Key
    ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) goto exit;
    
    mbedtls_ecp_group_load(&mbedtls_pk_ec(pk)->grp, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_mpi_read_binary(&mbedtls_pk_ec(pk)->d, private_key, 32);

    // Sign Hash
    ret = mbedtls_ecdsa_sign(&mbedtls_pk_ec(pk)->grp, &r, &s, &mbedtls_pk_ec(pk)->d,
                             hash, 32, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) goto exit;

    // Encode Signature in ASN.1 DER (U2F requirement)
    // Sequence { Integer r, Integer s }
    // We'll use a simplified manual encoding or mbedtls_ecdsa_write_signature if available.
    // mbedtls_ecdsa_write_signature writes full ASN.1 signature.
    
    size_t sig_len;
    ret = mbedtls_ecdsa_write_signature(&mbedtls_pk_ec(pk)->grp, MBEDTLS_MD_SHA256,
                                        hash, 32, signature, &sig_len,
                                        mbedtls_ctr_drbg_random, &ctr_drbg);
    // Note: signature buffer should be large enough (approx 72 bytes)
    
exit:
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    if (ret != 0) {
        ESP_LOGE(TAG, "ECC Sign Failed: -0x%04X", -ret);
        return ret;
    }
    return sig_len; // Return actual signature length
}

// AES-256-GCM Encrypt
int hal_aes_gcm_encrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *input, size_t length,
                        uint8_t *output, uint8_t *tag, size_t tag_len) {
    mbedtls_gcm_context ctx;
    int ret;

    mbedtls_gcm_init(&ctx);
    
    // 256-bit key = 32 bytes * 8 = 256 bits
    ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 256);
    if (ret != 0) goto exit;

    ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, length,
                                    iv, iv_len, aad, aad_len,
                                    input, output, tag_len, tag);

exit:
    mbedtls_gcm_free(&ctx);
    if (ret != 0) {
        ESP_LOGE(TAG, "AES GCM Encrypt Failed: -0x%04X", -ret);
    }
    return ret;
}

// AES-256-GCM Decrypt
int hal_aes_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *input, size_t length,
                        uint8_t *output, const uint8_t *tag, size_t tag_len) {
    mbedtls_gcm_context ctx;
    int ret;

    mbedtls_gcm_init(&ctx);

    ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 256);
    if (ret != 0) goto exit;

    ret = mbedtls_gcm_auth_decrypt(&ctx, length,
                                   iv, iv_len, aad, aad_len,
                                   tag, tag_len,
                                   input, output);

exit:
    mbedtls_gcm_free(&ctx);
    if (ret != 0) {
        ESP_LOGE(TAG, "AES GCM Decrypt Failed: -0x%04X", -ret);
    }
    return ret;
}
