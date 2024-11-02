#include <main.h>

void system_init(void *arg);
void workStatus(void *arg);

static char *TAG = "esp8266";

void gpio_init()
{
    ESP_LOGI(TAG, "Initializing GPIO\r\n");

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = ((1 << 2));
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    esp_err_t ERR = gpio_config(&io_conf);
    if (ERR != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to initialize GPIO\r\n");
    }
    else
    {
        ESP_LOGI(TAG, "Successfully initialized GPIO\r\n");
    }
}
/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of app application, init app function here.
 * Users can use tasks with priorities from 1 to 15
 * (priority of the freeRTOS timer is 2).
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void app_main(void)
{
    xTaskCreate(system_init, "Init_system", 4096, NULL, 9, NULL);

    xTaskCreate(workStatus, "Status", 1024, NULL, 1, NULL);
}

/******************************************************************************
 * FunctionName : system_init
 * Description  : main initializes task.
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void system_init(void *arg)
{
    gpio_init();

    test_main();

    uint16_t siz_dat = uxTaskGetStackHighWaterMark(NULL); // Info for debug
    ESP_LOGI(TAG, "system_init stack: %d\r\n", siz_dat);
    vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : workStatus
 * Description  : on board LED blinking every 1s for 1s.
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void workStatus(void *arg)
{
    while (true)
    {
        gpio_set_level(2, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(2, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}