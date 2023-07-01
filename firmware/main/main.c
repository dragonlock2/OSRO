#include <freertos/FreeRTOS.h>
#include <esp_console.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "wifi.h"
#include "server.h"
#include "oven.h"

static const char *TAG = "main";

void app_main(void) {
    // console init
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl);
    esp_console_start_repl(repl);

    // other init
    nvs_flash_init();
    esp_event_loop_create_default();
    
    // app init
    wifi_init();
    server_init();
    oven_init();
    ESP_LOGI(TAG, "booted! (tick period: %lums)", portTICK_PERIOD_MS);
}
