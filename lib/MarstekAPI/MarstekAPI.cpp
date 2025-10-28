#include "MarstekAPI.h"

// Parsing MASTEK open-api
// ======================= 

// Marstek API
// https://eu.hamedata.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf



/*
Marstek.GetDevice: OK (Mastek Venus V3 Firmware:139)
========== 
--> request:
{"id":0, "method":"Marstek.GetDevice", "params":{"ble_mac":"0"}}

<-- response: (??=individual values)
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5df??",
	"result":	{
		"device":	"VenusE 3.0",
		"ver":	122,
		"ble_mac":	"009b08a5df??",
		"wifi_mac":	"444e6d6b2f??",
		"wifi_name":	"???",
		"ip":	"192.168.2.??"
	}
}

{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"device":	"VenusE 3.0",
		"ver":	139,
		"ble_mac":	"009b08a5dfxx",
		"wifi_mac":	"444e6d6b2fxx",
		"wifi_name":	"xxxx",
		"ip":	"192.168.2.xx"
	}

*/

/// @brief get device info from Marstek
/// could be send as UDP-Broardcast
void MarstekAPI::MarstekGetDevice() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "Marstek.GetDevice";
  jsonRequest["params"]["ble_mac"] = "0";

  serializeJson(jsonRequest, serJsonRequest);
  debug_println(serJsonRequest);
  AsyncWebLog.println("[MARS]-TX->MarstekGetDevice\r\n");
  _UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  _UdpRPC.UDPPRINT(serJsonRequest.c_str());
  _UdpRPC.endPacket();
}






/*
Bat.GetStatus 
=============
--> request:
{"id":0, "method":"Bat.GetStatus", "params":{"id":0}}

<-- : NO RESPONSE !!!  Marstek V3 Firmware:122
<-- response example:  Marstek V3 Firmware 139
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"soc":	12,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	261.0,
		"bat_capacity":	61.0,
		"rated_capacity":	5120.0
	}
}

{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"soc":	85,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	321.0,
		"bat_capacity":	436.0,
		"rated_capacity":	5120.0
	}
}


{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"soc":	11,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	235.0,
		"bat_capacity":	60.0,
		"rated_capacity":	5120.0
	}
}

*/
/// @brief  "Bat.GetStatus"
void MarstekAPI::BatGetStatus() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "Bat.GetStatus";
  jsonRequest["params"]["ble_mac"] = "0";

  serializeJson(jsonRequest, serJsonRequest);
  debug_println(serJsonRequest);
  AsyncWebLog.println("[MARS]-TX->BatGetStatus\r\n");

  _UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  _UdpRPC.UDPPRINT(serJsonRequest.c_str());
  _UdpRPC.endPacket();
}


/*
ES.GetMode  
==========
--> request:
{"id":0, "method":"ES.GetMode", "params":{"id":0}}


<-- response Venus V3 Firmware 122:
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"mode":	"Auto",
		"ongrid_power":	130,
		"offgrid_power":	0,
		"bat_soc":	76
	}
}

<---- response  Venus V3 Firmware 139
example 1 : Bat discharge
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"mode":	"Auto",
		"ongrid_power":	294,
		"offgrid_power":	0,
		"bat_soc":	85,
		"ct_state":	0,
		"a_power":	0,
		"b_power":	0,
		"c_power":	0,
		"total_power":	0
	}
}

example 2: Bat is empty
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"mode":	"Auto",
		"ongrid_power":	0,
		"offgrid_power":	0,
		"bat_soc":	11,
		"ct_state":	0,
		"a_power":	0,
		"b_power":	0,
		"c_power":	0,
		"total_power":	0
	}
}


example 3: Bat charging
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"mode":	"Auto",
		"ongrid_power":	-1033,
		"offgrid_power":	0,
		"bat_soc":	11,
		"ct_state":	0,
		"a_power":	0,
		"b_power":	0,
		"c_power":	0,
		"total_power":	0
	}
} 
*/

/// @brief  "ES.GetMode"
void MarstekAPI::ESGetMode() 
{
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "ES.GetMode";
  jsonRequest["params"]["id"] = 0;

  serializeJson(jsonRequest, serJsonRequest);

   debug_print(" UDP send to:");
   debug_print(_remoteIPaddr);
   debug_print(":");
   debug_println(_remotePort);
   debug_println(serJsonRequest);

   AsyncWebLog.println("[MARS]-TX->ESGetMode\r\n");

   _UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
   _UdpRPC.UDPPRINT(serJsonRequest.c_str());
   _UdpRPC.endPacket();
}



/* ES.GetStatus
   ===========

{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"id":	0,
		"soc":	11,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	233.0,
		"bat_capacity":	60.0,
		"rated_capacity":	5120.0
	}
}

--> until now not implemented
*/




/// @brief get Date from UDP
void MarstekAPI::getUDPData() 
{
  uint8_t buffer[1024];
  String status = "OK";
  int packetSize = _UdpRPC.parsePacket();
  if (packetSize) http://192.168.2.96/
  {
    JsonDocument jsonUDPIn;
    int rSize = _UdpRPC.read(buffer, 1024);
    buffer[rSize] = 0; // add 0 for string end !
    debug_print("<-Rx UDP packet:");
	AsyncWebLog.print("[MARS]<-RxUDP: ");
    //debug_println((char *)buffer);
    deserializeJson(jsonUDPIn, buffer);
    // ES.Mode
    if (jsonUDPIn["result"].is<JsonVariant>()) 
    {
      JsonObject result = jsonUDPIn["result"];
	  // ES.GetStatus
      if (result["ongrid_power"].is<JsonVariant>())
      {
       data.batSoc       = result["bat_soc"];
       data.onGridPower  = result["ongrid_power"];
       data.offGridPower = result["offgrid_power"];
	   AsyncWebLog.printf("EM.GetStatus soc:%d ", data.batSoc);
      }
      else
      // Marstek.GetDevice
	  if (result["device"].is<JsonVariant>())
      {
        AsyncWebLog.printf("Marstek.GetDevice");
      }
	  else
	  {
		status = "unhandelt result";
	  }
    }
    else 
    {
        status ="unknown request";
    }
	AsyncWebLog.printf(" status:%s\r\n",status.c_str());
	debug_printf(" status:%s\r\n",status.c_str());
    
   _timer_rx_wait = millis();  // ready for next polling 
  }
}


// Public 
bool MarstekAPI::init(String ip, uint16_t port)
{
   _UdpRPC.begin(port); // MARSTEK send on same port back !! ...some kind of strange handling ;-)
   _remotePort = port;
   _remoteIPaddr.fromString(ip);
   debug_printf("init MarstekAPI IP:%s Port:%d\r\n",_remoteIPaddr.toString().c_str(), _remotePort);
   _timer_rx_wait = millis();
  return true;
}

void MarstekAPI::rxloop()
{
	getUDPData();
}


void MarstekAPI::txloop()
{
   if (millis() - _max_rx_wait > _timer_rx_wait) 
   {
    _timer_rx_wait = millis();                      // Reset time for next event
    switch (nextRequest)
    {
    case RequestType::ES_GETMODE:
      ESGetMode();
      // ...until now only this RequestType
      // for other Tx-Polling set to next ReqestType
      // nextRequest = RequestType::MARSTEK_GETDEVICE;
      break;
    case RequestType::BAT_GETSTATUS:
      BatGetStatus();
      nextRequest = RequestType::ES_GETMODE;
      break;
	case RequestType::MARSTEK_GETDEVICE:
	  MarstekGetDevice();
	  nextRequest = RequestType::ES_GETMODE;
	  break;
    // ... add more RequestTypes 
    default:
      break;
    }
   }
}