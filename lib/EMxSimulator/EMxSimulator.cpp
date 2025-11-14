#include "EMxSimulator.h"


//#undef DEBUG_PRINT

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

/// @brief  fix for Marstek (needs values with decimal numbers)
/// @param value 
/// @return 
double EMxSimulator::round2(double value) {
  int ivalue = (int)(value * 100.0 + (value > 0.0 ? 0.5 : -0.5));
  // fix Marstek bug: make sure to have decimal numbers
  if(ivalue % 100 == 0)
  {ivalue++;}
  return ivalue / 100.0;
  }
  

static double powerOld=0;
/// @brief set Power (Watt)
/// @param totalPower 
void EMxSimulator::setPowerData(double totalPower) {
  // for Shelly EM1

  double powerFilterValue = totalPower;

  /* alternative Filter --> auch nicht besser
  double powerFilterValue = totalPower;
  if (abs(totalPower) >= 10)
  {
     powerFilterValue = totalPower -(powerOld*filterFactor);
  }
  powerOld = powerFilterValue;
  */


  double tmp= totalPower - powerOld;
  if (tmp> 100) 
  {
     powerFilterValue = totalPower -(tmp / 2);  
  }

  /*
  // reduce amplification 
  if (abs(totalPower) > 500)
  {
     powerFilterValue = totalPower* filterFactor * 0.5;
  }
  else
  if ((abs(totalPower*filterFactor) >= 10))
  {
     powerFilterValue = totalPower * filterFactor;
  }
  else
  {
    if (totalPower > +10)
    {
      powerFilterValue = 11; // <= 10 regelt Marstek nicht mehr aus !
    }
    else
    if (totalPower < -10)
    {
      powerFilterValue = -11;
    }
    else
    {
      powerFilterValue = totalPower;
    }
  }
  */

  powerOld = totalPower;
  
 
  // unitl now only simulation for Shelly-EM1 (sum phase)
  TotalPower.power_raw          = round2(totalPower);
  TotalPower.power_filterfactor = round2(powerFilterValue);
  TotalPower.apparentPower      = round2(powerFilterValue);
  TotalPower.voltage            = defaultVoltage;
  TotalPower.current            = round2(powerFilterValue / double(defaultVoltage));
  TotalPower.frequency          = defaultFrequency;
  TotalPower.powerFactor        = defaultPowerFactor;
  
  // for Shelly 3EM
  /*
  for (int i = 0; i <= 2; i++) {
    PhasePower[i].power_filterfactor   = round2(pwrFilterValue) * 0.3333;
    PhasePower[i].voltage              = round2(defaultVoltage);
    PhasePower[i].current              = round2(PhasePower[i].power_filterfactor/ PhasePower[i].voltage);
    PhasePower[i].apparentPower        = round2(PhasePower[i].power_filterfactor);
    PhasePower[i].powerFactor          = defaultPowerFactor;
    PhasePower[i].frequency            = defaultFrequency;
  }
  */
 if (udpRequestCount != 0xFFFF)
 {
  udpRequestCount++;
 }
 
  //debug_printf("   %.1f[W]\r\n",totalPower);
}

/// @brief set energy data sum for all phase
/// @param totalEnergyGridSupply  
/// @param totalEnergyGridFeedIn 
void EMxSimulator::setEnergyData(double totalEnergyGridSupply, double totalEnergyGridFeedIn) {
  for (int i = 0; i <= 2; i++) {
    PhaseEnergy[i].consumption = round2(totalEnergyGridSupply * 0.3333);
    PhaseEnergy[i].gridfeedin = round2(totalEnergyGridFeedIn * 0.3333);
  }
  
  //debug_print("Total consuption: ");
  //debug_print(totalEnergyGridSupply);
  //debug_print(" - Total Grid Feed-In: ");
  //debug_println(totalEnergyGridFeedIn);
}

// for EM1
void EMxSimulator::EM1GetDeviceInfo() {
   JsonDocument jsonResponse;
  jsonResponse["name"] = shelly_name;
  jsonResponse["id"] = shelly_name;
  jsonResponse["mac"] = shelly_mac;
  jsonResponse["slot"] = 1;
  jsonResponse["model"] = "SPEM-003CEBEU";
  jsonResponse["gen"] = shelly_gen;
  jsonResponse["fw_id"] = shelly_fw_id;
  jsonResponse["ver"] = "1.4.4";
  jsonResponse["app"] = "ProEM50";
  jsonResponse["auth_en"] = false;
  jsonResponse["profile"] = "monophase";
  serializeJson(jsonResponse, serJsonResponse);
  //debug_println(serJsonResponse);
}

void EMxSimulator::EM1GetStatus()
{
  JsonDocument jsonResponse;
  jsonResponse["id"] = 0;
  jsonResponse["voltage"]    = TotalPower.voltage;
  jsonResponse["current"]    = TotalPower.current;
  jsonResponse["act_power"]  = TotalPower.power_filterfactor;
  jsonResponse["aprt_power"] = TotalPower.apparentPower;
  jsonResponse["pf"] = 1;
  jsonResponse["freq"] = 50;
  jsonResponse["calibration"] = "factory";
  serializeJson(jsonResponse, serJsonResponse);
  //debug_println(serJsonResponse);
}

void EMxSimulator::EM1GetConfig() {
  JsonDocument jsonResponse;
  jsonResponse["id"] = 0;
  jsonResponse["name"] = nullptr;
  jsonResponse["reverse"] = false;
  jsonResponse["ct_type"] = "50A";
  serializeJson(jsonResponse, serJsonResponse);
  //debug_println(serJsonResponse);
}


/// @brief set some default json values 
void EMxSimulator::rpcWrapper() {
  JsonDocument jsonResponse;
  JsonDocument doc;
  deserializeJson(doc, serJsonResponse);
  jsonResponse["id"] = 0;
  jsonResponse["src"] = shelly_name;
  if (strcmp(rpcUser, "EMPTY") != 0) {
    jsonResponse["dst"] = rpcUser;
  }
  jsonResponse["result"] = doc;
  serializeJson(jsonResponse, serJsonResponse);
}

/// @brief parse input from UDP
/// beta version: only pharse EM1.GetStatus (shelly PRO EM1-50)
void EMxSimulator::parseUdpRPC() 
{
  uint8_t buffer[1024];
  int packetSize = _UdpRPC.parsePacket();
  if (packetSize) 
  {
    JsonDocument json;
    // old:
    //int rSize = _UdpRPC.read(buffer, 1024);
    //buffer[rSize] = 0;
    _UdpRPC.read(buffer,packetSize);
    buffer[packetSize] =0;
    debug_printf("<-UDP-Rx %s:%d \r\n", _UdpRPC.remoteIP().toString().c_str(), _UdpRPC.remotePort());
    //debug_println((char *)buffer);

    deserializeJson(json, buffer);
    if (json["method"].is<JsonVariant>()) {
      rpcId = json["id"];
      strcpy(rpcUser, "EMPTY");
      _UdpRPC.beginPacket(_UdpRPC.remoteIP(), _UdpRPC.remotePort());
      // until now not implemented...maybe not necssesary ;-)
      //if (json["method"] == "Shelly.GetDeviceInfo") {
        //GetDeviceInfo();
        //rpcWrapper();
        //_UdpRPC.UDPPRINT(serJsonResponse.c_str());
      //} else if (json["method"] == "EM.GetStatus") {
        //EMGetStatus();
        //rpcWrapper();
        //_UdpRPC.UDPPRINT(serJsonResponse.c_str());
      // else if (json["method"] == "EMData.GetStatus") {
        //EMDataGetStatus();
        //rpcWrapper();
        //_UdpRPC.UDPPRINT(serJsonResponse.c_str());
      //} else if (json["method"] == "EM.GetConfig") {
        //EMGetConfig();
        //rpcWrapper();
        //_UdpRPC.UDPPRINT(serJsonResponse.c_str());
       //} 
       //else 
       if (json["method"] == "EM1.GetStatus") 
       {
#ifdef WEB_APP
       AsyncWebLog.printf("[EM1]GetStatus act_power:%4.2f\r\n", TotalPower.power_filterfactor);
#endif
        EM1GetStatus();
        rpcWrapper();
        debug_printf("->UDP-Tx %s:%d  ", _UdpRPC.remoteIP().toString().c_str(), udpPort);
        //debug_println(serJsonResponse);
        _UdpRPC.UDPPRINT(serJsonResponse.c_str());
        udpRequestCount = 0; // for Timeout !
       } 
       else if (json["method"] == "EM1.GetConfig") 
       {
        EM1GetConfig();
        rpcWrapper();
       
        _UdpRPC.UDPPRINT(serJsonResponse.c_str());
       } else {
        debug_printf("UDP-Rx: unknown request: %s\n", buffer);
       }
       delay(1);
       _UdpRPC.endPacket();
    }
  }
}

// ------------- Public -------------------------------------
/// @brief Init (place in 'init' function)
/// @param port UDP-Port 
/// @return true= ok , false= error
bool EMxSimulator::init(uint16_t port, float filterfactor)
{
    _UdpRPC.begin(port); // Test EM1
    setFilterFactor(filterfactor);
   debug_printf("init EMxSimulator Port:%d\r\n",port);
   return true;
}

/// @brief set values (sum of all phases)
/// @param watt 
/// @param kwhIn 
/// @param kwhOut 
/// @return 
bool EMxSimulator::setData_AllPhase(double watt, double kwhIn, double kwhOut)
{ 
  setPowerData(watt);
  setEnergyData(kwhIn, kwhOut);	
  return true;
}

/// @brief place in 'loop' function
void EMxSimulator::loop()
{
	parseUdpRPC();
}



/// @brief Test Timeout for UDP requests from MARSTEK
/// @return 0=ok 1=no UDP request from MARSTEK
bool EMxSimulator::getRequestTimeout()
{
  //debug_printf("[EMx] udpRequestCount:%d\r\n", udpRequestCount);
  {if ((udpRequestCount > MAX_REQUEST_COUNT) && (udpRequestCount != 0xFFFF)){return true;}else{return false;}}
}


