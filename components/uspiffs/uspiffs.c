#include <uspiffs.h>

static char *start_command = "$usf/";   // Komenda początku uspiffs po niej znajduje się nazwa pliku
static char *write_command = "$usf_w$"; // Komenda początku treści dla pliku
static char *next_command = "$usf_n$";  // Komenda końca treści dla pliku

/******************************************************************************
 * FunctionName : uspiffs_nvs_init
 * Description  : function initializes NVS.
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void uspiffs_nvs_init()
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
            ESP_LOGE(USPIFFS_DEB_TAG, "Flag spiffs_info isn't exist\r\n");
#endif
            nvs_close(my_handle); // Zamknięcie uchwytu do NVS
            return false;         // Zwrócenie false - brak danych zapisanych w SPIFFS (flaga nie istnieje)
        }
#ifdef USPIFFS_DEB_TAG
        else if (ERR == ESP_ERR_NVS_KEY_TOO_LONG)
        {
            ESP_LOGE(USPIFFS_DEB_TAG, "Change nvs key back to 'spiffs_info'!\r\n");
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
 * FunctionName : uspiffs_spiffs_init
 * Description  : function initializes SPIFFS.
 * Parameters   : uspiffs_conf - pointer to esp_vfs_spiffs_conf_t configuration structure
 * Returns      : - ESP_OK                  if success
 *                - ESP_ERR_NO_MEM          if objects could not be allocated
 *                - ESP_ERR_INVALID_STATE   if already mounted or partition is encrypted
 *                - ESP_ERR_NOT_FOUND       if partition for SPIFFS was not found
 *                - ESP_FAIL                if mount or format fails
 *******************************************************************************/
esp_err_t uspiffs_spiffs_init(esp_vfs_spiffs_conf_t *uspiffs_conf)
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
 * FunctionName : correct_file_name
 * Description  : function that checks whether the file name contains illegal characters.
 * Parameters   : file_name - pointer to file name
 * Returns      : 'true' - the file name is correct
 *                'false' - the file name contains illegal characters
 *******************************************************************************/
bool correct_file_name(const char *file_name)
{
    const char not_allowed_chars[] = "<>:\"/\\|?*\t\n "; // Definicja niedozwolonych znaków

    for (int i = 0; i < strlen(file_name); i++) // Przejście przez każdy znak w nazwie
    {
        char c = file_name[i];

        if (strchr(not_allowed_chars, c) != NULL) // Sprawdzenie, czy znak należy do niedozwolonych
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "Uncorrect file name!\r\n");
#endif
            return false; // Znak jest niedozwolony
        }
    }
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Correct file name!\r\n");
#endif
    return true; // Wszystkie znaki są dozwolone
}

/******************************************************************************
 * FunctionName : uspiffs_command_finder
 * Description  : function that checks which command appears first in the received data buffer.
 * Parameters   : moved_buffer - pointer to the buffer with the received data
 * Returns      : NULL - for an error to occur
 *                command - a string of characters corresponding to the command
 *******************************************************************************/
char *uspiffs_first_command_finder(char *moved_buffer)
{
    char *s_commands = strstr((const char *)moved_buffer, (const char *)start_command); // Szukanie komendy 'start_command'
    char *w_commands = strstr((const char *)moved_buffer, (const char *)write_command); // Szukanie komendy 'write_command'

    if ((s_commands == NULL) && (w_commands == NULL)) // Nie znaleziono rzadnej komendy początku
    {
#ifdef USPIFFS_DEB_TAG
        // ESP_LOGI(USPIFFS_DEB_TAG, "Data in: '%s'\r\n", moved_buffer);
        ESP_LOGE(USPIFFS_DEB_TAG, "There is no start command in the buffer!\r\n");
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
 * Description  : function that receives the content of the message between commands uspiffs.
 * Parameters   : bgn_command - message start command
 *                end_command - message end command
 *                moved_buffer - pointer to the buffer with the received data
 *                len_moved_buffer - amount of data received
 * Returns      : NULL - for an error to occur
 *                contents - a string of characters corresponding to the content of the message between commands
 *******************************************************************************/
char *uspiffs_contents(char *bgn_command, char *end_command, uint8_t *moved_buffer, uint16_t *len_moved_buffer)
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
    if (*len_moved_buffer <= bgn_position)                     // Sprawdzenie czy nie występuje overflow
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
    if (*len_moved_buffer < end_position)                      // Sprawdzenie czy nie występuje overflow
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
    *len_moved_buffer -= ((end_position + bgn_position) + (int)strlen((const char *)end_command));
    if (strstr(bgn_command, start_command))
    { // Wyjątek, po komendzie startu komendą końca jest write_command, która może być jednocześnie komędą początku!
        *len_moved_buffer += strlen((const char *)end_command);
    }

#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Contents: '%s'!\r\n", contents);
#endif
    return contents;
}

/******************************************************************************
 * FunctionName : uspiffs_init
 * Description  : function that initializes the necessary hardware peripherals.
 * Parameters   : uart_num - UART peripheral number
 *                uart_conf - user UART configuration parameters, for use default write NULL
 *                uspiffs_conf - user SPIFFS configuration, for use default write NULL
 * Returns      : - ESP_OK                  if success
 *                - ESP_ERR_INVALID_ARG     if UART parameters error
 *                - ESP_ERR_NO_MEM          if SPIFFS objects could not be allocated
 *                - ESP_ERR_INVALID_STATE   if SPIFFS already mounted or partition is encrypted
 *                - ESP_ERR_NOT_FOUND       if partition for SPIFFS was not found
 *                - ESP_FAIL                if SPIFFS mount or format fails
 *******************************************************************************/
esp_err_t uspiffs_init(uart_port_t uart_num, uart_config_t *uart_conf, esp_vfs_spiffs_conf_t *uspiffs_conf)
{
    esp_err_t ERR;

    /////////////////////////////////////////////////////
    ////////////////     UART INIT       ////////////////
    /////////////////////////////////////////////////////
    if (uart_conf == NULL) // Jeżeli użytkownik nie podał własnej konfiguracji wgraj domyślną
    {
        uart_config_t default_uart_conf = USPIFFS_UART_INIT_CONFIG_DEFAULT();
        ERR = uart_param_config(uart_num, &default_uart_conf);
        if (ERR != ESP_OK)
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "Default UART configuration error: '%s'!\r\n", esp_err_to_name(ERR));
#endif
            return ERR;
        }
    }
    else // Jeżeli użytkownik podał własną konfiguracje wgraj tą użytkownika
    {
        ERR = uart_param_config(uart_num, uart_conf);
        if (ERR != ESP_OK)
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "User UART configuration error: '%s'!\r\n", esp_err_to_name(ERR));
#endif
            return ERR;
        }
    }
    ERR = uart_driver_install(uart_num, 2 * RX_BUFFOR_L, 0, 0, NULL, 0);
    if (ERR != ESP_OK)
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "UART driver install error: '%s'!\r\n", esp_err_to_name(ERR));
#endif
        return ERR;
    }

    /////////////////////////////////////////////////////
    ////////////     NVS & SPIFFS INIT       ////////////
    /////////////////////////////////////////////////////
    uspiffs_nvs_init();
    if (uspiffs_conf == NULL) // Jeżeli użytkownik nie podał własnej konfiguracji wgraj domyślną
    {
        esp_vfs_spiffs_conf_t default_spiffs_conf = USPIFFS_SPIFFS_INIT_CONFIG_DEFAULT();
        ERR = uspiffs_spiffs_init(&default_spiffs_conf);
        if (ERR != ESP_OK)
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "Default SPIFFS configuration error: '%s'!\r\n", esp_err_to_name(ERR));
#endif
            return ERR;
        }
    }
    else // Jeżeli użytkownik podał własną konfiguracje wgraj tą użytkownika
    {
        ERR = uspiffs_spiffs_init(uspiffs_conf);
        if (ERR != ESP_OK)
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "User SPIFFS configuration error: '%s'!\r\n", esp_err_to_name(ERR));
#endif
            return ERR;
        }
    }

    return ESP_OK;
}
/******************************************************************************
 * FunctionName : uspiffs_create_file
 * Description  : none
 * Parameters   : file_name -
 *                contents -
 * Returns      : - ESP_OK
 *                - ESP_FAIL
 *******************************************************************************/
esp_err_t uspiffs_create_file(char *file_name, char *contents)
{
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Attempt to create file: '%s'\r\n", file_name);
#endif
    FILE *file = fopen(file_name, "w"); // Otwarcie pliku do zapisu "w" - writte (jeżeli takiego nie ma to utworzenie)
    if (file == NULL)                   // Błąd otwierania pliku
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Failed to open file for writing\r\n");
#endif
        return ESP_FAIL;
    }
    fprintf(file, contents); // Zapisanie treści do pliku
    fclose(file);            // Zamknięcie pliku
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "File written successfuly\r\n"); // Informacja o zakończeniu zapisu
#endif
    return ESP_OK;
}

/******************************************************************************
 * FunctionName : uspiffs_read_data
 * Description  : function that monitors the UART for a specified period of time
 *                in order to read the content contained between uspiffs commands.
 * Parameters   : uart_num - UART peripheral number
 *                waiting_time - waiting time for data in seconds
 * Returns      : - ESP_OK          if success
 *                - ESP_FAIL        if failed with buffer allocation
 *******************************************************************************/
esp_err_t uspiffs_read_data(uart_port_t uart_num, int64_t waiting_time)
{
    uint8_t *u_data = (uint8_t *)malloc(RX_BUFFOR_L); // Tymczasowy bufor dla przychodzących danych
    bool file_name_assigned = false;

    if (u_data == NULL)
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Memory allocation error for buffer!\r\n");
#endif
        return ESP_FAIL;
    }
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Start reading data from UART...\r\n");
#endif

    int64_t timer_start = esp_timer_get_time(); // Zapisz czas początkowy
    waiting_time *= 1000000;                    // Przeliczenie z mikrosekund na sekundy

    char *file_name = NULL; // Wskaźnik na nazwę pliku
    char *contents = NULL;  // Wskaźnik na treść pliku

    while (esp_timer_get_time() - timer_start < waiting_time)
    {
        uint16_t len_data_comming = uart_read_bytes(uart_num, u_data, RX_BUFFOR_L, 100 / portTICK_RATE_MS); // Odczyt danych z UART

        if (len_data_comming > RX_BUFFOR_L)
        {
#ifdef USPIFFS_DEB_TAG
            ESP_LOGE(USPIFFS_DEB_TAG, "Too many incoming data!\r\n");
#endif
        }
        else if (len_data_comming > 0)
        {
            uint16_t current_index = 0;           // Indeks do bieżącej pozycji w buforze
            uint16_t pro_data = 0;                // Ilość danych przetworzonych
            uint16_t rem_data = len_data_comming; // Ilość danych do przetworzenia

            while (rem_data) // Pętla przetwarzająca dane
            {
                char *command = uspiffs_first_command_finder((char *)(u_data + current_index)); // Znajdź komendę początkową

                if (command == NULL)
                {
#ifdef USPIFFS_DEB_TAG
                    ESP_LOGE(USPIFFS_DEB_TAG, "There are no further commands to process!\r\n");
#endif
                    break; // Jeśli nie ma więcej komend, zakończ pętlę
                }

                // Sprawdzenie czy pierwsza komenda odpowiada za nazwę pliku
                if (strstr(command, start_command))
                {
                    // Ustawienia wskaźnika na treść z nazwą pliku
                    contents = uspiffs_contents(command, write_command, (uint8_t *)(u_data + current_index), &rem_data);
                    // file_name = uspiffs_contents(command, write_command, (uint8_t *)(u_data + current_index), &rem_data);
                    // if (correct_file_name(file_name))
                    if (correct_file_name(contents)) // Sprawdzenie czy nazwa pliku jest poprawna
                    {
                        file_name_assigned = true; // Potwierdzenie, że istnieje plik do którego można przypisać treść
                    }
                }
                // Sprawdzenie czy pierwsza komenda odpowiada za odczyt treści do pliku
                else if ((strstr(command, write_command)) && file_name_assigned)
                { // "Przypisywanie treści następuje wówczas, gdy jest tą zapisać do czego"- Future YODA
                    contents = uspiffs_contents(command, next_command, (uint8_t *)(u_data + current_index), &rem_data);
                }
                else
                {
#ifdef USPIFFS_DEB_TAG
                    ESP_LOGE(USPIFFS_DEB_TAG, "Wrong command or uncorecct file name!\r\n");
#endif
                    break; // Jeśli nie ma więcej komend, zakończ pętlę
                }

                // Sprawdzenie czy między komendami jest treść
                if (contents == NULL)
                {
#ifdef USPIFFS_DEB_TAG
                    ESP_LOGE(USPIFFS_DEB_TAG, "Error with processing (begin) command: '%s'\r\n", command);
                    ESP_LOGE(USPIFFS_DEB_TAG, "There's no contents !\r\n");
#endif
                    free(contents); // Zwolnienie pamięci na treść
                    break;          // Błąd przetwarzania
                }
                else
                {
                    pro_data = (len_data_comming - rem_data); // Oblicz długość przetworzonych danych
                    free(contents);                           // Zwolnienie pamięci na treść
#ifdef USPIFFS_DEB_TAG
                    ESP_LOGI(USPIFFS_DEB_TAG, "Count of incoming data: '%d'\r\n", len_data_comming);
                    ESP_LOGI(USPIFFS_DEB_TAG, "Remaining data: '%d'\r\n", rem_data);
                    ESP_LOGI(USPIFFS_DEB_TAG, "Processed data: '%d'\r\n", pro_data);
                    ESP_LOGI(USPIFFS_DEB_TAG, "Current index: '%d'\r\n", current_index);
#endif
                }

                current_index = 0;         // Przesuwanie zawsze następuje od początku
                current_index += pro_data; // Przesuń indeks do następnej komendy
            }
        }
    }

    free(u_data);
    return ESP_OK;
}