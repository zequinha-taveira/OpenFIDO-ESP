#pragma once

#include <stdint.h>

// U2F HID Constants
#define U2F_HID_CID_BROADCAST   0xFFFFFFFF
#define U2F_HID_PACKET_SIZE     64

// U2F HID Commands
#define U2FHID_PING         (0x80 | 0x01)
#define U2FHID_MSG          (0x80 | 0x03)
#define U2FHID_LOCK         (0x80 | 0x04)
#define U2FHID_INIT         (0x80 | 0x06)
#define U2FHID_WINK         (0x80 | 0x08)
#define U2FHID_ERROR        (0x80 | 0x3F)

// U2F APDU Instructions
#define U2F_INS_REGISTER        0x01
#define U2F_INS_AUTHENTICATE    0x02
#define U2F_INS_VERSION         0x03

// Public API
void u2f_init(void);
void u2f_handle_report(uint8_t *report, uint16_t len);
