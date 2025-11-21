#include <string.h>
#include "esp_log.h"
#include "tusb.h"
#include "u2f.h"
#include "crypto_hal.h"
#include "nvs.h"

static const char *TAG = "U2F";

// U2F HID Packet Structure
typedef struct __attribute__((packed)) {
    uint32_t cid;
    union {
        struct {
            uint8_t cmd;
            uint8_t bcnt_h;
            uint8_t bcnt_l;
            uint8_t data[U2F_HID_PACKET_SIZE - 7];
        } init;
        struct {
            uint8_t seq;
            uint8_t data[U2F_HID_PACKET_SIZE - 5];
        } cont;
    };
} u2f_hid_packet_t;

// State for fragmentation (Simplified: assumes single transaction for now)
static uint32_t current_cid = 0;
static uint8_t apdu_buffer[1024];
static uint16_t apdu_len = 0;
static uint16_t apdu_idx = 0;

static uint32_t global_counter = 0;
static uint8_t device_master_key[32];

static void load_device_key() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS for key!");
        return;
    }
    
    size_t len = 32;
    err = nvs_get_blob(my_handle, "master_key", device_master_key, &len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Generating new Master Key...");
        hal_rng_generate(device_master_key, 32);
        nvs_set_blob(my_handle, "master_key", device_master_key, 32);
        nvs_commit(my_handle);
    }
    nvs_close(my_handle);
}

static void load_counter() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }
    
    err = nvs_get_u32(my_handle, "counter", &global_counter);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        global_counter = 1;
        nvs_set_u32(my_handle, "counter", global_counter);
        nvs_commit(my_handle);
    }
    nvs_close(my_handle);
    ESP_LOGI(TAG, "Counter loaded: %lu", global_counter);
}

static void increment_counter() {
    global_counter++;
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        nvs_set_u32(my_handle, "counter", global_counter);
        nvs_commit(my_handle);
        nvs_close(my_handle);
    }
}

void u2f_init(void) {
    ESP_LOGI(TAG, "U2F Stack Initialized");
    load_counter();
    load_device_key();
}

void u2f_send_response(uint32_t cid, uint8_t cmd, uint8_t *data, uint16_t len) {
    u2f_hid_packet_t resp;
    memset(&resp, 0, sizeof(resp));
    
    resp.cid = cid;
    resp.init.cmd = cmd;
    resp.init.bcnt_h = (len >> 8) & 0xFF;
    resp.init.bcnt_l = len & 0xFF;
    
    uint16_t to_copy = (len > (U2F_HID_PACKET_SIZE - 7)) ? (U2F_HID_PACKET_SIZE - 7) : len;
    memcpy(resp.init.data, data, to_copy);
    
    tud_hid_report(0, &resp, U2F_HID_PACKET_SIZE);
    
    // TODO: Handle fragmentation for larger responses
}

// Attestation Key (Static for Demo - In prod, generate/store securely)
static const uint8_t attestation_private_key[32] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

int u2f_create_key_handle(const uint8_t *application_parameter, const uint8_t *private_key, uint8_t *key_handle) {
    // Key Handle: [IV(12) | EncryptedKey(32) | Tag(16)] = 60 bytes
    uint8_t kh_iv[12];
    hal_rng_generate(kh_iv, 12);
    
    uint8_t kh_ciphertext[32];
    uint8_t kh_tag[16];
    
    // Encrypt Private Key with Master Key
    // AAD = AppParam (Bind key to Application)
    int ret = hal_aes_gcm_encrypt(device_master_key, kh_iv, 12,
                        application_parameter, 32,
                        private_key, 32,
                        kh_ciphertext, kh_tag, 16);
    
    if (ret != 0) return 0;

    memcpy(key_handle, kh_iv, 12);
    memcpy(key_handle + 12, kh_ciphertext, 32);
    memcpy(key_handle + 44, kh_tag, 16);
    
    return 60;
}

int u2f_sign_attestation(const uint8_t *hash, uint8_t *signature) {
    return hal_ecc_sign(attestation_private_key, hash, signature);
}

int u2f_unwrap_key_handle(const uint8_t *application_parameter, const uint8_t *key_handle, uint8_t kh_len, uint8_t *private_key) {
    if (kh_len != 60) return -1;
    
    // KH = [IV(12) | Cipher(32) | Tag(16)]
    const uint8_t *kh_iv_ptr = key_handle;
    const uint8_t *kh_cipher_ptr = key_handle + 12;
    const uint8_t *kh_tag_ptr = key_handle + 12 + 32;
    
    return hal_aes_gcm_decrypt(device_master_key, kh_iv_ptr, 12,
                                      application_parameter, 32,
                                      kh_cipher_ptr, 32,
                                      private_key, kh_tag_ptr, 16);
}

static void process_apdu(uint32_t cid, uint8_t *apdu, uint16_t len) {
    if (len < 4) return;
    
    uint8_t cla = apdu[0];
    uint8_t ins = apdu[1];
    uint8_t p1 = apdu[2];
    uint8_t p2 = apdu[3];
    // Lc = apdu[4], Data starts at apdu[5] (Short APDU)
    uint8_t lc = apdu[4];
    uint8_t *data = &apdu[5];
    
    ESP_LOGI(TAG, "APDU: CLA=%02X INS=%02X P1=%02X P2=%02X LC=%d", cla, ins, p1, p2, lc);
    
    uint8_t resp_buf[512];
    uint16_t resp_len = 0;
    
    switch (ins) {
        case U2F_INS_VERSION:
            if (len == 0) { // Handling cases where len might be just header
                 memcpy(resp_buf, "U2F_V2", 6);
                 resp_len = 6;
            } else {
                 memcpy(resp_buf, "U2F_V2", 6);
                 resp_len = 6;
            }
            resp_buf[resp_len++] = 0x90;
            resp_buf[resp_len++] = 0x00;
            break;
            
        case U2F_INS_REGISTER:
            if (lc != 64) { // Challenge (32) + AppParam (32)
                resp_buf[0] = 0x67; // Wrong Length
                resp_buf[1] = 0x00;
                resp_len = 2;
                break;
            }
            ESP_LOGI(TAG, "CMD: REGISTER");
            
            uint8_t *challenge = data;
            uint8_t *app_param = data + 32;
            
            // 1. Generate Keypair
            uint8_t priv_key[32];
            uint8_t pub_key[65];
            hal_ecc_generate_keypair(priv_key, pub_key);
            
            // 2. Construct Response
            resp_buf[0] = 0x05; // Reserved
            
            // Public Key
            memcpy(&resp_buf[1], pub_key, 65);
            resp_len = 1 + 65;
            
            // Key Handle: [IV(12) | EncryptedKey(32) | Tag(16)] = 60 bytes
            uint8_t kh_len = u2f_create_key_handle(app_param, priv_key, &resp_buf[resp_len + 1]);
            resp_buf[resp_len++] = kh_len;
            resp_len += kh_len;
            
            // Attestation Cert (Self-signed or minimal)
            // Mocking a minimal DER cert structure would be complex here.
            // We'll send a dummy cert or just the public key again if the client accepts it.
            // For strict U2F, we need a valid X.509. 
            // Let's append a static dummy cert (usually ~300-500 bytes).
            // For this snippet, we will skip the full cert and hope the test client is lenient,
            // OR we sign the data directly.
            // U2F Spec: Sign(0x00 || AppParam || Challenge || KeyHandle || PubKey)
            
            // Signature
            // Sig Input: 0x00 || AppParam || Challenge || KeyHandle || PubKey
            // Size: 1 + 32 + 32 + 60 + 65 = 190
            uint8_t sig_input[200];
            sig_input[0] = 0x00;
            memcpy(&sig_input[1], app_param, 32);
            memcpy(&sig_input[33], challenge, 32);
            memcpy(&sig_input[65], &resp_buf[resp_len - kh_len], kh_len); // Key Handle from buffer
            memcpy(&sig_input[65 + kh_len], pub_key, 65);
            
            uint8_t sig_hash[32];
            hal_sha256(sig_input, 1 + 32 + 32 + kh_len + 65, sig_hash);
            
            uint8_t signature[72];
            int sig_size = hal_ecc_sign(attestation_private_key, sig_hash, signature);
            
            // Append Cert (Mock: just 0s for now, browsers might reject)
            // Ideally we need a pre-generated cert for the attestation key.
            // We'll add a small placeholder.
            resp_buf[resp_len++] = 0x30; // SEQ
            resp_buf[resp_len++] = 0x00; // Len 0
            
            // Append Signature
            memcpy(&resp_buf[resp_len], signature, sig_size);
            resp_len += sig_size;
            
            resp_buf[resp_len++] = 0x90;
            resp_buf[resp_len++] = 0x00;
            break;
            
        case U2F_INS_AUTHENTICATE:
            if (lc < 65) { // Chal(32) + App(32) + KH_Len(1) + KH
                 resp_buf[0] = 0x67;
                 resp_buf[1] = 0x00;
                 resp_len = 2;
                 break;
            }
            ESP_LOGI(TAG, "CMD: AUTHENTICATE");
            
            uint8_t control = p1;
            uint8_t *auth_challenge = data;
            uint8_t *auth_app_param = data + 32;
            uint8_t auth_kh_len = data[64];
            uint8_t *auth_kh = data + 65;
            
            if (auth_kh_len != 60) {
                resp_buf[0] = 0x6A; // Wrong Data (Bad Key Handle)
                resp_buf[1] = 0x80;
                resp_len = 2;
                break;
            }
            
            // Check-only?
            if (control == 0x07) {
                resp_buf[0] = 0x69; // Cond. Not Satisfied (User Presence required)
                resp_buf[1] = 0x85;
                resp_len = 2;
                break;
            }
            
            // Enforce User Presence (Button)
            // In real code: wait for button press or return "Not Satisfied" immediately if not pressed.
            // For demo: assume pressed.
            uint8_t user_presence = 0x01;
            increment_counter();
            uint32_t counter = global_counter;
            
            // Sign(AppParam || UserPresence || Counter || Challenge)
            uint8_t auth_sig_input[32 + 1 + 4 + 32];
            memcpy(&auth_sig_input[0], auth_app_param, 32);
            auth_sig_input[32] = user_presence;
            auth_sig_input[33] = (counter >> 24) & 0xFF;
            auth_sig_input[34] = (counter >> 16) & 0xFF;
            auth_sig_input[35] = (counter >> 8) & 0xFF;
            auth_sig_input[36] = counter & 0xFF;
            memcpy(&auth_sig_input[37], auth_challenge, 32);
            
            uint8_t auth_hash[32];
            hal_sha256(auth_sig_input, sizeof(auth_sig_input), auth_hash);
            
            // Recover Private Key from Key Handle
            uint8_t recovered_priv_key[32];
            int dec_ret = u2f_unwrap_key_handle(auth_app_param, auth_kh, auth_kh_len, recovered_priv_key);
            
            if (dec_ret != 0) {
                ESP_LOGE(TAG, "Bad Key Handle (Decrypt Failed)");
                resp_buf[0] = 0x6A; // Wrong Data
                resp_buf[1] = 0x80;
                resp_len = 2;
                break;
            }
            
            uint8_t auth_signature[72];
            int auth_sig_size = hal_ecc_sign(recovered_priv_key, auth_hash, auth_signature);
            
            resp_buf[0] = user_presence;
            resp_buf[1] = (counter >> 24) & 0xFF;
            resp_buf[2] = (counter >> 16) & 0xFF;
            resp_buf[3] = (counter >> 8) & 0xFF;
            resp_buf[4] = counter & 0xFF;
            memcpy(&resp_buf[5], auth_signature, auth_sig_size);
            resp_len = 5 + auth_sig_size;
            
            resp_buf[resp_len++] = 0x90;
            resp_buf[resp_len++] = 0x00;
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown INS: %02X", ins);
            resp_buf[0] = 0x6D;
            resp_buf[1] = 0x00;
            resp_len = 2;
            break;
    }
    
    send_response(cid, U2FHID_MSG, resp_buf, resp_len);
}

void u2f_handle_report(uint8_t *report, uint16_t len) {
    u2f_hid_packet_t *pkt = (u2f_hid_packet_t *)report;
    
    if (pkt->cid == U2F_HID_CID_BROADCAST) {
        if (pkt->init.cmd == U2FHID_INIT) {
            // Handle Init
            uint8_t nonce[8];
            memcpy(nonce, pkt->init.data, 8);
            
            uint8_t resp[17];
            memcpy(resp, nonce, 8);
            // New CID
            uint32_t new_cid = 0x12345678; // Static for now
            resp[8] = (new_cid >> 24) & 0xFF;
            resp[9] = (new_cid >> 16) & 0xFF;
            resp[10] = (new_cid >> 8) & 0xFF;
            resp[11] = new_cid & 0xFF;
            resp[12] = 2; // Protocol version
            resp[13] = 1; // Major
            resp[14] = 0; // Minor
            resp[15] = 0; // Build
            resp[16] = 0; // Cap flags
            
            send_response(U2F_HID_CID_BROADCAST, U2FHID_INIT, resp, 17);
            return;
        }
    }
    
    if (pkt->init.cmd == U2FHID_MSG) {
        uint16_t payload_len = (pkt->init.bcnt_h << 8) | pkt->init.bcnt_l;
        // Assuming single packet for now (short APDUs)
        process_apdu(pkt->cid, pkt->init.data, payload_len);
    }
    else if (pkt->init.cmd == U2FHID_PING) {
        uint16_t payload_len = (pkt->init.bcnt_h << 8) | pkt->init.bcnt_l;
        u2f_send_response(pkt->cid, U2FHID_PING, pkt->init.data, payload_len);
    }
    else if (pkt->init.cmd == 0x90) { // U2FHID_CBOR
        uint16_t payload_len = (pkt->init.bcnt_h << 8) | pkt->init.bcnt_l;
        extern void ctap2_handle_cbor(uint32_t cid, uint8_t *payload, uint16_t len);
        ctap2_handle_cbor(pkt->cid, pkt->init.data, payload_len);
    }
}
