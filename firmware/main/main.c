#include <stdio.h>
#include <nvs_flash.h>

#include "wifi.h"
#include "server.h"

void app_main(void) {
    nvs_flash_init();
    wifi_connect();
    server_start();

    printf("Hello World! %s\r\n", CONFIG_WIFI_SSID);
}
