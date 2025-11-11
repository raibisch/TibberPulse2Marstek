#pragma once

#include "Arduino.h"

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

// my includes
#include "ESPAsyncWebServer.h"
#include "fs_switch.h"
#include "SMLdecode.h"
#include "AsyncWebLog.h"

#ifdef DEBUG_PRINT
#define debug_begin(...) Serial.begin(__VA_ARGS__);
#define debug_print(...) Serial.print(__VA_ARGS__);
#define debug_write(...) Serial.write(__VA_ARGS__);
#define debug_println(...) Serial.println(__VA_ARGS__);
#define debug_printf(...) Serial.printf(__VA_ARGS__);
#else
#define debug_begin(...)
#define debug_print(...)
#define debug_printf(...)
#define debug_write(...)
#define debug_println(...)
#endif

#define JSON_URL "/v1/json"

class EcoTrackSimulator 
{
public:
    EcoTrackSimulator() {};
    ~EcoTrackSimulator() {};
    //String sWeblog;
    

#ifdef WEB_APP
    // use existing AsyncWebserver
    void begin();
    bool setData_AllPhase(double watt, double grindIn, double grindOut);
    String buildJsonRequest();
    bool getRequestTimeout();
#else
     // for future extention run extra Webserver ...not implemented now 
     void begin(SMLdecode *decoder) {_decoder = decoder; debug_println("no WEB_APP !!..compile with 'WEB_APP' definiton");}
#endif 
private:
    int _watt = 0;
    int _watt_avg = 0;
    int _watt_old = 0;
    uint32_t _WhIn   = 0; // Achtung hier Wh nicht kWh !!
    uint32_t _WhOut  = 0;
    uint16_t _requestCount = 0; 

   
};


