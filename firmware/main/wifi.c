#include "wifi.h"

#include <mdns.h>
#include <string.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_wpa2.h>
#include <freertos/semphr.h>

static const char* TAG = "osro-wifi";

#ifdef CONFIG_WIFI_IS_AP
static void wifi_connect_handler(void* arg, esp_event_base_t event_base, 
                                    int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

#else
static SemaphoreHandle_t sem_wifi_connect;

static void wifi_connect_handler(void* arg, esp_event_base_t event_base, 
                                    int32_t event_id, void* event_data) {
    static uint32_t retry = 0;
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Retry %lu connect to the AP...", retry++);
        esp_wifi_connect();
    }
}

static void ip_connect_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data) {
    xSemaphoreGive(sem_wifi_connect);
}
#endif // CONFIG_WIFI_IS_AP

void wifi_connect() {
    /* init drivers */
    esp_netif_init();
    esp_event_loop_create_default();

    #ifdef CONFIG_WIFI_IS_AP
    /* more init */
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    /* register handlers for debug */
    esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID, &wifi_connect_handler, NULL, NULL);

    /* config AP details */
    wifi_config_t wifi_config = {
        .ap = {
            .ssid     = CONFIG_WIFI_SSID,
            .ssid_len = strlen(CONFIG_WIFI_SSID),
            .channel  = CONFIG_WIFI_CHANNEL,
            .password = CONFIG_WIFI_PASSWORD,
            .max_connection = CONFIG_WIFI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK /* TODO switch to WPA3 */
        },
    };
    if (strlen(CONFIG_WIFI_PASSWORD) < 8) {
        ESP_LOGE(TAG, "Password too short, switching to OPEN");
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    #else
    /* more init */
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
    xSemaphoreTake(sem_wifi_connect, portMAX_DELAY);
    #endif // CONFIG_WIFI_IS_AP

    #ifdef CONFIG_WIFI_HAS_MDNS
    ESP_LOGI(TAG, "Starting mDNS");
    mdns_init();
    mdns_hostname_set(CONFIG_WIFI_MDNS_HOSTNAME);
    mdns_instance_name_set(CONFIG_WIFI_MDNS_DEFAULT_INSTANCE);
    #endif
}
