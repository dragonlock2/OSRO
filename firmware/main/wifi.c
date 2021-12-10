#include "wifi.h"

#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_wpa2.h>
#include <freertos/semphr.h>

static const char* TAG = "osro-wifi";

static SemaphoreHandle_t sem_wifi_connect;

static void wifi_connect_handler(void* arg, esp_event_base_t event_base, 
                                    int32_t event_id, void* event_data) {
    static uint32_t retry = 0;
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Retry %d connect to the AP...", retry++);
        esp_wifi_connect();
    }
}

static void ip_connect_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data) {
    xSemaphoreGive(sem_wifi_connect);
}

void wifi_connect() {
    /* init drivers */
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    sem_wifi_connect = xSemaphoreCreateBinary();

    /* register handlers to connect and retry on disconnect */
    esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID, &wifi_connect_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT,
        IP_EVENT_STA_GOT_IP, &ip_connect_handler, NULL, NULL);

    /* config AP details */
    #if defined(CONFIG_WIFI_AUTH_OPEN)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    #elif defined(CONFIG_WIFI_AUTH_WPA2_PSK)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    #else // defined(CONFIG_WIFI_AUTH_WPA2_ENTERPRISE)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
        }
    };
    esp_wifi_sta_wpa2_ent_set_identity(
        (uint8_t*)CONFIG_WIFI_USERNAME, strlen(CONFIG_WIFI_USERNAME));
    esp_wifi_sta_wpa2_ent_set_username(
        (uint8_t*)CONFIG_WIFI_USERNAME, strlen(CONFIG_WIFI_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password(
        (uint8_t*)CONFIG_WIFI_PASSWORD, strlen(CONFIG_WIFI_PASSWORD));
    esp_wifi_sta_wpa2_ent_enable();
    #endif
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    /* wait for connection */
    xSemaphoreTake(sem_wifi_connect, portMAX_DELAY);
}