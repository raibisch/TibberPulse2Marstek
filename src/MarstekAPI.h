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

#define MARSTEKAPI_UDP_PORT 30000

class MarstekAPI
{
  private:  
 
   WiFiUDP    _UdpRPC;
   uint16_t  _remotePort = MARSTEKAPI_UDP_PORT; 
   IPAddress _remoteIPaddr;
#ifdef ESP32
#define UDPPRINT print
#else
#define UDPPRINT write
#endif
#endif
    
   String serJsonResponse;
   String serJsonRequest;
   // Helper Functions
   JsonVariant resolveJsonPath(JsonVariant variant, const char *path);
   void getUDPData();
   void GetUDP_Packet();


   struct MarstekData {
     double onGridPower;
     double offGridPower;
     double feedIn;
     double feedOut;
     int    batSoc;
   };
   MarstekData data;

  public:
    MarstekAPI() {};
    ~MarstekAPI() {};
   bool init(String ip, uint16_t port);
   void loop();
   void GetDevice();
   void ESGetMode();

   inline int getSOC() {return data.batSoc;};
   inline int getOnGridPower() {return data.onGridPower;};

};


