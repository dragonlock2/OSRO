idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
        "server.c"
        "oven.c"
        "profile.c"
    INCLUDE_DIRS
        "."
)

spiffs_create_partition_image(storage ../../webui/build FLASH_IN_PROJECT)
