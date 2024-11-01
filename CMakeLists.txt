# Minimalna wersja CMake
cmake_minimum_required(VERSION 3.5)

# Dołączenie pliku CMake z SDK (ten plik ustawia środowisko ESP-IDF)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Ustawienie zmiennej środowiskowej z ścieżką do własnych komponentów (wewnątrz kontenera)
set(EXTRA_COMPONENT_DIRS /opt/esp/ESP8266_RTOS_SDK/components
                         "${PROJECT_DIR}/components")

# Nazwa projektu
project(esp8266_uspiffs)