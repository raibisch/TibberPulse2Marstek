#include "MarstekAPI.h"

// Parsing MASTEK open-api
// ======================= 

// Marstek API
// https://eu.hamedata.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf




void MarstekAPI::sendUDPData(const char* sJson)
{
   debug_print("->Tx UDP ");
   debug_print(_remoteIPaddr);
   debug_print(":");
   debug_println(_remotePort);
   debug_println(serJsonRequest);

  _UdpRPC.setTimeout(1500);
  _UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  _UdpRPC.UDPPRINT(sJson);
   getUDPData();
  _UdpRPC.endPacket();
}





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
void MarstekAPI::Marstek_GetDevice() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "Marstek.GetDevice";
  jsonRequest["params"]["ble_mac"] = "0";

  serializeJson(jsonRequest, serJsonRequest);
  debug_println(serJsonRequest);
  AsyncWebLog.println("[MARS]-TX->MarstekGetDevice\r\n");

  sendUDPData(serJsonRequest.c_str());

  //_UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  //_UdpRPC.UDPPRINT(serJsonRequest.c_str());
  //_UdpRPC.endPacket();
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
		"soc":	11,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	202.0,
		"bat_capacity":	60.0,
		"rated_capacity":	5120.0
	}
}

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
void MarstekAPI::Bat_GetStatus() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "Bat.GetStatus";
  jsonRequest["params"]["ble_mac"] = "0";

  serializeJson(jsonRequest, serJsonRequest);
  debug_println(serJsonRequest);
  AsyncWebLog.println("[MARS]-TX->BatGetStatus\r\n");

  String sRequest = "{\"id\":0, \"method\":\"Bat.GetStatus\", \"params\":{\"id\":0}}";

  sendUDPData(sRequest.c_str());
  //sendUDPData(serJsonRequest.c_str());

  //_UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  //_UdpRPC.UDPPRINT(serJsonRequest.c_str());
  //_UdpRPC.endPacket();
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
void MarstekAPI::ES_GetMode() 
{
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "ES.GetMode";
  jsonRequest["params"]["id"] = 0;

  serializeJson(jsonRequest, serJsonRequest);

   AsyncWebLog.println("[MARS]-TX->ESGetMode\r\n");
   String sRequest = "{\"id\":0, \"method\":\"ES.GetMode\", \"params\":{\"id\":0}}";
   sendUDPData(sRequest.c_str());
   //sendUDPData(serJsonRequest.c_str());

   //_UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
   //_UdpRPC.UDPPRINT(serJsonRequest.c_str());
   //_UdpRPC.endPacket();
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

void MarstekAPI::ES_GetStatus()
{

   String sRequest = "{\"id\":0, \"method\":\"ES.GetStatus\", \"params\":{\"id\":0}}";
   sendUDPData(sRequest.c_str());

}




/// @brief get Date from UDP
void MarstekAPI::getUDPData() 
{
  uint8_t buffer[1024];
  String status = "";
  _UdpRPC.setTimeout(1500);
  int packetSize = _UdpRPC.parsePacket();
  if (packetSize > 0)
  {
     JsonDocument jsonUDPIn;
     //int rSize = _UdpRPC.read(buffer, 1024);
	 for (int i = 0; i < packetSize; i++)
	 {
		buffer[i] = _UdpRPC.read();
	 }
	 buffer[packetSize] = 0;
	 
     //buffer[rSize] = 0; // add 0 for string end !
     debug_printf("<-Rx UDP PacketSize:%d \n%s", packetSize, (char*)buffer);
 	 AsyncWebLog.print("[MARS]<-RxUDP: ");
     //debug_println((char *)buffer);
     deserializeJson(jsonUDPIn, buffer);
     if (jsonUDPIn["result"].is<JsonVariant>()) 
     {
      JsonObject result = jsonUDPIn["result"];

	  // ES.GetMode
      if (result["ongrid_power"].is<JsonVariant>())
      {
       data.batSoc       = result["bat_soc"];
       data.onGridPower  = result["ongrid_power"];
       data.offGridPower = result["offgrid_power"];
	   AsyncWebLog.printf("ES.GetMode soc:%d pwr:%d", data.batSoc, data.onGridPower);
      }
	  else
	  // ES.GetStatus
	  if (result["soc"].is<JsonVariant>())
      {
        data.batSoc     = result["soc"];
		AsyncWebLog.printf("ES.GetStatus soc:%d ", data.batSoc);
	  }
      else
      // Marstek.GetDevice
	  if (result["device"].is<JsonVariant>())
      {
		int ver = result["ver"];
        AsyncWebLog.printf("Marstek.GetDevice Version:%d", ver);
      }
	  else
	  {
		status = "ERROR:unhandelt";
		debug_print((char*) buffer);
		AsyncWebLog.print((char *) buffer);
	  }
     }
     else 
     {
       status ="ERROR:unknown";
	   debug_print((char *) buffer); 
	   AsyncWebLog.print((char *) buffer);
     }
   
	AsyncWebLog.printf(" %s\r\n",status.c_str());
	debug_printf(" %s\r\n",status.c_str());
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



void MarstekAPI::loop()
{
   if ((millis() - _max_rx_wait) > _timer_rx_wait) 
   {
    _timer_rx_wait = millis();                      // Reset time for next event
    switch (nextRequest)
    {
    case RequestType::ES_GETMODE:
      ES_GetMode();
      //nextRequest = RequestType::ES_GETSTATUS;
      break;
	case RequestType::ES_GETSTATUS:
	  ES_GetStatus();
      nextRequest = RequestType::BAT_GETSTATUS;
	  break;
    case RequestType::BAT_GETSTATUS:
      Bat_GetStatus();
      nextRequest = RequestType::MARSTEK_GETDEVICE;
      break;
	case RequestType::MARSTEK_GETDEVICE:
	  Marstek_GetDevice();
	  nextRequest = RequestType::ES_GETMODE;
	  break;
    default:
      break;
    }
   }

   getUDPData();
}