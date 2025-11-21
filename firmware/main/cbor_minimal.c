#include "cbor_minimal.h"
#include <string.h>

// --- Encoder ---

void cbor_encoder_init(cbor_encoder_t *enc, uint8_t *buf, size_t size) {
    enc->buf = buf;
    enc->size = size;
    enc->offset = 0;
}

static void encode_head(cbor_encoder_t *enc, uint8_t major, uint64_t val) {
    if (enc->offset >= enc->size) return;

    if (val < 24) {
        enc->buf[enc->offset++] = major | (uint8_t)val;
    } else if (val <= 0xFF) {
        if (enc->offset + 2 > enc->size) return;
        enc->buf[enc->offset++] = major | 24;
        enc->buf[enc->offset++] = (uint8_t)val;
    } else if (val <= 0xFFFF) {
        if (enc->offset + 3 > enc->size) return;
        enc->buf[enc->offset++] = major | 25;
        enc->buf[enc->offset++] = (uint8_t)(val >> 8);
        enc->buf[enc->offset++] = (uint8_t)val;
    } else if (val <= 0xFFFFFFFF) {
        if (enc->offset + 5 > enc->size) return;
        enc->buf[enc->offset++] = major | 26;
        enc->buf[enc->offset++] = (uint8_t)(val >> 24);
        enc->buf[enc->offset++] = (uint8_t)(val >> 16);
        enc->buf[enc->offset++] = (uint8_t)(val >> 8);
        enc->buf[enc->offset++] = (uint8_t)val;
    } else {
        // 64-bit not implemented for minimal
    }
}

void cbor_encode_uint(cbor_encoder_t *enc, uint64_t val) {
    encode_head(enc, CBOR_UINT, val);
}

void cbor_encode_int(cbor_encoder_t *enc, int64_t val) {
    if (val >= 0) {
        encode_head(enc, CBOR_UINT, (uint64_t)val);
    } else {
        encode_head(enc, CBOR_NEGINT, (uint64_t)(-1 - val));
    }
}

void cbor_encode_bytes(cbor_encoder_t *enc, const uint8_t *data, size_t len) {
    encode_head(enc, CBOR_BYTES, len);
    if (enc->offset + len <= enc->size) {
        memcpy(enc->buf + enc->offset, data, len);
        enc->offset += len;
    }
}

void cbor_encode_text(cbor_encoder_t *enc, const char *text) {
    size_t len = strlen(text);
    encode_head(enc, CBOR_TEXT, len);
    if (enc->offset + len <= enc->size) {
        memcpy(enc->buf + enc->offset, text, len);
        enc->offset += len;
    }
}

void cbor_encode_map_start(cbor_encoder_t *enc, size_t len) {
    encode_head(enc, CBOR_MAP, len);
}

void cbor_encode_array_start(cbor_encoder_t *enc, size_t len) {
    encode_head(enc, CBOR_ARRAY, len);
}

// --- Decoder ---

void cbor_decoder_init(cbor_decoder_t *dec, const uint8_t *buf, size_t size) {
    dec->buf = buf;
    dec->size = size;
    dec->offset = 0;
}

// Simplistic decoder helpers
static uint8_t peek(cbor_decoder_t *dec) {
    if (dec->offset >= dec->size) return 0xFF;
    return dec->buf[dec->offset];
}

static uint8_t read_byte(cbor_decoder_t *dec) {
    if (dec->offset >= dec->size) return 0;
    return dec->buf[dec->offset++];
}

bool cbor_decode_uint(cbor_decoder_t *dec, uint64_t *val) {
    uint8_t head = read_byte(dec);
    uint8_t major = head & 0xE0;
    uint8_t info = head & 0x1F;
    
    if (major != CBOR_UINT) return false;
    
    if (info < 24) {
        *val = info;
    } else if (info == 24) {
        *val = read_byte(dec);
    } else if (info == 25) {
        *val = ((uint64_t)read_byte(dec) << 8) | read_byte(dec);
    } else {
        return false; // Not supporting > 16 bit for minimal decoder yet
    }
    return true;
}

bool cbor_decode_bytes(cbor_decoder_t *dec, const uint8_t **data, size_t *len) {
    uint8_t head = read_byte(dec);
    uint8_t major = head & 0xE0;
    uint8_t info = head & 0x1F;
    
    if (major != CBOR_BYTES) return false;
    
    size_t l = 0;
    if (info < 24) {
        l = info;
    } else if (info == 24) {
        l = read_byte(dec);
    } else if (info == 25) {
        l = ((size_t)read_byte(dec) << 8) | read_byte(dec);
    }
    
    if (dec->offset + l > dec->size) return false;
    
    *data = dec->buf + dec->offset;
    *len = l;
    dec->offset += l;
    return true;
}

bool cbor_decode_text(cbor_decoder_t *dec, const char **text, size_t *len) {
    uint8_t head = read_byte(dec);
    uint8_t major = head & 0xE0;
    uint8_t info = head & 0x1F;
    
    if (major != CBOR_TEXT) return false;
    
    size_t l = 0;
    if (info < 24) {
        l = info;
    } else if (info == 24) {
        l = read_byte(dec);
    } else if (info == 25) {
        l = ((size_t)read_byte(dec) << 8) | read_byte(dec);
    }
    
    if (dec->offset + l > dec->size) return false;
    
    *text = (const char *)(dec->buf + dec->offset);
    *len = l;
    dec->offset += l;
    return true;
}

bool cbor_decode_map_header(cbor_decoder_t *dec, size_t *size) {
    uint8_t head = peek(dec);
    uint8_t major = head & 0xE0;
    uint8_t info = head & 0x1F;
    
    if (major != CBOR_MAP) return false;
    read_byte(dec); // Consume head
    
    if (info < 24) {
        *size = info;
    } else if (info == 24) {
        *size = read_byte(dec);
    } else {
        return false; // Simplified
    }
    return true;
}

bool cbor_decode_array_header(cbor_decoder_t *dec, size_t *size) {
    uint8_t head = peek(dec);
    uint8_t major = head & 0xE0;
    uint8_t info = head & 0x1F;
    
    if (major != CBOR_ARRAY) return false;
    read_byte(dec); // Consume head
    
    if (info < 24) {
        *size = info;
    } else if (info == 24) {
        *size = read_byte(dec);
    } else {
        return false; // Simplified
    }
    return true;
}

int cbor_peek_major_type(cbor_decoder_t *dec) {
    uint8_t head = peek(dec);
    if (head == 0xFF) return -1;
    return head & 0xE0;
}
