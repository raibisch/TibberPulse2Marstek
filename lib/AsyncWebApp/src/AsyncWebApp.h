#ifndef AsyncWebApp_h
#define AsyncWebApp_h

#include "Arduino.h"
#include "stdlib_noniso.h"
#include <functional>

#ifdef ESP32
  #include <SPIFFS.h> 
  #include <HTTPClient.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
  #include <WiFi.h>
  #include <HTTPClient.h> 
  #include <Preferences.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266mDNS.h>
#endif

#include <ESPAsyncWebServer.h>

//#include "Update.h"

#include "fs_switch.h"

////////////////////////////////////////


// by JG: point to Webpage in SPIFF
#define OTA_URL "/ota.html"

class AsyncWebAppClass
{
public:
    void begin(AsyncWebServer *server);
    static String parseHtmlTemplate(const String& var) { return "";}

private:
    AsyncWebServer *_server;
     
};
extern AsyncWebAppClass AsyncWebApp;
#endif
