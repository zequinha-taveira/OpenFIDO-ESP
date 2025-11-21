#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include "u2f.h"

static const char *TAG = "U2F_MAIN";

// Pin Definitions (Adjust based on Schematic)
#define LED_PIN     18
#define BUTTON_PIN  0

// Initialize NVS (Non-Volatile Storage)
void init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Initialized");
}

// Initialize GPIO for Button and LED
void init_gpio() {
    // LED Output
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    // Button Input (Pull-up)
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    
    ESP_LOGI(TAG, "GPIO Initialized");
}

// Main Application Entry Point
void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32 U2F Token...");

    // 1. Init Hardware
    init_nvs();
    init_gpio();

    // 2. Init USB Stack (TinyUSB)
    ESP_LOGI(TAG, "Initializing TinyUSB...");
    tusb_init();

    // 3. Main Loop
    while (1) {
        // Handle TinyUSB tasks
        tud_task(); 
        
        // Simple Blink to show life
        static int led_state = 0;
        gpio_set_level(LED_PIN, led_state);
        led_state = !led_state;
        
        // Check Button
        if (gpio_get_level(BUTTON_PIN) == 0) {
            ESP_LOGI(TAG, "Button Pressed!");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
    }
}

// TinyUSB Callbacks
void tud_mount_cb(void) {
    ESP_LOGI(TAG, "USB Mounted");
}

void tud_umount_cb(void) {
    ESP_LOGI(TAG, "USB Unmounted");
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  // TODO: Handle U2F Get Report
  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  
  // Pass HID report to U2F stack
  u2f_handle_report((uint8_t*)buffer, bufsize);
}
