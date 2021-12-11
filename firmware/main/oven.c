#include "oven.h"
#include "profiles.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

static const char* TAG = "osro-oven";

/* Oven functions */
static double current_temp = 0.0;
static double target_temp = 0.0;
static bool running = false;

static int counter = 0;

static void oven_loop(void* arg) {
    while (1) {
        current_temp = 300.0*esp_random()/0xffffffff;

        if (counter) {
            // target_temp = 300.0*esp_random()/0xffffffff;
            counter--;
        } else {
            target_temp = 25.0;
            running = false;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/* Public functions */
void oven_setup() {
    xTaskCreate(oven_loop, "oven_loop", 2048, NULL, 10, NULL);
    ESP_LOGI(TAG, "Oven setup finished!");
}

bool oven_start(int idx, double temp) {
    running = true;

    counter = 10;
    target_temp = temp;

    ESP_LOGI(TAG, "Oven started on profile %d at temp %.3f°C", idx, temp);
    return true;
}

void oven_stop() {
    running = false;
    ESP_LOGI(TAG, "Oven stopped!");
}

temps_t oven_get_temps() {
    return (temps_t) {
        .current = current_temp,
        .target = target_temp,
        .running = running,
    };
}
