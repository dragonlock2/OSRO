# OSRO (Open-Source Reflow Oven)

OSRO is an ESP32-C3 powered WiFi-connected reflow oven controller designed for low cost and ease of use. Web UI is built on React JS for those aesthetics.

Actual profile support with PID and autotuning is still WIP pending on me finding an oven to convert.

## Building

1. Build the web UI.
```
cd webui
npm install
npm run build
```
2. Follow the [instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) to get ESP-IDF set up. Currently tested with ESP-IDF `v4.4`.
3. Setup the build for the firmware. Make sure to set the WiFi SSID and password under `OSRO WiFi configuration`.
```
cd firmware
get_idf
idf.py set-target esp32c3
idf.py menuconfig
```
4. Build and flash the firmware.
```
cd firmware
idf.py build
idf.py -p <serial port> flash
```
