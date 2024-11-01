#ifndef __USPIFFS_H__
#define __USPIFFS_H__

#include <gpio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>

// #define USPIFFS_DEB_TAG "uspiffs"
// #define CUSTOM_NVS_PART_NAME "nvs" // Niestandardowa nazwa dla obszaru nvs z partitions.csv (domyślna to "nvs")

void init_nvs();
bool data_inside_spiffs();
void data_written_to_spiffs(bool written);

#endif