#include <uspiffs.h>

static char *TAG = "uspiffs";

void init_nvs()
{
    esp_err_t RES_to_FT = nvs_flash_init(); // Próba inicjalizacji pamięci NVS (Non-Volatile Storage)
    if ((RES_to_FT == ESP_ERR_NVS_NO_FREE_PAGES) || (RES_to_FT == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {                                       // Sprawdzenie, czy jest potrzeba resetu NVS
        ESP_ERROR_CHECK(nvs_flash_erase()); // Formatowanie (czyszczenie) NVS
        RES_to_FT = nvs_flash_init();       // Powtórna inicjalizacja pamięci NVS po wyczyszczeniu
    }
    ESP_ERROR_CHECK(RES_to_FT); // Sprawdzenie, czy po ponownej inicjalizacji wystąpił błąd
}

bool is_data_written() // Sprawdzenie czy dane zostały zapisane do SPIFFS
{
    nvs_handle_t my_handle; // Uchwyt do operacji na NVS
    int8_t flag = 1;        // Flaga (domyślna wartość: 1 - dane nie zostały zapisane)

    /*#if CUSTOM_NVS_PART_NAME*/
    esp_err_t ERR = nvs_open("storage", NVS_READWRITE, &my_handle); // Otwórz partycję NVS o nazwie "nvs"
                                                                    /*#else
                                                                        esp_err_t ERR = nvs_open_from_partition(nvs_part_name, "storage", NVS_READWRITE, &my_handle);
                                                                    #endif*/
    if (ERR == ESP_OK)                                              // Jeśli otwarcie partycji powiodło się pobierz wartość flagi
    {                                                               // 'data_written' z NVS, a jeśli nie istnieje, ustaw flage na 1

        ERR = nvs_get_i8(my_handle, "data_written", &flag); // Pobierz wartość flagi 'data_written' z NVS
        if (ERR == ESP_ERR_NVS_NOT_FOUND)                   // Sprawdzenie, czy flaga istnieje w NVS
        {
            flag = 1; // Ustawienie domyślnej wartości flagi, jeśli flaga nie istnieje
        }
        nvs_close(my_handle); // Zamknięcie uchwytu do NVS
    }
    return (flag == 0); // Zwraca true, jeśli dane zapisane, w przeciwnym wypadku false
}

void set_data_written_flag(bool written) // Funkcja ustawiająca flagę oznaczającą zapis danych
{
    nvs_handle_t my_handle; // Uchwyt do operacji na NVS

    /*#if CUSTOM_NVS_PART_NAME*/
    esp_err_t ERR = nvs_open("storage", NVS_READWRITE, &my_handle); // Otwórz partycję NVS o nazwie "nvs"
                                                                    /*#else
                                                                        esp_err_t ERR = nvs_open_from_partition(nvs_part_name, "storage", NVS_READWRITE, &my_handle);
                                                                    #endif*/
    if (ERR == ESP_OK)                                              // Sprawdzenie, czy otwarcie partycji powiodło się
    {
        int8_t flag = written ? 0 : 1;                     // Ustaw flagę - 0 jeśli dane są zapisane, 1 jeśli nie są
        ERR = nvs_set_i8(my_handle, "data_written", flag); // Zapis flagi w NVS pod flagą 'data_written'
        if (ERR == ESP_OK)                                 // Sprawdzenie, czy zapis flagi się powiódł
        {
            nvs_commit(my_handle); // Zatwierdzenie zmian w NVS
        }
        nvs_close(my_handle); // Zamknięcie uchwytu do NVS
    }
}

void spiffs_init_test()
{
    ESP_LOGI(TAG, "Initializing SPIFFS\r\n");
    esp_vfs_spiffs_conf_t spiffs_conf;  // Struktura konfiguracyjna dla SPIFFS
    spiffs_conf.base_path = "/spiffs";  // Ścieżka bazowa dla systemu plików
    spiffs_conf.partition_label = NULL; // Domyślna partycja SPIFFS
    spiffs_conf.max_files = 5;          // Maksymalna liczba otwartych plików
    // spiffs_conf.format_if_mount_failed = is_data_written(); // Alternatywne podejście do ustawienia format_if_mount_failed
    bool try = is_data_written(); //(0 - brak danych spiffs z formatowaniem) (1 - dane dostępne spiffs bez formatowania)
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
    bool try = is_data_written(); //(0 - brak danych spiffs z formatowaniem) (1 - dane dostępne spiffs bez formatowania)
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

    set_data_written_flag(true); // Dane zapisane i odczytane prawidłowo (dane już istnieją)
}

void test_main()
{
    init_nvs();
    spiffs_init_test();
    spiffs_data_test();
}