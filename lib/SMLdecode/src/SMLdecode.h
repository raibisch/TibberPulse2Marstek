#pragma once

#include <Arduino.h>

#ifdef ESP32
#ifdef ESP32_C3
  #include <FS.h>
  #include <LittleFS.h>
#else
  #include <FS.h>
  #include <LittleFS.h>
  #include <SPIFFS.h> 
#endif
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

#ifdef SML_TASMOTA
#include "ArduinoJson.h"
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

class SMLdecode
{
  private:
    enum { SMLPAYLOADMAXSIZE = 300 }; // for raw SML message ... could be smaller if TASMOTA json is read
    byte smlpayload[SMLPAYLOADMAXSIZE] {0}; 
    byte* httpGETRequest(const char* http_url);
    bool decodePayload(uint32_t &retval, byte * payload, byte* smlcode, uint smlsize,  uint offset);
   
  protected:
    int16_t _watt = 0;
    double _inputkWh   = 0.0;
    double _outputkWh  = 0.0;

    String _url = "";
    String _passw = "";
    String _usern = "";
    WiFiClient _wificlient;
  
  public:
    SMLdecode() {};
    ~SMLdecode () {};
   bool init(const char* url, const char* user, const char* pass);
   bool read();
   //char* readStatus();
   int16_t getWatt() {return _watt;};
   double getInputkWh() {return _inputkWh;};
   double getOutputkWh() {return _outputkWh;};
};
