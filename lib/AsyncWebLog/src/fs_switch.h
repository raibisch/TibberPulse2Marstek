#pragma once

#ifdef FS_LITTLEFS
#pragma message("Info : USE 'LITTLEFS' Filesystem")
#include <LittleFS.h>
#define myFS LittleFS
#else
#pragma message("Info : USE 'SPIFFS' Filesystem")
#ifdef ESP32
//#ifndef ESP32_C3
#include <SPIFFS.h>
//#endif
#endif
#define myFS SPIFFS
#endif 


