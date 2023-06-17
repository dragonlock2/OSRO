#include <string.h>
#include <sys/stat.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include "server.h"

/* private data */
static const char* TAG = "server";

#define SPIFFS_ROOT "/spiffs"
#define SPIFFS_FILENAME_MAX_LEN 64

/* private helpers */
static inline bool is_file_ext(const char *filename, const char *ext) {
    if (strlen(filename) < strlen(ext)) {
        return false;
    }
    return strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0;
}

static esp_err_t http_get_handler(httpd_req_t *req) {
    // get filename
    char filename[sizeof(SPIFFS_ROOT) + SPIFFS_FILENAME_MAX_LEN] = SPIFFS_ROOT;
    if (strcmp(req->uri, "/") == 0) {
        strcat(filename, "/index.html"); // special case
    } else {
        strncat(filename, req->uri, SPIFFS_FILENAME_MAX_LEN);
    }

    ESP_LOGI(TAG, "serving %s", filename);

    // open file
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file doesn't exist");
        return ESP_OK;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "can't read file");
        return ESP_OK;
    }

    // set correct MIME type
    const char *mime_map[][2] = {
        { ".html", "text/html"              },
        { ".js",   "application/javascript" },
        { ".css",  "text/css"               },
        { ".ico",  "image/x-icon"           },
        { ".png",  "image/png"              },
        { ".json", "application/json"       },
    };
    bool mime_flag = false;
    for (int i = 0; i < sizeof(mime_map) / sizeof(mime_map[0]); i++) {
        if (is_file_ext(filename, mime_map[i][0])) {
            httpd_resp_set_type(req, mime_map[i][1]);
            mime_flag = true;
            break;
        }
    }
    if (!mime_flag) {
        httpd_resp_set_type(req, "text/plain"); // .txt, etc.
    }

    // send file
    static char chunk[32768]; // can't put on the stack
    size_t chunk_size;
    do {
        chunk_size = fread(chunk, 1, sizeof(chunk), file);
        httpd_resp_send_chunk(req, chunk, chunk_size);
    } while (chunk_size > 0);
    httpd_resp_send_chunk(req, NULL, 0);
    fclose(file);

    return ESP_OK;
}

/* public functions */
void server_init(void) {
    // init SPIFFS
    esp_vfs_spiffs_conf_t cfg = {
        .base_path              = SPIFFS_ROOT,
        .partition_label        = NULL,
        .max_files              = 32,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&cfg);

    // init http server
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;
    httpd_start(&server, &config);

    const httpd_uri_t get = {
        .uri      = "/*",
        .method   = HTTP_GET,
        .handler  = http_get_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server, &get);

    // TODO add oven callbacks
}
