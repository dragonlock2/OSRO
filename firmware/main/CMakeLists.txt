idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
    INCLUDE_DIRS
        "."
)

# TODO re-enable
# spiffs_create_partition_image(storage ../../webui/build FLASH_IN_PROJECT)
