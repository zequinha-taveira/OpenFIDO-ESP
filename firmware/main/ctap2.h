#pragma once

#include <stdint.h>

// CTAP2 Commands
#define CTAP2_MAKE_CREDENTIAL   0x01
#define CTAP2_GET_ASSERTION     0x02
#define CTAP2_GET_INFO          0x04
#define CTAP2_CLIENT_PIN        0x06
#define CTAP2_RESET             0x07
#define CTAP2_GET_NEXT_ASSERT   0x08

// CTAP2 Status Codes
#define CTAP2_OK                0x00
#define CTAP2_ERR_INVALID_CBOR  0x12
#define CTAP2_ERR_MISSING_PARAM 0x14
#define CTAP2_ERR_UNSUPPORTED_OP 0x2B

void ctap2_handle_cbor(uint32_t cid, uint8_t *payload, uint16_t len);
