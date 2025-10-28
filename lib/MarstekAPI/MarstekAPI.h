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

#define REMOTE_TX_PORT 30000
#define LOCAL_RX_PORT  30000

class MarstekAPI
{
  private:  
 
   WiFiUDP    _UdpRPC;
   uint16_t  _remotePort = REMOTE_TX_PORT; 
   uint16_t  _localRxPort = LOCAL_RX_PORT;
   IPAddress _remoteIPaddr;
#ifdef ESP32
#define UDPPRINT print
#else
#define UDPPRINT write
#endif
#endif

   // wait for UDP answer (Rx) before resending
   const unsigned long   _max_rx_wait = 1500; // 1.5 sec.
   unsigned long         _timer_rx_wait = 0;
    
   String serJsonResponse;
   String serJsonRequest;
   // Helper Functions
   //JsonVariant resolveJsonPath(JsonVariant variant, const char *path);
   void getUDPData();

   // put this to public for external call
   void MarstekGetDevice();
   void ESGetMode();
   void BatGetStatus();
   
   struct MarstekData {
     double onGridPower;
     double offGridPower;
     double feedIn;
     double feedOut;
     int    batSoc;
   };
   MarstekData data;

   enum RequestType 
   {
    ES_GETMODE = 0,
    BAT_GETSTATUS,
    MARSTEK_GETDEVICE,

    MAX_REQUESTTYPE = 0xff
   };
   RequestType nextRequest = RequestType::ES_GETMODE;


  public:
    MarstekAPI() {};
    ~MarstekAPI() {};
   bool init(String ip, uint16_t port);
   void rxloop();
   void txloop();
   inline int getSOC() {return data.batSoc;};
   inline int getOnGridPower() {return data.onGridPower;};

};


