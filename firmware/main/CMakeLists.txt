idf_component_register(SRCS "main.c" "usb_descriptors.c" "crypto_hal.c" "u2f.c" "ctap2.c" "cbor_minimal.c"
                    INCLUDE_DIRS "."
                    REQUIRES tinyusb mbedtls nvs_flash driver)
