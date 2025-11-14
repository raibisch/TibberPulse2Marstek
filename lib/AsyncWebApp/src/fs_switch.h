
#ifdef FS_LITTLEFS
#include <LittleFS.h>
#define myFS LittleFS
#else
#ifdef ESP32
//#ifndef ESP32_C3
#include <SPIFFS.h>
//#endif
#endif
#define myFS SPIFFS
#endif 


