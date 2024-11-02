#include <test.h>

static char *TAG = "test";

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
    uart_driver_install(UART_NUM_0, 2048, 0, 0, NULL, 0);

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
    uint8_t *data = (uint8_t *)malloc(1024);           // Tymczasowy bufor dla przychodzących danych
    uint8_t *uspiffs_filename = (uint8_t *)malloc(32); // Tymczasowa tablica z nazwą pliku
    if ((data == NULL) || (uspiffs_filename == NULL))
    {
        ESP_LOGE(TAG, "Nie udało się przydzielić pamięci dla buforów\r\n");
    }
    else
    {
        uint32_t timer_start = esp_timer_get_time(); // Zapisz czas początkowy
        uint32_t timer_duration = 10000000;          // 10 sekund w mikrosekundach
        ESP_LOGI(TAG, "Time start: %d\r\n", timer_start);

        char received_data[1024] = {0};
        bool uspiffs_open = false;
        bool reading_data = false;

        while (esp_timer_get_time() - timer_start < timer_duration)
        {
            int len = uart_read_bytes(UART_NUM_0, data, 1024, 100 / portTICK_RATE_MS); // Read data from the UART
            if (len > 0)
            {
                if (!uspiffs_open)
                {
                    char *command_pos = strstr((char *)data, start_command); // Szukamy komendy startowej
                    if (command_pos != NULL)
                    {
                        // Znajdź koniec nazwy pliku na podstawie następnej komendy "$usf_w$"
                        char *write_pos = strstr(command_pos, write_command);
                        if (write_pos != NULL)
                        {
                            int file_name_length = write_pos - (command_pos + strlen(start_command));
                            strncpy((char *)uspiffs_filename, command_pos + strlen(start_command), file_name_length);
                            uspiffs_filename[file_name_length] = '\0'; // Dodaj null-terminator
                            uspiffs_open = true;
                            reading_data = false;
                            ESP_LOGI(TAG, "Odczytano nazwę pliku: %s\r\n", uspiffs_filename);
                        }
                    }
                }
                else
                {
                    if (!reading_data)
                    {
                        // Sprawdzenie czy "$usf_w$" jest obecne
                        char *write_pos = strstr((char *)data, write_command);
                        if (write_pos != NULL)
                        {
                            reading_data = true;
                            received_data[0] = '\0';                                         // Czyszczenie bufora
                            strncat(received_data, write_pos + strlen(write_command), 1023); // Pobierz pierwsze dane
                        }
                    }
                    else
                    {
                        // Szukamy końca danych "$usf_n$"
                        char *end_pos = strstr((char *)data, next_command);
                        if (end_pos != NULL)
                        {
                            strncat(received_data, (char *)data, end_pos - (char *)data);       // Kopiujemy dane do "$usf_n$"
                            uart_write_bytes(UART_NUM_0, received_data, strlen(received_data)); // Wysyłanie przez UART
                            uspiffs_open = false;                                               // Zakończ cykl
                            reading_data = false;
                            ESP_LOGI(TAG, "Wysłano dane przez UART: %s\n", received_data);
                        }
                        else
                        {
                            // Dodaj całość do bufora, jeśli nie znaleziono zakończenia
                            strncat(received_data, (char *)data, len);
                        }
                    }
                }
            }
        }

        timer_start = esp_timer_get_time();
        ESP_LOGI(TAG, "Time end: %d\r\n", timer_start);

        free(uspiffs_filename);
        free(data);
    }
    spiffs_data_test();
}