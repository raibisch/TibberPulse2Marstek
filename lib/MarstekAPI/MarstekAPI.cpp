#include "MarstekAPI.h"

// Parsing MASTEK open-api
// ======================= 

/// @brief fracment from old code --> ???
/// @param variant 
/// @param path 
/// @return 
/*
JsonVariant MarstekAPI::resolveJsonPath(JsonVariant variant, const char *path) {
  for (size_t n = 0; path[n]; n++) {
    // Not a full array support, but works for Shelly 3EM emeters array!
    if (path[n] == '[') {
      variant = variant[JsonString(path, n)][atoi(&path[n+1])];
      path += n + 4;
      n = 0;
    }
    if (path[n] == '.') {
      variant = variant[JsonString(path, n)];
      path += n + 1;
      n = 0;
    }
  }
  return variant[path];
}
*/

/* fracment form old code..
void EMxSimulator::rpcWrapper() {
  JsonDocument jsonResponse;
  JsonDocument doc;
  deserializeJson(doc, serJsonResponse);
  jsonResponse["id"] = 1;
  jsonResponse["src"] = shelly_name;
  if (strcmp(rpcUser, "EMPTY") != 0) {
    jsonResponse["dst"] = rpcUser;
  }
  jsonResponse["result"] = doc;
  serializeJson(jsonResponse, serJsonResponse);
}
*/


// Marstek API
// https://eu.hamedata.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf


/*
code fragment:
==============


  // in UDP error, block execution
  if (UDP.begin(localPort) != 1) 
  {
    Serial.println("Connection failed");
    while (true) { delay(1000); } 
  }

  Serial.println("UDP successful");
}

void SendUDP_ACK()
{
  UDP.beginPacket(UDP.remoteIP(), remotePort);
  UDP.write("ACK");
  UDP.endPacket();
}

void SendUDP_Packet(String content)
{
  UDP.beginPacket(UDP.remoteIP(), remotePort);
  UDP.write(content.c_str());
  UDP.endPacket();
}

void GetUDP_Packet(bool sendACK = true)
{
  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    // read the packet into packetBufffer
    UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    Serial.println();
    Serial.print("Received packet of size ");
    Serial.print(packetSize);
    Serial.print(" from ");
    Serial.print(UDP.remoteIP());
    Serial.print(":");
    Serial.println(UDP.remotePort());
    Serial.print("Payload: ");
    Serial.write((uint8_t*)packetBuffer, (size_t)packetSize);
    Serial.println();
    // do something like:
    //ProcessPacket(String(packetBuffer));

    //// send a reply, to the IP address and port that sent us the packet we received
    if(sendACK) SendUDP_ACK();
  }
  delay(10);
}

*/



/*
GetDevice: OK (Mastek Venus V3 Firmware:122)
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
*/
void MarstekAPI::GetDevice() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "Marstek.GetDevice";
  jsonRequest["params"]["ble_mac"] = "0";

  serializeJson(jsonRequest, serJsonRequest);
  debug_println(serJsonRequest);

  _UdpRPC.UDPPRINT(serJsonRequest.c_str());
  delay(100);

}

/*
Bat.GetStatus --> : NO RESPONSE !!!  (Marstek V3 Firmware:122)
=============
--> request:

<-- response exampele:
{
	"id":	0,
	"src":	"VenusE 3.0-009b08a5dfa5",
	"result":	{
		"code":	-32700,
		"message":	"Parse error",
		"data":	403
	}
}
*/

/*
ES.GetMode  --> OK (Mastek Venus V3 Firmware:122)
==========
--> request:
{"id":0, "method":"ES.GetMode", "params":{"id":0}}


<-- response example:
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

*/
void MarstekAPI::ESGetMode() {
  JsonDocument jsonRequest;
  
  jsonRequest["id"] = 0;
  jsonRequest["method"] = "ES.GetMode";
  jsonRequest["params"]["id"] = 0;

  serializeJson(jsonRequest, serJsonRequest);
    Serial.print(" UDP send to:");
    Serial.print(_remoteIPaddr);
    Serial.print(":");
    Serial.println(_remotePort);
   debug_println(serJsonRequest);
  _UdpRPC.beginPacket(_remoteIPaddr, _remotePort);
  _UdpRPC.UDPPRINT(serJsonRequest.c_str());
  delay(100);
  _UdpRPC.endPacket();

}

#define UDP_TX_PACKET_SIZE 200
char packetBuffer[UDP_TX_PACKET_SIZE]; //buffer to hold incoming packet,
/// @brief Testroutine ohne functon
void MarstekAPI::GetUDP_Packet()
{
  int packetSize = _UdpRPC.parsePacket();
  if (packetSize)
  {
    Serial.println();
    Serial.print("Received packet of size ");
    Serial.print(packetSize);
    Serial.print(" from ");
    Serial.print(_UdpRPC.remoteIP());
    Serial.print(":");
    Serial.println(_UdpRPC.remotePort());
    // read the packet into packetBufffer
    _UdpRPC.read(packetBuffer, UDP_TX_PACKET_SIZE);

    Serial.print("Payload: ");
    Serial.write((uint8_t*)packetBuffer, (size_t)packetSize);
    Serial.println();
    // do something like:
    //ProcessPacket(String(packetBuffer));

  }
  delay(10);
}


/// @brief get Date from UDP
void MarstekAPI::getUDPData() 
{
  uint8_t buffer[1024];
  int packetSize = _UdpRPC.parsePacket();
  if (packetSize) 
  {
    JsonDocument jsonUDPIn;
    int rSize = _UdpRPC.read(buffer, 1024);
    buffer[rSize] = 0;
    debug_print("Received UDP packet:");
    debug_println((char *)buffer);
    deserializeJson(jsonUDPIn, buffer);
    // ES.Mode
    if (jsonUDPIn["result"].is<JsonVariant>()) 
    {
      JsonObject result = jsonUDPIn["result"];
      data.batSoc       = result["bat_soc"];
      data.onGridPower  = result["ongrid_power"];
      data.offGridPower = result["offgrid_power"];
    }
    else 
    {
        debug_printf("UDP: unknown request: %s\n", buffer);
    }
    
  }
}


// Public 
#define LOCAL_PORT 29000
bool MarstekAPI::init(String ip, uint16_t port)
{
   _UdpRPC.begin(LOCAL_PORT); 
   _remotePort = port;
   _remoteIPaddr.fromString(ip);
   debug_printf("init MarstekAPI IP:%s Port:%d\r\n",_remoteIPaddr.toString().c_str(), _remotePort);
  return true;
}

void MarstekAPI::loop()
{
	getUDPData();
}


