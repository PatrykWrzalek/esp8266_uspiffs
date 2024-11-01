#include <test.h>

static char *TAG = "test";

void spiffs_init_test()
{
    ESP_LOGI(TAG, "Initializing SPIFFS\r\n");
    esp_vfs_spiffs_conf_t spiffs_conf;  // Struktura konfiguracyjna dla SPIFFS
    spiffs_conf.base_path = "/spiffs";  // Ścieżka bazowa dla systemu plików
    spiffs_conf.partition_label = NULL; // Domyślna partycja SPIFFS
    spiffs_conf.max_files = 5;          // Maksymalna liczba otwartych plików
    // spiffs_conf.format_if_mount_failed = data_inside_spiffs(); // Alternatywne podejście do ustawienia format_if_mount_failed
    bool try = data_inside_spiffs(); //(0 - brak danych, spiffs z formatowaniem) (1 - dane dostępne, spiffs bez formatowania)
    ESP_LOGI(TAG, "flaga 'try' = %d\r\n", try);
    if (try) // Inicjalizacja SPIFFS bez formatowania
    {
        ESP_LOGI(TAG, "SPIFFS initialization without formatting\r\n");
        spiffs_conf.format_if_mount_failed = false; // Brak formatowania, jeśli montowanie się nie powiodło
    }
    else // Inicjalizacja SPIFFS z formatowania
    {
        ESP_LOGI(TAG, "SPIFFS initialization with formatting\r\n");
        spiffs_conf.format_if_mount_failed = true; // Formatowanie, jeśli montowanie się nie powiodło
    }

    esp_err_t ERR = esp_vfs_spiffs_register(&spiffs_conf); // Zainicjuj i zamontuj system plików SPIFFS na podstawie powyższej konfiguracji
    if (ERR != ESP_OK)
    {
        if (ERR == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem\r\n"); // Błąd montowania lub formatowania systemu
        }
        else if (ERR == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition\r\n"); // Nie znaleziono partycji SPIFFS
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)\r\n", esp_err_to_name(ERR)); // Inny błąd inicjalizacji SPIFFS
        }
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Successfully initialized SPIFFS\r\n");
    }
}

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
    init_nvs();
    spiffs_init_test();
    spiffs_data_test();
}