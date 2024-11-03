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
 * FunctionName : uspiffs_get_contents
 * Description  : none
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
char *uspiffs_get_contents(const char *contents_start, const char *contents_end)
{
    const char *pos = strstr(contents_start, contents_end);
    if (pos)
    {
        // Obliczamy długość różnicy
        size_t diff_length = pos - contents_start;
        // Tworzymy nowy bufor na wynik
        char *result = (char *)malloc(diff_length + 1); // +1 na znak null
        if (result)
        {
            strncpy(result, contents_start, diff_length);
            result[diff_length] = '\0'; // Null-terminator
        }
        return result;
    }
// Zwracamy NULL, jeśli 'contents_end' nie jest częścią 'contents_start'
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Wrong contents buffer combination!\r\n");
#endif
    return NULL;
}

/******************************************************************************
 * FunctionName : uspiffs_artifact_finder
 * Description  : none
 * Parameters   : none
 * Returns      : 'true' - artifact found.
 *                'false' - artifact not found.
 *******************************************************************************/
bool uspiffs_artifact_finder(char *command, char *command_buffer, char *moved_buffer, uint16_t len_data)
{
    uint16_t command_len = (uint16_t)strlen((const char *)command); // Ilość danych jakie zajmuje komenda

    // Ilość znaków jakie znajdują się w buforze 'moved_buffer' przed wystąpieniem komendy
    uint16_t len_diffrence = (uint16_t)strcmp((const char *)command_buffer, (const char *)moved_buffer);

#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Incoming data count: %d\r\n", len_data);
    ESP_LOGI(USPIFFS_DEB_TAG, "Chars before command: %d\r\n", len_diffrence);
    ESP_LOGI(USPIFFS_DEB_TAG, "Command length: %d\r\n", command_len);
#endif
    if ((len_diffrence + command_len) > len_data)
    { // Jeżeli ilość znaków przed komendą, wraz z znakami potrzebnymi na wypisanie komendy
// jest większa od ilości danych przychodzących. Oznacza wystąpienie artefaktu.
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "The incoming data does not contain a command!\r\n");
#endif
        return true;
    }

    return false;
}

/******************************************************************************
 * FunctionName : uspiffs_contents
 * Description  : none
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
char *uspiffs_contents(char *bgn_command, char *end_command, uint8_t *moved_buffer, uint16_t len_moved_buffer)
{
    // Wymagane jest asynchroniczne odbieranie danych w przerwaniu, a nie co jakiś czas, albo inne rozwiązanie!
    // Odbieranie co jakiś czas, może powodować, że komenda końca przejdzie do następnego pakietu!

    /**********************************************************************************/
    /*  Sprawdzenie czy obie komendy początku i końca są częsią danych przychodzących  */
    /**********************************************************************************/

    // Bufor 'contents_start' zawiera całość danych (z 'moved_buffer') znajdującą się za komendą początku
    char *contents_start = strstr((const char *)moved_buffer, (const char *)bgn_command); // Szukanie komendy początku
    if (contents_start == NULL)                                                           // Nie znaleziono początku informacji
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Not found contents start!\r\n");
#endif
        return NULL;
    }
    // Bufor 'contents_end' zawiera całość danych (z 'moved_buffer') znajdującą się za komenda końca
    char *contents_end = strstr((const char *)moved_buffer, (const char *)end_command); // Szukanie końca informacji
    if (contents_end == NULL)                                                           // Nie znaleziono końca informacji
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Not found contents end!\r\n");
#endif
        return NULL;
    }

    /**********************************************************************************/
    /*   Sprawdzenie czy, któraś z komend nie jest artefaktem występującym w buforze  */
    /**********************************************************************************/

    bool artifact_err = false;
    // Sprawdzamy czy komenda początku jest częścią przychodzących danych
    artifact_err = uspiffs_artifact_finder(bgn_command, contents_start, (char *)moved_buffer, len_moved_buffer);
    if (artifact_err)
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Artifact error begin command: '%s'!\r\n", bgn_command);
#endif
        return NULL;
    }
    // Sprawdzamy czy komenda końca jest częścią przychodzących danych
    artifact_err = uspiffs_artifact_finder(end_command, contents_end, (char *)moved_buffer, len_moved_buffer);
    if (artifact_err)
    {
#ifdef USPIFFS_DEB_TAG
        ESP_LOGE(USPIFFS_DEB_TAG, "Artifact error end command: '%s'!\r\n", end_command);
#endif
        return NULL;
    }

    /**********************************************************************************/
    /*                  Sprawdzenie czy wiadomość z bufora ma treść                   */
    /**********************************************************************************/

    contents_start += strlen((const char *)bgn_command);                // Przesunięcie początku bufora na pozycję z treścią
    char *content = uspiffs_get_contents(contents_start, contents_end); // Wydobycie treści
    if (content != NULL)
    {
        contents_end += strlen((const char *)bgn_command); // Przesunięcie na koniec bufora
        // Tutaj trzeba by było ściągnąć ilość 'len_moved_buffer'
        // oraz przesunąć całość bufora 'moved_buffer'
    }
#ifdef USPIFFS_DEB_TAG
    ESP_LOGI(USPIFFS_DEB_TAG, "Contents: '%s'!\r\n", content);
    ESP_LOGI(USPIFFS_DEB_TAG, "Inside buffer 'contents_start': '%s'!\r\n", contents_start);
    ESP_LOGI(USPIFFS_DEB_TAG, "Inside buffer 'contents_end': '%s'!\r\n", contents_end);
#endif

    return content;
}