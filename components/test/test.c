#include <test.h>

static char *TAG = "test";

static uint16_t BUFFOR_L = 1024;

static char *start_command = "$usf/";
static char *write_command = "$usf_w$";
static char *next_command = "$usf_n$";
// static char *uspiffs_end = "$uspiffs_snd$";

void spiffs_data_test()
{
    FILE *f;
    bool try = data_inside_spiffs(); //(0 - brak danych spiffs z formatowaniem) (1 - dane dostępne spiffs bez formatowania)
    if (!try)
    {
        ESP_LOGI(TAG, "Opening file...\r\n");
        f = fopen("/spiffs/hello.txt", "w"); // Otwarcie pliku do zapisu "w" - writte (jeżeli takiego nie ma to utworzenie)
        if (f == NULL)                       // Błąd otwierania pliku
        {
            ESP_LOGE(TAG, "Failed to open file for writing\r\n");
            return;
        }
        fprintf(f, "Hello World!");        // Zapisanie tekstu do pliku
        fclose(f);                         // Zamknięcie pliku
        ESP_LOGI(TAG, "File written\r\n"); // Informacja o zakończeniu zapisu
    }

    ESP_LOGI(TAG, "Reading file...\r\n");
    f = fopen("/spiffs/hello.txt", "r"); // Otwarcie pliku do odczytu "r" - read
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading\r\n"); // Błąd otwierania pliku do odczytu
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f); // Odczytanie linii z pliku
    ESP_LOGI(TAG, "Read from file: < %s >", line);
    fclose(f); // Zamknięcie pliku

    data_written_to_spiffs(true); // Dane zapisane i odczytane prawidłowo (dane już istnieją)
}

char *get_difference(const char *src1, const char *src2)
{
    const char *pos = strstr(src1, src2);
    if (pos)
    {
        // Obliczamy długość różnicy
        size_t diff_length = pos - src1;
        // Tworzymy nowy bufor na wynik
        char *result = (char *)malloc(diff_length + 1); // +1 na znak null
        if (result)
        {
            strncpy(result, src1, diff_length);
            result[diff_length] = '\0'; // Null-terminator
        }
        return result;
    }
    // Zwracamy NULL, jeśli src2 nie jest częścią src1
    return NULL;
}

void uart_data_reading(uint8_t *u_data)
{
    uint16_t len = uart_read_bytes(UART_NUM_0, u_data, BUFFOR_L, 20 / portTICK_RATE_MS); // Read data from the UART
    if (len >= BUFFOR_L)
    {
        ESP_LOGE(TAG, "To many inncoming data!\r\n");
    }
    else if (len > 0)
    {
        char *usf_start = strstr((const char *)u_data, (const char *)start_command); // Znalezienie początku informacji 'start_command'
        if (usf_start != NULL)                                                       // Początek informacji 'start_command' znaleziony
        {
            usf_start += (strlen((const char *)start_command) - 1);                               // Przesunięcie początku bufora na pozycję z nazwą pliku
            char *usf_contents_start = strstr((const char *)u_data, (const char *)write_command); // Znalezienie początku informacji 'write_command'
            if (usf_contents_start != NULL)                                                       // Początek informacji 'write_command' znaleziony
            {
                char *file_name = get_difference(usf_start, usf_contents_start);
                if (file_name)
                {
                    ESP_LOGI(TAG, "Recived file name: %s\r\n", file_name);
                    free(file_name); // Zwolnienie pamięci na nazwę pliku
                }
                else
                {
                    ESP_LOGE(TAG, "File name not found!\r\n");
                }
                usf_contents_start += strlen((const char *)write_command); // Przesunięcie początku bufora na pozycję początku treści

                char *usf_contents_end = strstr((const char *)u_data, (const char *)next_command); // Znalezienie końca informacji 'write_command'
                if (usf_contents_end != NULL)                                                      // Koniec informacji 'write_command' znaleziony
                {
                    char *usf_contents = get_difference(usf_contents_start, usf_contents_end);
                    if (usf_contents)
                    {
                        ESP_LOGI(TAG, "Recived contents: %s\r\n", usf_contents);
                        free(usf_contents); // Zwolnienie pamięci na nazwę pliku
                    }
                    else
                    {
                        ESP_LOGE(TAG, "File contents not found!\r\n");
                    }
                }
            }
        }

        ESP_LOGI(TAG, "All data being recived: %s\r\n", (char *)u_data);
    }
}

void test_main()
{
    /////////////////////////////////////////////////////
    ////////////////     UART INIT       ////////////////
    /////////////////////////////////////////////////////
    uart_config_t uart_config;
    uart_config.baud_rate = 74880;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 2 * BUFFOR_L, 0, 0, NULL, 0);

    /////////////////////////////////////////////////////
    ////////////     NVS & SPIFFS INIT       ////////////
    /////////////////////////////////////////////////////
    init_nvs();

    esp_vfs_spiffs_conf_t spiffs_conf;         // Struktura konfiguracyjna dla SPIFFS
    spiffs_conf.base_path = "/spiffs";         // Ścieżka bazowa dla systemu plików
    spiffs_conf.partition_label = NULL;        // Domyślna partycja SPIFFS
    spiffs_conf.max_files = 5;                 // Maksymalna liczba otwartych plików
    spiffs_conf.format_if_mount_failed = true; // Formatowanie, jeśli montowanie się nie powiodło
    uspiffs_init(&spiffs_conf);
    /////////////////////////////////////////////////////
    ////////////////        TIMER        ////////////////
    /////////////////////////////////////////////////////
    // TO_DO: NEED SW FLOW CONTROLL
    uint8_t *u_data = (uint8_t *)malloc(BUFFOR_L); // Tymczasowy bufor dla przychodzących danych

    if (u_data == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation error for buffers!\r\n");
    }
    else
    {
        uint32_t timer_start = esp_timer_get_time(); // Zapisz czas początkowy
        uint32_t timer_duration = 10000000;          // 10 sekund w mikrosekundach
        ESP_LOGI(TAG, "Time start: %d\r\n", timer_start);

        // uint8_t buffer_index = 0;
        while (esp_timer_get_time() - timer_start < timer_duration)
        {
            uart_data_reading(u_data);
        }

        timer_start = esp_timer_get_time();
        ESP_LOGI(TAG, "Time end: %d\r\n", timer_start);

        free(u_data);
    }
    spiffs_data_test();
}