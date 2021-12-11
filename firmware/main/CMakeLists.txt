idf_component_register(SRCS "main.c" "wifi.c" "server.c" "oven.c" "profiles.c"
                    INCLUDE_DIRS "." "./inc")

# spiffs_create_partition_image(storage ../../webui/build FLASH_IN_PROJECT)
