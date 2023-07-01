#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include "oven.h"

/* private data */
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

static const int MISO_PIN  = 0;
static const int SCK_PIN   = 10;
static const int CS_PINS[] = {3}; // {1, 3};

static const int ZCD_PIN     = 4;
static const int HEAT_PINS[] = {6, 7};

#define PWM_PERIOD (240) // ~2s @ 60Hz AC

static const char *TAG = "oven";

static struct {
    SemaphoreHandle_t lock;
    TickType_t        start;
    profile_type_t    type;
    oven_status_t     status;

    spi_device_handle_t temps[COUNT_OF(CS_PINS)];

    int pwm_counter;
    int pwm_compare;
    int pwm_compare_next;
} oven_data;

/* private helpers */
static void temp_init(void) {
    const spi_bus_config_t bus_cfg = {
        .mosi_io_num     = -1,
        .miso_io_num     = MISO_PIN,
        .sclk_io_num     = SCK_PIN,
        .max_transfer_sz = 32,
        .flags           = SPICOMMON_BUSFLAG_MASTER,
    };
    spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_DISABLED);

    for (int i = 0; i < COUNT_OF(CS_PINS); i++) {
        const spi_device_interface_config_t dev_cfg = {
            .mode           = 1,
            .clock_speed_hz = 4000000, // 4.3 MHz max
            .spics_io_num   = CS_PINS[i],
            .flags          = 0,
            .queue_size     = 1,
        };
        spi_bus_add_device(SPI2_HOST, &dev_cfg, &oven_data.temps[i]);
    }
}

static double temp_get(void) {
    // MAX6675 device
    uint8_t buf[2];
    spi_transaction_t tran = {
        .flags     = 0,
        .length    = sizeof(buf) * 8,
        .tx_buffer = NULL,
        .rx_buffer = buf,
    };

    double sum = 0.0;
    for (int i = 0; i < COUNT_OF(CS_PINS); i++) {
        spi_device_transmit(oven_data.temps[i], &tran);
        uint16_t raw = (buf[0] << 8) | buf[1];
        sum += (raw >> 3) * 0.25;
    }
    return sum / COUNT_OF(CS_PINS); // average all sensors for now
}

static void IRAM_ATTR pwm_handler(void* arg) {
    oven_data.pwm_counter++;
    if (oven_data.pwm_counter >= PWM_PERIOD) {
        oven_data.pwm_counter = 0;
        oven_data.pwm_compare = oven_data.pwm_compare_next;
    }

    bool on = oven_data.pwm_counter < oven_data.pwm_compare;
    for (int i = 0; i < COUNT_OF(HEAT_PINS); i++) {
        gpio_set_level(HEAT_PINS[i], on);
    }
}

static void pwm_init(void) {
    oven_data.pwm_counter      = 0;
    oven_data.pwm_compare      = 0;
    oven_data.pwm_compare_next = 0;

    gpio_reset_pin(ZCD_PIN);
    gpio_set_direction(ZCD_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ZCD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(ZCD_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ZCD_PIN, pwm_handler, NULL);

    for (int i = 0; i < COUNT_OF(HEAT_PINS); i++) {
        gpio_reset_pin(HEAT_PINS[i]);
        gpio_set_direction(HEAT_PINS[i], GPIO_MODE_INPUT_OUTPUT);
        gpio_set_drive_capability(HEAT_PINS[i], GPIO_DRIVE_CAP_3);
        gpio_set_level(HEAT_PINS[i], 0);
    }
}

static void pwm_set(double duty) {
    int c = duty * PWM_PERIOD;
    taskENTER_CRITICAL(NULL);
    oven_data.pwm_compare_next = c;
    taskEXIT_CRITICAL(NULL);
}

static void oven_thread(void *arg) {
    temp_init();
    pwm_init();
    ESP_LOGI(TAG, "oven initialized!");

    TickType_t wait = xTaskGetTickCount();
    while (true) {
        double temp = temp_get();

        profile_status_t target = {
            .temp = ROOM_TEMP,
            .done = true,
        };
        xSemaphoreTake(oven_data.lock, portMAX_DELAY);
        oven_data.status.current = temp;
        if (oven_data.status.running) {
            double elapsed = (xTaskGetTickCount() - oven_data.start) / (portTICK_PERIOD_MS * 1000.0);
            target = profile_status(oven_data.type, elapsed);
            oven_data.status.target  = target.temp;
            oven_data.status.running = !target.done;
        }
        xSemaphoreGive(oven_data.lock);

        if (target.done) {
            pwm_set(0.0);
        } else {
            // TODO PID stuff, bang-bang for now
            if (temp > target.temp + 0.0) {
                pwm_set(0.0);
            } else if (temp < target.temp - 5.0) {
                pwm_set(1.0);
            }
        }

        vTaskDelayUntil(&wait, 250 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/* public functions */
void oven_init(void) {
    oven_data.lock           = xSemaphoreCreateBinary();
    oven_data.type           = PROFILE_TYPE_MANUAL;
    oven_data.status.current = ROOM_TEMP;
    oven_data.status.target  = ROOM_TEMP;
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
