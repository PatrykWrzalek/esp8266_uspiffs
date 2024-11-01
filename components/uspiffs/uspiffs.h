#ifndef __USPIFFS_H__
#define __USPIFFS_H__

#include <gpio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>

// #define CUSTOM_NVS_PART_NAME false // Niestandardowa nazwa dla obszaru nvs - true niestandardowa, false domyślna
// const char *nvs_part_name = "nvs"; // Nazwa obszaru dla nvs z partitions.csv (domyślna to "nvs")

void init_nvs();
bool is_data_written();
void set_data_written_flag(bool written);
void test_main();

#endif