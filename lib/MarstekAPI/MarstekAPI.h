#ifndef MARSTEK_API_H
#define MARSTEK_API_H

#include <Arduino.h>

#ifdef ESP32
  #include <SPIFFS.h> 
  #include <HTTPClient.h>
  #include <AsyncTCP.h>
  //#include <ESPmDNS.h>
  #include <WiFi.h>
  #include <HTTPClient.h> 
  #include <Preferences.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  //#include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>
#include "ArduinoJson.h"


#ifdef WEB_APP
#include "AsyncWebLog.h" // for logging in Webbroweser
#endif



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

#define MARSTEK_MAX_REQUEST_COUNT 20 

class MarstekAPI
{
  private:  
   
   WiFiUDP    _UdpRPC;
   uint16_t        _remotePort  = 30000; 
   const uint16_t  _localRxPort = 30000;
   IPAddress _remoteIPaddr;
#ifdef ESP32
#define UDPPRINT print
#else
#define UDPPRINT write
#endif
#endif

   // wait for UDP answer (Rx) before resending
   unsigned long   _pollRateMsec = 30000; // 30sec
   uint16_t udpRequestCount = 0;
    
   String serJsonResponse;
   String serJsonRequest;
   // Helper Functions
   //JsonVariant resolveJsonPath(JsonVariant variant, const char *path);
   void getUDPData();
   void sendUDPData(const char* sJson);

   // put this to public for external call
   void Marstek_GetDevice();
   void ES_GetMode();
   void ES_GetStatus();
   void Bat_GetStatus();
   
   struct MarstekData {
     int onGridPower;
     int offGridPower;
     int    batSoc;
   };
   MarstekData data;

   enum RequestType 
   {
    ES_GETMODE = 0,
    ES_GETSTATUS,
    BAT_GETSTATUS,
    MARSTEK_GETDEVICE,

    MAX_REQUESTTYPE = 0xff
   };
   RequestType nextRequest = RequestType::ES_GETMODE;


  public:
    MarstekAPI() {};
    ~MarstekAPI() {};
   bool init(String ip, uint16_t port, uint16_t pollRateSec);
   void loop();
   inline int getSOC() {return data.batSoc;};
   inline int getOnGridPower() {return data.onGridPower;};
   inline void setPollRateSec(uint16_t pollRateSec) {_pollRateMsec = pollRateSec * (unsigned long)1000;};
   bool getRequestTimeout();
};


