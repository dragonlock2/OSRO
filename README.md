# OSRO (Open-Source Reflow Oven)

OSRO is an ESP32-S2 powered WiFi-connected reflow oven controller designed for low cost and ease of use. Web UI is built on React JS for those aesthetics.

## Building

1. Build the web UI.
```
cd webui
npm install
npm run build
```
2. Follow the [instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) to get ESP-IDF set up. Currently tested with ESP-IDF `v4.3.1`.
3. Build the firmware.
```
cd firmware
get_idf
idf.py set-target esp32s2
idf.py build
```
