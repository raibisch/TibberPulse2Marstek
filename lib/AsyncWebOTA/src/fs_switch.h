
#ifdef FS_LITTLEFS
#include <LittleFS.h>
#define myFS LittleFS
#else
 #ifdef ESP32
  #include <SPIFFS.h>
 #endif
 #define myFS SPIFFS
#endif 


