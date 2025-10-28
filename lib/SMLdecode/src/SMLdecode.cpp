#include "SMLdecode.h"

//----------------------------------------------------public Functions----------------------------------------------------

bool SMLdecode::init(const char* url, const char* user, const char* pass)
{
  _url.reserve(20);
  _passw.reserve(10);
  _usern.reserve(10);
  _url = url; _usern = user; _passw = pass;

  debug_println("SMLdecode-init OK!");
  return true;
}


#ifdef SML_TASMOTA
// special version to get the values from the main energy-meter 
// from tasmota device to read the optical SML-Signal fom the main energy-meter
String SMLdecode::httpGETRequest(const char* http_url) 
{
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(_wificlient, http_url);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = ""; 
  
  if (httpResponseCode>0) {
    //debug_print("HTTP Response code: ");
    //debug_println(httpResponseCode);
    payload = http.getString();
    //debug_printf("httpGetRequest: %s\r\n",payload);
  }
  else {
    debug_print("Error code: ");
    debug_println(httpResponseCode);
  }
  // Free resources
  http.end();

  payload.trim();
  return payload;
}

/// @brief my implementation for reading a TASMOTA device...adapt to your needs
void getSmlMeter()
{
  // by JG : todo: replace hardcoded url with config-value
  String s = httpGETRequest("http://192.168.2.88/cm?cmnd=status%2010");
  /* Format of Tasmota SML
  {
   "StatusSNS": {
    "Time": "2024-04-29T18:28:19",
    "SML": {
      "Total_in": 5953.3436,
      "Total_out": 8620.3801,
      "Power_curr": -27
    }
   }
  }
 */
 JsonDocument doc;
 deserializeJson(doc, s);
 valSML_kwh_in  = doc["StatusSNS"]["SML"]["Total_in"];
 valSML_kwh_out = doc["StatusSNS"]["SML"]["Total_out"];
 valSML_watt   = doc["StatusSNS"]["SML"]["Power_curr"];
 String sml = "SML-Meter-Watt: ";
 sml += String(valSML_watt);
 //AsyncWebLog.println(sml);
}
#endif

#define SML_TIBBER 1

#ifdef SML_TIBBER
/* private functions */

// my special version to get the values from the tibber-host 
// ...quick and dirty ;-) only testet for Meter: ISKRA MT631 !!!
// write your own routine if you need this values !!!
byte * SMLdecode::httpGETRequest(const char* http_url) 
{
  HTTPClient http;

  // for TEST !!!
  // _usern = "admin"; 
  // _passw = "xxxx-xxxx";

  int getlength = 0;
  http.setTimeout(700); // in msec
  http.begin(_wificlient, http_url);
  http.setAuthorization(_usern.c_str(), _passw.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) 
  {
    
    getlength = http.getSize();

    //debug_printf("[SML] datasize: %d\r\n", getlength);
    if ((getlength > SMLPAYLOADMAXSIZE) || (getlength <=0))
    {
      http.end();
      return NULL;
    }

    WiFiClient * w = http.getStreamPtr();
    w->readBytes(smlpayload, getlength);

    if (getlength < 200) 
    {
     debug_printf("smlpayload length ?:%d Data: ", getlength);
     for (size_t i = 0; i < 10; i++)
     {
       debug_printf("%02x ",smlpayload[i]);
     }
     debug_println("...");
     getlength = 0;
    }  
  }
  else 
  {
    debug_print("TIBBER httpGETRequest Error code: ");
    debug_println(httpResponseCode);
  }
  // Free resources
  http.end();

  if (getlength <= 0)
  {
    return NULL;
  }

  return smlpayload;
}

/// @brief decode raw SML-Message
/// @param retval   reference to returend value !allway in uint23 ... so cast for xour need !!
/// @param payload  raw SML
/// @param smlcode SML-Sequence-ID to find
/// @param smlsize size of payload
/// @param offset  offset from end of SML-Sequence-ID to length byte of value
/// @return 
bool SMLdecode::decodePayload(uint32_t &retval, byte* payload, byte* smlcode, uint smlsize,  uint offset) 
{
  byte *loc = (byte*)memmem(payload, SMLPAYLOADMAXSIZE, smlcode, smlsize);
  if (loc == NULL) {
     debug_printf("ERROR decodeSMLval ID:'%x.%x.%x' not found\r\n", smlcode[4], smlcode[5], smlcode[6]);
    return false;
  }

  /* Test fake "-1"
  if (nlen == 2){
    loc[offset]  = 0xff;
    loc[offset+1] = 0xff;
  }
  */
  // 'nlen' aus SML lesen (aendert sich bei Leistung je nach Größe des Wertes !!!)
  uint8_t nlen = (loc[offset-1] & 0x0F) -1;

  /* for extra debugging
  debug_printf("nlen=%d\r\n", nlen);
  debug_print("HEX: ");
  for (size_t i = 0; i < offset+nlen+1; i++)
  {
    debug_printf("%02x ", loc[i]);
  }
  debug_println();
  */
  
  if ((nlen < 1) ||(nlen > 8))
  {
    debug_printf("decodeSMLPayload: unvalid length &d\r\n", nlen);
    return false;
  }
  
  byte* pval = loc + offset;
  uint32_t value=0;
  if (nlen == 1){
   value = (int8_t) *(pval);
  }
  else {
   nlen=nlen+1; 
   while (--nlen) {
      //value<<=8;
      value = value << 8;
      value|=*(pval)++;
   }
  }
  retval = value;
  return true;
}

/*
optional:
/// get status Data from Tibber-Pulse
// http://<ip-address>/nodes.json";
 
example Data:
[
  {
    "node_id": 1,
    "product_id": 49344,
    "product_model": "TFD01",
    "eui": "b43522fffe387a02",
    "version": "1201-f4b8d10b",
    "average_rssi": -65,
    "rssi": -67,
    "average_lqi": 245,
    "lqi": 255,
    "last_seen_ms": 290,
    "last_data_ms": 290,
    "available": true,
    "ota_upload_state": "idle",
    "ota_distribute_status": "up2date",
    "paired": true,
    "advertise_sent": false,
    "model": "tibber-pulse-ir-node-efr32",
    "ota_tag": 123,
    "manifest_version": "1201-f4b8d10b"
  }
]

char*  SMLdecode::readStatus()
{
   bool ret = true;
  String sURL = "http://";
  sURL += _url;
  sURL += "/nodes.json";

   HTTPClient http;

  // for TEST !!!
  // _usern = "admin"; 
  // _passw = "xxxx-xxxx";

  int getlength = 0;
#ifdef ESP32
  http.setConnectTimeout(500); 
#else
  http.setTimeout(500);
#endif
  http.begin(_wificlient, sURL);
  http.setAuthorization(_usern.c_str(), _passw.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) 
  {
    getlength = http.getSize();
    if ((getlength > SMLPAYLOADMAXSIZE) || (getlength <=0))
    {
      http.end();
      return NULL;
    }
    WiFiClient * w = http.getStreamPtr();
    w->readBytes(smlpayload, getlength);
  }
  else 
  {
    debug_print("TIBBER httpGETRequest Error code: ");
    debug_println(httpResponseCode);
  }

  char * payload = (char*) httpGETRequest(sURL.c_str());
  
  if (payload == NULL)
  {
    return NULL;
  }
  return payload;
}
*/


/* public */
/// @brief my implementation for reading a Tibber device...adapt to your needs
bool SMLdecode::read()
{
  // http://192.168.2.87/data.json?node_id=1  ( for test with webbrowser)
  // varStore.varSML_s_url="192.168.2.87";
  bool ret = true;
  String sURL = "http://";
  sURL += _url;
  sURL += "/data.json?node_id=1";
  byte * payload = httpGETRequest(sURL.c_str());
  
  if (payload == NULL)
  {
    return false;
  }

  // -------------new --------------------------
  byte sml_1_8_0[] {0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xff};   // Energy IN  (1.8.0)
  byte sml_2_8_0[] {0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x00, 0xff};   // Energy OUT (2.8.0)
  byte sml_1_7_0[] {0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xff};    // Power Sum  (1.7.0) in= pos. out= neg)
  uint32_t inputenergy  = 0;
  uint32_t outputenergy = 0;
  if (!decodePayload(inputenergy,smlpayload, sml_1_8_0, sizeof(sml_1_8_0), 19))
  {
    ret = false;
  }
  if (!decodePayload(outputenergy, smlpayload, sml_2_8_0, sizeof(sml_2_8_0), 15)) 
  {
    ret = false;
  }

  if (ret == true)
  {
    _inputkWh  = double(inputenergy  / 10000.0);
    _outputkWh = double(outputenergy / 10000.0);
    //setEnergyData(double(inputenergy/ 10000.0), double(outputenergy / 10000.0));
  }
     
      // for Power:
      // length is variable !!! (2 byte= for big values)
      //                                           53=16bit int
      // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 91 01 145W
      // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 95 01 149W
      // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 8d 01 141W
      // length is variable !!! (1 byte for small values)
      //                                           52=8bit int 
      // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 76 01 01 
      // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 66 01 01

  
  uint32_t watt = 0;
  if (decodePayload(watt, smlpayload, sml_1_7_0, sizeof(sml_1_7_0), 15)) 
  {
    _watt = int16_t(watt);
    //setPowerData(int16_t(watt) / 1000.0);
  }
  else {
    debug_println ("SML ERROR Parsing sml_1_7_0");
    ret = false;
  }
  // extra debug info
  if (ret == true) {
    debug_printf("[SML] Power:%d  1.8.0:%05.3f  2.8.0:%05.3f\r\n", _watt, _inputkWh, _outputkWh);
  }
  // ----------- end new -----------------------

  /* ------------ old -------------------------

  // Energy IN (1.8.0)

  byte sml_1_8_0[] {0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xff};
  _inputkWh = double(decodeSMLval(payload, sml_1_8_0, sizeof(sml_1_8_0), 19) / 10000.0);
 
  // Energy OUT (2.8.0)
  byte sml_2_8_0[] {0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x00, 0xff};
  _outputkWh = double(decodeSMLval(payload, sml_2_8_0, sizeof(sml_2_8_0), 15) / 10000.0);
 
  // for Power:
  // length is variable !!! (2 byte= for big values)
  //                                           53=16bit int
  // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 91 01 145W
  // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 95 01 149W
  // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 8d 01 141W
  // length is variable !!! (1 byte for small values)
  //                                           52=8bit int 
  // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 76 01 01 
  // 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 66 01 01

  // Power (in= pos. out= neg)
  byte sml_16_7_0[] {0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xff};
  _watt = int16_t(decodeSMLval(payload, sml_16_7_0, sizeof(sml_16_7_0), 15));

  debug_printf("[SML] Power:%d  1.8.0:%05.3f  2.8.0:%05.3f\r\n", _watt, _inputkWh, _outputkWh);
  /// ------------ old -------------------------
  */


  return ret;
}
#endif



