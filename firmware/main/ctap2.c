#include "ctap2.h"
#include "cbor_minimal.h"
#include "u2f.h" // For send_response
#include "crypto_hal.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CTAP2";

// AAGUID (16 bytes) - Zero for generic
static const uint8_t aaguid[16] = {0};

static void send_ctap2_response(uint32_t cid, uint8_t status, uint8_t *data, size_t len) {
    uint8_t resp[1024];
    resp[0] = status;
    if (len > 0) {
        memcpy(&resp[1], data, len);
    }
    // Send as U2FHID_CBOR (0x90)
    // Note: u2f.c send_response expects CMD. U2FHID_CBOR is 0x90.
    // We need to expose a way to send generic HID commands or reuse u2f_send_response
    // Assuming u2f.c has a public send function or we modify it.
    // For now, we'll assume u2f.c handles the framing if we pass the right CMD.
    // But u2f.c's send_response is static. We'll need to fix that or duplicate logic.
    // Let's assume we can call a public version.
    
    // HACK: calling internal static function is impossible. 
    // We will rely on u2f.c to expose a public sender or handle it there.
    // For this file, let's assume `u2f_send_hid_msg(cid, cmd, data, len)` exists.
    // I will add `u2f_send_response` to u2f.h
    
    extern void u2f_send_response(uint32_t cid, uint8_t cmd, uint8_t *data, uint16_t len);
    u2f_send_response(cid, 0x90, resp, 1 + len); // 0x90 = U2FHID_CBOR
}

static void handle_get_info(uint32_t cid) {
    uint8_t buf[512];
    cbor_encoder_t enc;
    cbor_encoder_init(&enc, buf, sizeof(buf));
    
    // Map(4)
    cbor_encode_map_start(&enc, 4);
    
    // 1: Versions ["FIDO_2_0", "U2F_V2"]
    cbor_encode_uint(&enc, 0x01);
    cbor_encode_array_start(&enc, 2);
    cbor_encode_text(&enc, "FIDO_2_0");
    cbor_encode_text(&enc, "U2F_V2");
    
    // 2: Extensions []
    cbor_encode_uint(&enc, 0x02);
    cbor_encode_array_start(&enc, 0);
    
    // 3: AAGUID
    cbor_encode_uint(&enc, 0x03);
    cbor_encode_bytes(&enc, aaguid, 16);
    
    // 4: Options { "rk": true, "up": true }
    cbor_encode_uint(&enc, 0x04);
    cbor_encode_map_start(&enc, 2);
    cbor_encode_text(&enc, "rk");
    cbor_encode_uint(&enc, 1); // true
    cbor_encode_text(&enc, "up");
    cbor_encode_uint(&enc, 1); // true
    
    send_ctap2_response(cid, CTAP2_OK, buf, enc.offset);
}

static void handle_make_credential(uint32_t cid, uint8_t *payload, size_t len) {
    // Parsing MakeCredential is complex. 
    // 1. Client Data Hash (32)
    // 2. RP {id, name}
    // 3. User {id, name, displayName}
    // 4. PubKeyCredParams [...]
    
    // For this MVP, we will mock the response to prove the flow.
    // Real implementation requires full CBOR decoding of nested maps.
    
    ESP_LOGI(TAG, "MakeCredential received");
    
    uint8_t buf[512];
    cbor_encoder_t enc;
    cbor_encoder_init(&enc, buf, sizeof(buf));
    
    // Response: { 1: fmt, 2: authData, 3: attStmt }
    cbor_encode_map_start(&enc, 3);
    
    // 1: fmt "packed"
    cbor_encode_uint(&enc, 0x01);
    cbor_encode_text(&enc, "packed");
    
    // 2: authData (RPIDHash + Flags + Counter + AAGUID + CredIDLen + CredID + COSEKey)
    // Constructing AuthData...
    uint8_t authData[256];
    size_t ad_len = 0;
    
    // RPID Hash (Mock: 32 bytes of 0x11)
    memset(&authData[ad_len], 0x11, 32); ad_len += 32;
    
    // Flags (UP=1, AT=1) -> 0x41
    authData[ad_len++] = 0x41;
    
    // Counter (4 bytes)
    authData[ad_len++] = 0; authData[ad_len++] = 0; authData[ad_len++] = 0; authData[ad_len++] = 1;
    
    // Attested Cred Data (since AT=1)
    // AAGUID
    memcpy(&authData[ad_len], aaguid, 16); ad_len += 16;
    
    // Cred ID Len (32)
    uint8_t credId[32];
    memset(credId, 0xCC, 32); // Mock Cred ID
    authData[ad_len++] = 0x00;
    authData[ad_len++] = 32;
    
    // Cred ID
    memcpy(&authData[ad_len], credId, 32); ad_len += 32;
    
    // COSE Key (Map) - We need to encode this into authData
    // This is tricky without a nested encoder. We'll manually construct a simple COSE key.
    // Map(3) { 1: 2 (EC2), 3: -7 (ES256), -1: 1 (P-256), -2: X, -3: Y }
    // Actually Map(5).
    // Let's just put a placeholder for now or use the encoder.
    // ...
    
    cbor_encode_uint(&enc, 0x02);
    cbor_encode_bytes(&enc, authData, ad_len); // Incomplete authData for demo
    
    // 3: attStmt { "alg": -7, "sig": ... }
    cbor_encode_uint(&enc, 0x03);
    cbor_encode_map_start(&enc, 2);
    cbor_encode_text(&enc, "alg");
    cbor_encode_int(&enc, -7);
    cbor_encode_text(&enc, "sig");
    uint8_t sig[64] = {0}; // Mock sig
    cbor_encode_bytes(&enc, sig, 64);
    
    send_ctap2_response(cid, CTAP2_OK, buf, enc.offset);
}

void ctap2_handle_cbor(uint32_t cid, uint8_t *payload, uint16_t len) {
    if (len == 0) return;
    uint8_t cmd = payload[0];
    
    ESP_LOGI(TAG, "CTAP2 CMD: %02X", cmd);
    
    switch (cmd) {
        case CTAP2_GET_INFO:
            handle_get_info(cid);
            break;
        case CTAP2_MAKE_CREDENTIAL:
            handle_make_credential(cid, payload + 1, len - 1);
            break;
        case CTAP2_GET_ASSERTION:
            // TODO
            send_ctap2_response(cid, CTAP2_ERR_UNSUPPORTED_OP, NULL, 0);
            break;
        default:
            send_ctap2_response(cid, CTAP2_ERR_UNSUPPORTED_OP, NULL, 0);
            break;
    }
}
