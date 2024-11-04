#include <uspiffs.h>

/******************************************************************************
 * FunctionName : init_nvs
 * Description  : function initializes NVS.
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
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

/******************************************************************************
 * FunctionName : data_inside_spiffs
 * Description  : function checking whether data has been written to SPIFFS.
 * Parameters   : none
 * Returns      : 'true' - data were written to SPIFFS
 *                'false' - data weren't written to SPIFFS
 *******************************************************************************/
bool data_inside_spiffs()
{
    nvs_handle_t my_handle; // Uchwyt do operacji na NVS
    int8_t flag = 0x00;     // Stan bitu 0 flagi 'spiffs_info' oznacza brak zapisanych danych

#ifdef CUSTOM_NVS_PART_NAME
    esp_err_t ERR = nvs_open_from_partition(CUSTOM_NVS_PART_NAME, "storage", NVS_READWRITE, &my_handle);
#else
    esp_err_t ERR = nvs_open("storage", NVS_READWRITE, &my_handle); // Otwórz partycję NVS o domyślnej nazwie "nvs"
#endif
    if (ERR == ESP_OK) // Jeśli otwarcie partycji powiodło się pobierz wartość flagi
    {                  // 'spiffs_info' z NVS, a jeśli nie istnieje wyzeruj flagę

        ERR = nvs_get_i8(my_handle, "spiffs_info", &flag); // Pobierz wartość flagi 'spiffs_info' z NVS
        if (ERR == ESP_ERR_NVS_NOT_FOUND)                  // Sprawdzenie, czy flaga istnieje w NVS
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGI(USPIFFS_DEB_TAG, "Flag spiffs_info isn't exist\r\n");
#endif
            nvs_close(my_handle); // Zamknięcie uchwytu do NVS
            return false;         // Zwrócenie false - brak danych zapisanych w SPIFFS (flaga nie istnieje)
        }
#ifdef USPIFFS_DEB_TAG
        else if (ERR == ESP_ERR_NVS_KEY_TOO_LONG)
        {
            ESP_LOGI(USPIFFS_DEB_TAG, "Change nvs key back to 'spiffs_info'!\r\n");
        }
        ESP_LOGI(USPIFFS_DEB_TAG, "Read spiffs_info = %d\r\n", flag);
#endif
        nvs_close(my_handle); // Zamknięcie uchwytu do NVS
    }

    return (flag & (1 << 0)); // Zwraca true, jeśli dane zapisane, w przeciwnym wypadku false
}

/******************************************************************************
 * FunctionName : data_written_to_spiffs
 * Description  : function that sets whether data has or has not been written to SPIFFS.
 * Parameters   : written - 'true' data were written
 *                          'false' data weren't written to SPIFFS
 * Returns      : none
 *******************************************************************************/
void data_written_to_spiffs(bool written) // Funkcja ustawiająca flagę oznaczającą zapis danych
{
    nvs_handle_t my_handle; // Uchwyt do operacji na NVS
    int8_t flag = 0x00;

#ifdef CUSTOM_NVS_PART_NAME
    esp_err_t ERR = nvs_open_from_partition(CUSTOM_NVS_PART_NAME, "storage", NVS_READWRITE, &my_handle);
#else
    esp_err_t ERR = nvs_open("storage", NVS_READWRITE, &my_handle); // Otwórz partycję NVS o nazwie "nvs"
#endif
    if (ERR == ESP_OK) // Sprawdzenie, czy otwarcie partycji powiodło się
    {
        nvs_get_i8(my_handle, "spiffs_info", &flag);      // Pobierz wartość flagi 'spiffs_info' z NVS
        flag |= (written << 0);                           // Ustaw bit0 flagi na - 1 jeśli dane są zapisane, 0 jeśli nie są
        ERR = nvs_set_i8(my_handle, "spiffs_info", flag); // Zapis flagi w NVS pod flagą 'spiffs_info'
        if (ERR == ESP_OK)                                // Sprawdzenie, czy zapis flagi się powiódł
        {
            nvs_commit(my_handle); // Zatwierdzenie zmian w NVS
#ifdef USPIFFS_DEB_TAG
            ESP_LOGI(USPIFFS_DEB_TAG, "Successfuly commit changes\r\n");
        }
        else
        {
            ESP_LOGE(USPIFFS_DEB_TAG, "Error committing changes = %s\r\n", esp_err_to_name(ERR));
        }
        ESP_LOGI(USPIFFS_DEB_TAG, "Written spiffs_info = %d\r\n", flag);
#else
        }
#endif
        nvs_close(my_handle); // Zamknięcie uchwytu do NVS
    }
}

/******************************************************************************
                     TTTTTTTT  EEEEE  SSSSSS  TTTTTTTT
                        TT     EE     SS         TT
                        TT     EEEE   SSSSSS     TT
                        TT     EE         SS     TT
                        TT     EEEEE  SSSSSS     TT
 *******************************************************************************/
/******************************************************************************
 * An area of ​​code that is in the testing stage.
 *******************************************************************************/

/******************************************************************************
 * FunctionName : uspiffs_init
 * Description  : function initializes SPIFFS.
 * Parameters   : uspiffs_conf - ointer to esp_vfs_spiffs_conf_t configuration structure
 * Returns      : - ESP_OK                  if success
 *                - ESP_ERR_NO_MEM          if objects could not be allocated
 *                - ESP_ERR_INVALID_STATE   if already mounted or partition is encrypted
 *                - ESP_ERR_NOT_FOUND       if partition for SPIFFS was not found
 *                - ESP_FAIL                if mount or format fails
 *******************************************************************************/
esp_err_t uspiffs_init(esp_vfs_spiffs_conf_t *uspiffs_conf)
{
    bool spiffs_have_data = data_inside_spiffs(); // 0 - brak danych, 1 - dane dostępne
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Initializing uspiffs...\r\n");
    if (spiffs_have_data)
    {
        ESP_LOGI(USPIFFS_DEB_TAG, "Data inside SPIFFS initialization without formatting\r\n");
    }
    else
    {
        ESP_LOGI(USPIFFS_DEB_TAG, "No data inside SPIFFS initialization with formatting\r\n");
    }
#endif
    uspiffs_conf->format_if_mount_failed = !spiffs_have_data; // Formatowanie lub brak, jeśli montowanie się nie powiodło

    esp_err_t ERR = esp_vfs_spiffs_register((const esp_vfs_spiffs_conf_t *)uspiffs_conf); // Zainicjuj i zamontuj system plików SPIFFS
    if (ERR != ESP_OK)
    {
#ifdef USPIFFS_DEB_TAG
        if (ERR == ESP_FAIL)
        {
            ESP_LOGE(USPIFFS_DEB_TAG, "Failed to mount or format filesystem\r\n"); // Błąd montowania lub formatowania systemu
        }
        else if (ERR == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(USPIFFS_DEB_TAG, "Failed to find SPIFFS partition\r\n"); // Nie znaleziono partycji SPIFFS
        }
        else
        {
            ESP_LOGE(USPIFFS_DEB_TAG, "Failed to initialize SPIFFS (%s)\r\n", esp_err_to_name(ERR)); // Inny błąd inicjalizacji SPIFFS
        }
#endif
        return ERR;
    }
    else
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGI(USPIFFS_DEB_TAG, "Successfully initialized SPIFFS\r\n");
#endif
        return ESP_OK;
    }
}

/******************************************************************************
 * FunctionName : uspiffs_command_finder
 * Description  : none
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
char *uspiffs_first_command_finder(char *moved_buffer)
{
    char *start_command = "$usf/";   // Komenda początku uspiffs po niej znajduje się nazwa pliku
    char *write_command = "$usf_w$"; // Komenda początku treści dla pliku
    char *next_command = "$usf_n$";  // Komenda końca treści dla pliku

    char *s_commands = strstr((const char *)moved_buffer, (const char *)start_command); // Szukanie komendy 'start_command'
    char *w_commands = strstr((const char *)moved_buffer, (const char *)write_command); // Szukanie komendy 'write_command'

    if ((s_commands == NULL) && (w_commands == NULL)) // Nie znaleziono rzadnej komendy początku
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGI(USPIFFS_DEB_TAG, "There is no start command in the buffer!\r\n");
#endif
        return NULL;
    }

    uint16_t size_start = s_commands ? (uint16_t)(s_commands - moved_buffer) : UINT16_MAX; // Obliczanie pozycji dla każdej komendy, jeśli istnieją
    uint16_t size_write = w_commands ? (uint16_t)(w_commands - moved_buffer) : UINT16_MAX;

    if (size_start < size_write) // Wybieranie komendy na podstawie pozycji w buforze
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGI(USPIFFS_DEB_TAG, "First command in buffer is: '%s'\r\n", start_command);
#endif
        return (char *)start_command;
    }
    else
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGI(USPIFFS_DEB_TAG, "First command in buffer is: '%s'\r\n", write_command);
#endif
        return (char *)write_command;
    }
}

/******************************************************************************
 * FunctionName : uspiffs_contents
 * Description  : none
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
char *uspiffs_contents(char *bgn_command, char *end_command, uint8_t *moved_buffer, uint16_t len_moved_buffer)
{
    char *start_command = "$usf/"; // Komenda początku uspiffs po niej znajduje się nazwa pliku

    /**********************************************************************************/
    /*  Sprawdzenie czy obie komendy początku i końca są częsią danych przychodzących */
    /**********************************************************************************/

    // 1) Wskaźnik 'contents' wskazuje całość danych (z 'moved_buffer') znajdującą się za komendą początku
    char *contents = strstr((const char *)moved_buffer, (const char *)bgn_command); // Szukanie komendy początku
    if (contents == NULL)                                                           // Nie znaleziono początku informacji
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Not found contents start!\r\n");
#endif
        return NULL;
    }
    int bgn_position = (int)(contents - (char *)moved_buffer); // Wyznaczenie pozycji początku wiadomości
    bgn_position += (int)strlen(bgn_command);                  // Przesunięcie początku wiadomości na początek treści
    if (len_moved_buffer <= bgn_position)                      // Sprawdzenie czy nie występuje overflow
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Overflow error for command: %s!\r\n", bgn_command);
#endif
        return NULL;
    }

    // 2) Wskaźnik 'contents' wskazuje całość danych (z 'moved_buffer') znajdującą się za komendą końca
    contents = strstr((const char *)(moved_buffer + bgn_position), (const char *)end_command); // Szukanie komendy końca
    if (contents == NULL)                                                                      // Nie znaleziono końca informacji
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Not found contents end!\r\n");
#endif
        return NULL;
    }
    int end_position = (int)(contents - (char *)moved_buffer); // Wyznaczenie pozycji końca treści
    end_position += (int)strlen(end_command);                  // Przesunięcie końca treści na koniec wiadomości
    if (len_moved_buffer < end_position)                       // Sprawdzenie czy nie występuje overflow
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Overflow error for command: %s!\r\n", end_command);
#endif
        return NULL;
    }

    // 3) Sprawdzenie czy w buforze znajduje się treść
    end_position -= (int)strlen((const char *)end_command); // Przesunięcie końca wiadomości na koniec treści
    if (end_position == bgn_position)                       // Sprawdzenie czy wiadomość ma treść
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Invalid range between start and end!\r\n");
#endif
        return NULL;
    }

    /**********************************************************************************/
    /*                  Sprawdzenie czy wiadomość z bufora ma treść                   */
    /**********************************************************************************/

    end_position -= bgn_position; // Zmienna 'end_position' wskazuje długość treści do skopiowania

    // contents = (char *)malloc(end_position); // Alokacja bufora `contents` dla skopiowanej treści
    contents = (char *)malloc(end_position + 1); // +1 na znak '\0'
    if (contents == NULL)
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Memory allocation failed for contents!\r\n");
#endif
        return NULL;
    }
    memcpy(contents, moved_buffer + bgn_position, end_position); // Kopiowanie treści z `moved_buffer` do `contents`
    contents[end_position] = '\0';                               // Zakończenie ciągu znakowego
    // Odjęcie od ilości danych w buforze ('moved_buffer') danych, które zostały już przetworzone
    len_moved_buffer -= ((end_position + bgn_position) + (int)strlen((const char *)end_command));
    if (bgn_command == start_command)
    { // Wyjątek, po komendzie startu komendą końca jest write_command, która może być jednocześnie komędą początku!
        len_moved_buffer += strlen((const char *)end_command);
    }

#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Contents: '%s'!\r\n", contents);
    ESP_LOGI(USPIFFS_DEB_TAG, "Rest data in incoming buffer: %d!\r\n", len_moved_buffer);
    ESP_LOGI(USPIFFS_DEB_TAG, "Data in incoming buffer: '%s'!\r\n", moved_buffer);
#endif

    return contents;
}