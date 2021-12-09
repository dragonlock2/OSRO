# OSRO (Open-Source Reflow Oven)

OSRO is an ESP32-S2 powered WiFi-connected reflow oven controller designed for low cost and ease of use.

## Building

1. Follow the [instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) to get ESP-IDF setup. Currently tested with ESP-IDF `v4.3.1`.
2. Build the firmware.

```
cd firmware
get_idf
idf.py set-target esp32s2
idf.py build
```
