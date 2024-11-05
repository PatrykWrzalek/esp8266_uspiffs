#include <test.h>

static char *TAG = "test";

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
    esp_err_t ERR = uspiffs_init(UART_NUM_0, NULL, NULL);
    if (ERR != ESP_OK)
    {
        ESP_LOGE(TAG, "Error: '%s'!\r\n", esp_err_to_name(ERR));
    }
    /////////////////////////////////////////////////////
    ////////////////        TIMER        ////////////////
    /////////////////////////////////////////////////////

    ERR = uspiffs_read_data(UART_NUM_0, 10);
    if (ERR != ESP_OK)
    {
        ESP_LOGE(TAG, "Read data error: '%s'!\r\n", esp_err_to_name(ERR));
    }

    spiffs_data_test();
}