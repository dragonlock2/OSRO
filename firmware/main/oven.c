#include "oven.h"
#include "profiles.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <math.h>

static const char* TAG = "osro-oven";

/* IO */
const uint8_t LED = 5;
const uint8_t ZCD = 4;
const gpio_num_t HEAT[] = { 7, 6 };
const uint8_t MISO = 0;
const uint8_t SCK  = 10;
const gpio_num_t CS[] = { 1, 3 };

#define NUM_HEAT (sizeof(HEAT) / sizeof(gpio_num_t))
#define NUM_CS   (sizeof(CS) / sizeof(gpio_num_t))

/* Helpers */
static spi_device_handle_t max6675s[NUM_CS];

static void gpio_toggle_level(gpio_num_t gpio_num) {
    gpio_set_level(gpio_num, !gpio_get_level(gpio_num));
}

static void IRAM_ATTR zcd_handler(void* arg) {
    // TODO implement "pwm"
}

static void get_temps(double* temps) {
    uint8_t data[2];
    spi_transaction_t trans = {
        .length = 16,
        .rx_buffer = data,
    };

    for (int i = 0; i < NUM_CS; i++) {
        spi_device_transmit(max6675s[i], &trans);
        uint16_t raw = data[0] << 8 | data[1];
        if (raw & 0x04) {
            temps[i] = NAN;
        } else {
            temps[i] = (raw >> 3) * 0.25;
        }
    }
}

/* Oven functions */

static temps_t status = {
    .current = 0.0,
    .target = 0.0,
    .running = false,
};

static int counter = 0;

static void oven_loop(void* arg) { // TODO actual loop
    double temps[NUM_CS] = {0, 0};

    while (1) {
        get_temps(temps);
        status.current = temps[0];

        if (counter) {
            counter--;
        } else {
            status.target = temps[1];
            status.running = false;
        }

        gpio_toggle_level(LED);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/* Public functions */
void oven_setup() {
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(LED, 0);

    gpio_reset_pin(ZCD);
    gpio_set_direction(ZCD, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ZCD, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(ZCD, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ZCD, zcd_handler, NULL);

    for (int i = 0; i < NUM_HEAT; i++) {
        gpio_reset_pin(HEAT[i]);
        gpio_set_direction(HEAT[i], GPIO_MODE_INPUT_OUTPUT);
        gpio_set_drive_capability(HEAT[i], GPIO_DRIVE_CAP_3); // strongest
        gpio_set_level(HEAT[i], 0);
    }

    const spi_bus_config_t bus_cfg = {
        .miso_io_num = MISO,
        .sclk_io_num = SCK,
        .max_transfer_sz = 32,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_DISABLED);

    for (int i = 0; i < NUM_CS; i++) {
        spi_device_interface_config_t dev_cfg = {
            .mode = 1,
            .clock_speed_hz = 4000000, // 4.3 MHz max
            .spics_io_num = CS[i],
            .flags = 0,
            .queue_size = 2,
        };
        spi_bus_add_device(SPI2_HOST, &dev_cfg, &max6675s[i]);
    }

    xTaskCreate(oven_loop, "oven_loop", 2048, NULL, 10, NULL);
    ESP_LOGI(TAG, "Oven setup finished!");
}

bool oven_start(int idx, double temp) {
    status.running = true;

    counter = 10;
    status.target = temp;

    ESP_LOGI(TAG, "Oven started on profile %d at temp %.3f°C", idx, temp);
    return true;
}

void oven_stop() {
    status.running = false;
    ESP_LOGI(TAG, "Oven stopped!");
}

temps_t oven_get_temps() {
    return status;
}
