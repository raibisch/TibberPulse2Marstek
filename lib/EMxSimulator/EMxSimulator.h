#ifndef EMX_SIM_H
#define EMX_SIM_H

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

/* UDP-Client example
 * --> no begin !!
 * --> beginPacket with ip and port !!
 * 
 include <WiFi.h>
#include <WiFiUdp.h>

// WiFi network name and password:
const char * networkName = "your-ssid";
const char * networkPswd = "your-password";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "192.168.0.255";
const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);
  
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop(){
  //only send data when connected
  if(connected){
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    udp.printf("Seconds since boot: %lu", millis()/1000);
    udp.endPacket();
  }
  //Wait for 1 second
  delay(1000);
}
 * 
 */



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

//#define EM1_HTTP_SIMULATION
//#define EM1_UDP_SIMULATION 1

//#define EM3_HTTP_SIMULATION
//#define EM3_UDP_SIMULATION

#ifdef EM1_UDP_SIMULATION
#define EMX_UDP_PORT 2223 // Shelly EM-50 (single phase) 
#else
#define EMX_UDP_PORT 1010 // Shelly 3M 120A
#endif


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
   u_int16_t udpRequestCount = 0; // increment at getPowerData, reset at parseUdpRPC

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

