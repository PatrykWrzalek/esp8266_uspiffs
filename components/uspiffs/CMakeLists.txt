idf_component_register(SRCS "uspiffs.c"
                       INCLUDE_DIRS "."
                       REQUIRES main esp8266 log nvs_flash spiffs)