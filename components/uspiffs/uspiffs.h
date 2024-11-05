#ifndef __USPIFFS_H__
#define __USPIFFS_H__

#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <uart.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>

#define USPIFFS_DEB_TAG "uspiffs"
// #define CUSTOM_NVS_PART_NAME "nvs" // Niestandardowa nazwa dla obszaru nvs z partitions.csv (domyślna to "nvs")

#define RX_BUFFOR_L 1024

#define USPIFFS_UART_INIT_CONFIG_DEFAULT() \
    {                                      \
        .baud_rate = 74880,                \
        .data_bits = UART_DATA_8_BITS,     \
        .parity = UART_PARITY_DISABLE,     \
        .stop_bits = UART_STOP_BITS_1,     \
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

#define USPIFFS_SPIFFS_INIT_CONFIG_DEFAULT() \
    {                                        \
        .base_path = "/spiffs",              \
        .partition_label = NULL,             \
        .max_files = 5,                      \
        .format_if_mount_failed = true};

void uspiffs_nvs_init();
bool data_inside_spiffs();
void data_written_to_spiffs(bool written);

esp_err_t uspiffs_spiffs_init(esp_vfs_spiffs_conf_t *uspiffs_conf);
bool correct_file_name(const char *file_name);
char *uspiffs_first_command_finder(char *moved_buffer);
char *uspiffs_contents(char *bgn_command, char *end_command, uint8_t *moved_buffer, uint16_t *len_moved_buffer);
esp_err_t uspiffs_init(uart_port_t uart_num, uart_config_t *uart_conf, esp_vfs_spiffs_conf_t *uspiffs_conf);
esp_err_t uspiffs_read_data(uart_port_t uart_num, int64_t waiting_time);

#endif