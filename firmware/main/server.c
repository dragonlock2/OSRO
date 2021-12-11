#include "server.h"
#include "oven.h"
#include "profiles.h"

#include <sys/stat.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_http_server.h>
#include <cJSON.h>

static const char* TAG = "osro-server";

/* File server */
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

/* REST interface */
static esp_err_t http_temps_handler(httpd_req_t *req) {
    temps_t temps = oven_get_temps();
    ESP_LOGI(TAG, "Sending temps { current: %.3f°C target: %.3f°C run: %d}",
        temps.current, temps.target, temps.running);

    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "current", temps.current);
    cJSON_AddNumberToObject(root, "target", temps.target);
    cJSON_AddBoolToObject(root, "running", temps.running);
    const char *root_str = cJSON_Print(root);
    httpd_resp_sendstr(req, root_str);
    free((void*) root_str);
    cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t http_profiles_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON* profs = cJSON_AddArrayToObject(root, "profiles");

    for (int i = 0; i < NUM_PROFILES; i++) {
        const profile_t* profile = PROFILES[i];
        cJSON* prof = cJSON_CreateObject();
        cJSON_AddStringToObject(prof, "name", profile->name);
        cJSON* pts = cJSON_AddArrayToObject(prof, "pts");

        float time = 0;
        for (int j = 0; j < profile->num_pts; j++) {
            const point_t point = profile->pts[j];
            time += point.time; // convert times to timestamps
            cJSON* pt = cJSON_CreateObject();
            cJSON_AddNumberToObject(pt, "time", time);
            cJSON_AddNumberToObject(pt, "temp", point.temp);
            cJSON_AddItemToArray(pts, pt);
        }

        cJSON_AddItemToArray(profs, prof);
    }

    const char *root_str = cJSON_Print(root);
    httpd_resp_sendstr(req, root_str);
    free((void*) root_str);
    cJSON_Delete(root);

    ESP_LOGI(TAG, "Sent profiles");

    return ESP_OK;
}

static esp_err_t http_start_handler(httpd_req_t *req) {
    /* read request into buffer */
    char buf[128];
    if (req->content_len >= sizeof(buf)) { // includes null terminator!
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "too long");
        return ESP_OK;
    }
    int cur_len = 0, recv_len = 0;
    while (cur_len < req->content_len) {
        recv_len = httpd_req_recv(req, buf + cur_len, req->content_len);
        if (recv_len < 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "fail?");
            return ESP_OK;
        }
        cur_len += recv_len;
    }
    buf[req->content_len] = '\0';

    /* parse JSON values */
    ESP_LOGI(TAG, "Received /start JSON: %s", buf);
    cJSON* root      = cJSON_Parse(buf);
    cJSON* idx_json  = cJSON_GetObjectItem(root, "idx");
    cJSON* temp_json = cJSON_GetObjectItem(root, "temp");
    if (!cJSON_IsNumber(idx_json) || !cJSON_IsNumber(temp_json)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_OK;
    }
    int    idx  = cJSON_GetNumberValue(idx_json);
    double temp = cJSON_GetNumberValue(temp_json);
    cJSON_Delete(root);

    /* process request */
    if (oven_start(idx, temp)) {
        httpd_resp_sendstr(req, "Starting oven!");
    } else {
        httpd_resp_sendstr(req, "Couldn't start oven :(");
    }
    return ESP_OK;
}

static esp_err_t http_stop_handler(httpd_req_t *req) {
    oven_stop();
    httpd_resp_sendstr(req, "Stopping oven!");
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

    const httpd_uri_t temps = {
        .uri       = "/temps",
        .method    = HTTP_GET,
        .handler   = http_temps_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &temps);

    const httpd_uri_t profiles = {
        .uri       = "/profiles",
        .method    = HTTP_GET,
        .handler   = http_profiles_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &profiles);

    const httpd_uri_t start = {
        .uri       = "/start",
        .method    = HTTP_POST,
        .handler   = http_start_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &start);

    const httpd_uri_t stop = {
        .uri       = "/stop",
        .method    = HTTP_POST,
        .handler   = http_stop_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &stop);

    const httpd_uri_t get = {
        .uri       = "/*",
        .method    = HTTP_GET,
        .handler   = http_get_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &get);
}
