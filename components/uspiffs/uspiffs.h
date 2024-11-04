#ifndef __USPIFFS_H__
#define __USPIFFS_H__

#include <stdlib.h>
#include <string.h>
#include <gpio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>

#define USPIFFS_DEB_TAG "uspiffs"
// #define CUSTOM_NVS_PART_NAME "nvs" // Niestandardowa nazwa dla obszaru nvs z partitions.csv (domyślna to "nvs")

void init_nvs();
bool data_inside_spiffs();
void data_written_to_spiffs(bool written);

esp_err_t uspiffs_init(esp_vfs_spiffs_conf_t *uspiffs_conf);
char *uspiffs_first_command_finder(char *moved_buffer);
char *uspiffs_get_contents(const char *contents_start, const char *contents_end);
char *uspiffs_contents(char *bgn_command, char *end_command, uint8_t *moved_buffer, uint16_t *len_moved_buffer);

#endif