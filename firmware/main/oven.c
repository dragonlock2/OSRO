#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "oven.h"

/* private data */
static const char *TAG = "oven";

static struct {
    SemaphoreHandle_t lock;
    TickType_t        start;
    profile_type_t    type;
    oven_status_t     status;
} oven_data;

/* private helpers */
static void oven_thread(void *arg) {
    // TODO init temp and pwm
    ESP_LOGI(TAG, "oven initialized!");

    TickType_t wait = xTaskGetTickCount();
    while (true) {
        // TODO grab temps
        double temp = 30.0;

        profile_status_t target = {
            .temp = ROOM_TEMP,
            .done = true,
        };
        xSemaphoreTake(oven_data.lock, portMAX_DELAY);
        if (oven_data.status.running) {
            double elapsed = (xTaskGetTickCount() - oven_data.start) / (portTICK_PERIOD_MS * 1000.0);
            target = profile_status(oven_data.type, elapsed);
            oven_data.status.current = temp;
            oven_data.status.target  = target.temp;
            oven_data.status.running = !target.done;
        }
        xSemaphoreGive(oven_data.lock);

        if (target.done) {
            // TODO set pwm to 0%
        } else {
            // TODO PID stuff, set pwm
        }

        vTaskDelayUntil(&wait, 250 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/* public functions */
void oven_init(void) {
    oven_data.lock           = xSemaphoreCreateBinary();
    oven_data.type           = PROFILE_TYPE_MANUAL;
    oven_data.status.current = 0.0;
    oven_data.status.target  = 0.0;
    oven_data.status.running = false;
    xSemaphoreGive(oven_data.lock);
    xTaskCreate(oven_thread, "oven", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
}

void oven_start(profile_type_t profile, double temp) {
    if (profile < PROFILE_TYPE_COUNT) {
        profile_set_temp(profile, temp);
        xSemaphoreTake(oven_data.lock, portMAX_DELAY);
        oven_data.type = profile;
        oven_data.start = xTaskGetTickCount();
        oven_data.status.running = true;
        xSemaphoreGive(oven_data.lock);
        ESP_LOGI(TAG, "starting profile %d at temp %.1fC", profile, temp);
    }
}

void oven_stop(void) {
    xSemaphoreTake(oven_data.lock, portMAX_DELAY);
    oven_data.status.target  = ROOM_TEMP;
    oven_data.status.running = false;
    xSemaphoreGive(oven_data.lock);
    ESP_LOGI(TAG, "stop");
}

void oven_status(oven_status_t *status) {
    if (status) {
        xSemaphoreTake(oven_data.lock, portMAX_DELAY);
        *status = oven_data.status;
        xSemaphoreGive(oven_data.lock);
    }
}
