#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// CBOR Major Types
#define CBOR_UINT   0x00
#define CBOR_NEGINT 0x20
#define CBOR_BYTES  0x40
#define CBOR_TEXT   0x60
#define CBOR_ARRAY  0x80
#define CBOR_MAP    0xA0

// Encoder
typedef struct {
    uint8_t *buf;
    size_t size;
    size_t offset;
} cbor_encoder_t;

void cbor_encoder_init(cbor_encoder_t *enc, uint8_t *buf, size_t size);
void cbor_encode_uint(cbor_encoder_t *enc, uint64_t val);
void cbor_encode_int(cbor_encoder_t *enc, int64_t val);
void cbor_encode_bytes(cbor_encoder_t *enc, const uint8_t *data, size_t len);
void cbor_encode_text(cbor_encoder_t *enc, const char *text);
void cbor_encode_map_start(cbor_encoder_t *enc, size_t len);
void cbor_encode_array_start(cbor_encoder_t *enc, size_t len);

// Decoder (Simplified for FIDO2 flat maps)
typedef struct {
    const uint8_t *buf;
    size_t size;
    size_t offset;
} cbor_decoder_t;

void cbor_decoder_init(cbor_decoder_t *dec, const uint8_t *buf, size_t size);
bool cbor_decode_uint(cbor_decoder_t *dec, uint64_t *val);
bool cbor_decode_bytes(cbor_decoder_t *dec, const uint8_t **data, size_t *len);
bool cbor_decode_text(cbor_decoder_t *dec, const char **text, size_t *len);
bool cbor_decode_map_header(cbor_decoder_t *dec, size_t *size);
bool cbor_decode_array_header(cbor_decoder_t *dec, size_t *size);
int cbor_peek_major_type(cbor_decoder_t *dec);
