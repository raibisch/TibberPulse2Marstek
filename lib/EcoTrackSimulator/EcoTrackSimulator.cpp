//*****************************************************
// Simulate everhome EcoTracker as CT-Device for Mastek
// Testet with:
// Marstek Venus V3 Firmware V139 CT-Device: 'EcoTracker(Beta)'
// 
// until now only tested with ESP32 
//****************************************************

#include "EcoTrackSimulator.h"

 /// @brief build json message (called at webserver request)
 /// @return json message as String
 String EcoTrackSimulator::buildJsonRequest()
 {
    _requestCount = 0;
    debug_printf      ("[MARS]<->[ECOT] v1/json {\"power\":%d... \r\n", _watt);
    AsyncWebLog.printf("[MARS]<->[ECOT] v1/json {\"power\":%d... \r\n", _watt);
    // {\"power\":" + s(0cpwr) + ",\"powerAvg\":" + s(0cpwr) + ",\"energyCounterIn\":" + s(0sml[2]*1000) + ",\"energyCounterOut\":" + s(0sml[3]*1000) + "}"
    String sFetch = "{\"power\":";
    sFetch += _watt;
    sFetch += ",\"powerAvg\":";
    sFetch += _watt_avg;
    sFetch += ",\"energyCounterIn\":";
    sFetch += _WhIn;
    sFetch += ",\"EnergyCounterOut\":";
    sFetch += _WhOut;
    sFetch += "}";
   return sFetch;
 }


/// @brief set values (sum of all phases)
/// @param watt 
/// @param kwhIn 
/// @param kwhOut 
/// @return true
bool EcoTrackSimulator::setData_AllPhase(double watt, double kwhIn, double kwhOut)
{ 

  double powerFilterValue = watt;

  // reduce amplification 
  if (abs(watt) > 500)
  {
     powerFilterValue = watt * 0.9;
  }
  /**
  else
  if ((abs(watt*_filterFactor) >= 10))
  {
     powerFilterValue = watt * _filterFactor;
  }
  else
  {
    if (watt > +10)
    {
      powerFilterValue = 11; // <= 10 regelt Marstek nicht mehr aus !
    }
    else
    if (watt < -10)
    {
      powerFilterValue = -11;
    }
    else
    {
      powerFilterValue = watt;
    }
  }*/

  _watt = (uint)powerFilterValue;
  _watt_avg = (uint)(powerFilterValue + _watt_old) / 2;
  _watt_old = (uint)powerFilterValue;

  _WhIn = (uint)(kwhIn  *1000);
  _WhOut= (uint)(kwhOut *1000); 
  _requestCount++;
  return true;
}

/// @brief Test Timeout for UDP requests from MARSTEK
/// @return 0=ok 1=no UDP request from MARSTEK
bool EcoTrackSimulator::getRequestTimeout()
{
  //debug_printf("[EMx] udpRequestCount:%d\r\n", udpRequestCount);
  {if (_requestCount > 50){return true;}else{return false;}}
}


/// @brief init ( call in setup() )
/// set up mDNS for Marstek search for EcoTracker
void EcoTrackSimulator::begin(float filterFactor)
{
  _filterFactor = filterFactor;
  // Set up mDNS responder
  String baseName = "ecoctracker-01";
  if (!MDNS.begin(baseName.c_str()))
  {
    debug_println("Error setting up MDNS responder!");
  }

#ifdef ESP32
  MDNS.addService("_http", "_tcp", 80);
  MDNS.addService("_everhome","_tcp",80);
  /* wird nicht gebraucht !
  mdns_txt_item_t serviceTxtData[4] = 
  {
    { "fw_id", SYS_Version},
    { "arch", "esp8266" },
    { "id", baseName.c_str() }
  };
  */
  mdns_service_instance_name_set("_http", "_tcp", baseName.c_str());
  //mdns_service_txt_set("_http", "_tcp", serviceTxtData, 4);

  mdns_service_instance_name_set("_everhome", "_tcp", baseName.c_str());
  //mdns_service_txt_set("_everhome", "_tcp", serviceTxtData, 4);

#else
  hMDNSService = MDNS.addService(0, "_http", "_tcp", 80);
  hMDNSService2 = MDNS.addService(0, "_everhome", "_tcp", 80);
  if (hMDNSService) 
  {
    MDNS.setServiceName(hMDNSService, baseName.c_str());
    MDNS.addServiceTxt(hMDNSService, "fw_id", SYS_Version);
    MDNS.addServiceTxt(hMDNSService, "arch", "esp8266");
    MDNS.addServiceTxt(hMDNSService, "id", baseName.c_str());
  }
  if (hMDNSService2) 
  {
    MDNS.setServiceName(hMDNSService2, baseName.c_str());
    MDNS.addServiceTxt(hMDNSService2, "fw_id", SYS_Version);
    MDNS.addServiceTxt(hMDNSService2, "arch", "esp8266");
    MDNS.addServiceTxt(hMDNSService2, "id", baseName.c_str());
  }
#endif
  debug_println("EcoTracker mDNS started");
 
}

