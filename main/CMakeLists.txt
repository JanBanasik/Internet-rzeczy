idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event esp_netif esp_http_client nvs_flash driver
)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=format)