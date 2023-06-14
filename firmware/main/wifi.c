#include <string.h>
#include <argtable3/argtable3.h>
#include <mdns.h>
#include <esp_console.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_wpa2.h>
#include "wifi.h"

/* private data */
static const char *TAG = "wifi";

static struct {
    struct arg_str *type;
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_str *user;
    struct arg_end *end;
} connect_args;

/* private helpers */
static void wifi_connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
            break;
        }

        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "retry connect to the AP...");
            esp_wifi_connect();
            break;
    }
}

static void wifi_connect(const char *type, const char *ssid, const char *pass, const char *user) {
    esp_wifi_stop();
    esp_wifi_sta_wpa2_ent_disable();
    if (strcmp(type, "ap") == 0) {
        wifi_config_t wifi_config = {
            .ap = {
                .ssid_len        = 0,
                .channel         = 6,
                .authmode        = WIFI_AUTH_WPA2_PSK, // TODO enable WPA3 when ESP-IDF updated
                .max_connection  = 4,
                .beacon_interval = 100,
                .pairwise_cipher = WIFI_CIPHER_TYPE_CCMP,
                .ftm_responder   = false,
                .pmf_cfg = {
                    .capable  = true,
                    .required = true,
                },
                // .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            },
        };
        strncpy((char*) wifi_config.ap.ssid,     ssid, sizeof(wifi_config.ap.ssid));
        strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));
        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    } else {
         wifi_config_t wifi_config = {
            .sta = {
                .scan_method     = WIFI_FAST_SCAN,
                .bssid_set       = false,
                .channel         = 0,
                .listen_interval = 0,
                .sort_method     = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold = {
                    .rssi     = 0,
                    .authmode = WIFI_AUTH_OPEN,
                },
                .pmf_cfg = {
                    .capable  = true,
                    .required = false,
                },
                .rm_enabled         = false,
                .btm_enabled        = false,
                .mbo_enabled        = false,
                .ft_enabled         = false,
                .owe_enabled        = false,
                .transition_disable = false,
                .sae_pwe_h2e        = WPA3_SAE_PWE_BOTH,
                .failure_retry_cnt  = 1,
            },
        };
        strncpy((char*) wifi_config.sta.ssid,     ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char*) wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
        if (strcmp(type, "open") == 0) {
            wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        } else if (strcmp(type, "wpa2") == 0) {
            wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        } else if (strcmp(type, "wpa3") == 0) {
            wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK;
            wifi_config.sta.pmf_cfg.required   = true;
        } else if (strcmp(type, "wpa2_ent") == 0) {
            esp_wifi_sta_wpa2_ent_set_identity((uint8_t*) user, strlen(user));
            esp_wifi_sta_wpa2_ent_set_username((uint8_t*) user, strlen(user));
            esp_wifi_sta_wpa2_ent_set_password((uint8_t*) pass, strlen(pass));
            esp_wifi_sta_wpa2_ent_enable();
        } else {
            ESP_LOGE(TAG, "unknown type, settings messed up");
        }
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    }
    esp_wifi_start();
}

static int wifi_connect_command(int argc, char **argv) {
    if (arg_parse(argc, argv, (void**) &connect_args) != 0) {
        arg_print_errors(stderr, connect_args.end, argv[0]);
        return 1;
    }
    wifi_connect(connect_args.type->sval[0], connect_args.ssid->sval[0],
        connect_args.pass->sval[0], connect_args.user->sval[0]);
    return 0;
}

/* public functions */
void wifi_init(void) {
    // init default connection
    esp_netif_init();
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_connect_handler, NULL, NULL);

#if defined (CONFIG_WIFI_IS_AP)
    const char *type = "ap";
    const char *ssid = CONFIG_WIFI_SSID;
    const char *pass = CONFIG_WIFI_PASSWORD;
    const char *user = "";
#elif defined (CONFIG_WIFI_AUTH_OPEN)
    const char *type = "open";
    const char *ssid = CONFIG_WIFI_SSID;
    const char *pass = "";
    const char *user = "";
#elif defined (CONFIG_WIFI_AUTH_WPA2_PSK)
    const char *type = "wpa2";
    const char *ssid = CONFIG_WIFI_SSID;
    const char *pass = CONFIG_WIFI_PASSWORD;
    const char *user = "";
#elif defined (CONFIG_WIFI_AUTH_WPA3_PSK)
    const char *type = "wpa3";
    const char *ssid = CONFIG_WIFI_SSID;
    const char *pass = CONFIG_WIFI_PASSWORD;
    const char *user = "";
#elif defined (CONFIG_WIFI_AUTH_WPA2_ENTERPRISE)
    const char *type = "wpa2_ent";
    const char *ssid = CONFIG_WIFI_SSID;
    const char *pass = CONFIG_WIFI_PASSWORD;
    const char *user = CONFIG_WIFI_USERNAME;
#endif
    wifi_connect(type, ssid, pass, user);

    // init MDNS
    mdns_init();
    mdns_hostname_set(CONFIG_WIFI_MDNS_HOSTNAME);
    mdns_instance_name_set(CONFIG_WIFI_MDNS_DEFAULT_INSTANCE);

    // init command
    connect_args.type = arg_str1(NULL, NULL,   "<type>", "type (ap, open, wpa2, wpa3, wpa2_ent)");
    connect_args.ssid = arg_str1(NULL, NULL,   "<ssid>", "SSID");
    connect_args.pass = arg_str0("p",  "pass", "<pass>", "password");
    connect_args.user = arg_str0("u",  "user", "<user>", "username (wpa2_ent only)");
    connect_args.end  = arg_end(10);
    const esp_console_cmd_t connect_cmd = {
        .command  = "wifi",
        .help     = "host/connect network",
        .hint     = NULL,
        .func     = wifi_connect_command,
        .argtable = &connect_args,
    };
    esp_console_cmd_register(&connect_cmd);
}
