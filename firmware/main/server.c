#include "server.h"

#include <sys/stat.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_http_server.h>

static const char* TAG = "osro-server";

#define DIR_BASE      "/spiffs"
#define FNAME_MAX_LEN 64

static bool is_file_ext(char* filename, char* ext) {
    if (strlen(filename) < strlen(ext)) {
        return false;
    }
    return strcasecmp(&filename[strlen(filename)-strlen(ext)], ext) == 0;
}

static esp_err_t http_get_handler(httpd_req_t *req) {
    /* get filename */
    char filename[sizeof(DIR_BASE)+FNAME_MAX_LEN] = DIR_BASE;
    strncat(filename, req->uri, FNAME_MAX_LEN);
    if (strcmp(req->uri, "/") == 0) {
        strcat(filename, "index.html"); // special case
    }
    ESP_LOGI(TAG, "Serving %s", filename);

    /* open file */
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File doesn't exist");
        return ESP_OK;
    }

    FILE *fd = fopen(filename, "r");
    if (!fd) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Can't read file");
        return ESP_OK;
    }

    /* set correct MIME type */
    if (is_file_ext(filename, ".html")) {
        httpd_resp_set_type(req, "text/html");
    } else if (is_file_ext(filename, ".js")){
        httpd_resp_set_type(req, "application/javascript");
    } else if (is_file_ext(filename, ".css")){
        httpd_resp_set_type(req, "text/css");
    } else if (is_file_ext(filename, ".ico")){
        httpd_resp_set_type(req, "image/x-icon");
    } else if (is_file_ext(filename, ".png")){
        httpd_resp_set_type(req, "image/png");
    } else if (is_file_ext(filename, ".json")){
        httpd_resp_set_type(req, "application/json");
    } else { // .txt, etc.
        httpd_resp_set_type(req, "text/plain");
    }

    /* send file */
    static char chunk[32768];
    size_t chunk_size;
    do {
        chunk_size = fread(chunk, 1, sizeof(chunk), fd);
        httpd_resp_send_chunk(req, chunk, chunk_size);
        ESP_LOGI(TAG, "  Sent chunk %d", chunk_size);
    } while (chunk_size > 0);
    fclose(fd);
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

void server_start() {
    /* init SPIFFS */
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 32,
        .format_if_mount_failed = true
    };
    esp_vfs_spiffs_register(&conf);

    /* start http server */
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;
    httpd_start(&server, &config);

    const httpd_uri_t get = {
        .uri       = "/*",
        .method    = HTTP_GET,
        .handler   = http_get_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &get);
}
