#include <stdio.h>
#include <nvs_flash.h>

#include "wifi.h"
#include "server.h"
#include "oven.h"

void app_main(void) {
    nvs_flash_init();
    wifi_connect();
    server_start();
    oven_setup();
}
