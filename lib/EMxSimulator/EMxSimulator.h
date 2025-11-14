#ifndef EMX_SIM_H
#define EMX_SIM_H

#include <Arduino.h>


#ifdef ESP32
#ifndef ESP32_C3
  #include <SPIFFS.h> 
#endif
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

//#define EM1_HTTP_SIMULATION
//#define EM1_UDP_SIMULATION 1

//#define EM3_HTTP_SIMULATION
//#define EM3_UDP_SIMULATION

#ifdef EM1_UDP_SIMULATION
#define EMX_UDP_PORT 2223 // Shelly EM-50 (single phase) 
#else
#define EMX_UDP_PORT 1010 // Shelly 3M 120A
#endif

#define MAX_REQUEST_COUNT 100

class EMxSimulator
{
  private:  
    //byte* httpGETRequest(const char* http_url);
    
    // Helper Functions
    double round2(double value);
    void rpcWrapper();
    void setPowerData(double totalPower);
    void setEnergyData(double totalEnergyGridSupply, double totalEnergyGridFeedIn);
    void parseUdpRPC();
    void EM1GetDeviceInfo();
    void EM1GetStatus();
    void EM1GetConfig();
    
   struct PowerData {
     double current;
     double voltage;
     double power_raw;
     double power_filterfactor;
     double apparentPower;
     double powerFactor;
     double frequency;
   };

   struct EnergyData {
    double gridfeedin;
    double consumption;
   };

   PowerData TotalPower;    
   PowerData PhasePower[3];
   EnergyData PhaseEnergy[3];
   String serJsonResponse;
   double filterFactor;
   u_int16_t udpRequestCount = 0xffff; // increment at getPowerData, reset at parseUdpRPC

   const uint8_t defaultVoltage = 230;
   const uint8_t defaultFrequency = 50;
   const uint8_t defaultPowerFactor = 1;

   char shelly_fw_id[32] = "20241011-114455/1.4.4-g6d2a586";
   char shelly_mac[13]   = "30aea48fdf30";
   char shelly_gen[2] = "2";
   int rpcId = 1;
   char rpcUser[20] = "user_1";
  #ifdef EM1_UDP_SIMULATION
   char shelly_name[26] = "shellyproem1-30aea48fdf30";
  #else 
   char shelly_name[26] = "shellypro3em-30aea48fdf30";
  #endif

  protected:

    WiFiUDP    _UdpRPC;
    uint udpPort = EMX_UDP_PORT;
#ifdef ESP32
#define UDPPRINT print
#else
#define UDPPRINT write
#endif


#if (defined EM1_HTTP_SIMULATION) || (defined EM3_HTTP_SIMULATION)
WiFiClient _wificlient;
#endif

  public:
    EMxSimulator() {};
    ~EMxSimulator () {};
   bool init(uint16_t port, float filterfactor=1.0);
   void loop();
   bool setData_AllPhase(double watt, double grindIn, double grindOut);
   void setFilterFactor(double ff) { filterFactor = ff;};
   bool getRequestTimeout();
};

#endif

