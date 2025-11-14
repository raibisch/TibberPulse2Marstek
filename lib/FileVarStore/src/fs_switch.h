
#ifdef FS_LITTLEFS
 #include <FS.h>
 #include <LittleFS.h>
 #define myFS LittleFS
#else
 #include <SPIFFS.h>
 #define myFS SPIFFS
#endif 


