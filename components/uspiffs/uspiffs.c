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
            ESP_LOGI(USPIFFS_DEB_TAG, "Error committing changes = %d\r\n", ERR);
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