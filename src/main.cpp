#include <Arduino.h>
// Build in libs
#include <Esp.h>
#include <FS.h>
#include <Preferences.h>     
#include <base64.h>  

           
#ifdef ESP32
  
#ifdef ESP32_C3
  #include <HTTPClient.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
  #include <WiFi.h>
  #include <HTTPClient.h> 
  #include <Preferences.h>
#else
  #include "soc/soc.h"
  #include "soc/rtc_cntl_reg.h"  
  #include "esp_rom_gpio.h"
  #include "esp_pm.h"        // power-management
  #include "esp_check.h"     // ESP IDF error handling

  #include <HTTPClient.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
  #include <WiFi.h>
  #include <HTTPClient.h> 
  #include <Preferences.h>
#endif
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266mDNS.h>
#endif
                 
#include <ESPAsyncWebServer.h>         

#ifdef DISPLAY_OLED
#include <U8g2lib.h>
#endif


// my libs
#include "FileVarStore.h"
#include "AsyncWebLog.h"
#include "AsyncWebOTA.h"
#include "ESP32ntp.h"
#include "XPString.h"
#include "AsyncWebApp.h"

// mylibs for actual program
#include "SMLdecode.h"
#include "EMxSimulator.h"
#include "MarstekAPI.h"
#include "EcoTrackSimulator.h"

// special for S2 and S4
#if defined ESP_S2_MINI || ESP_S3_ZERO
#include "driver/temp_sensor.h"
#endif

// now set in platformio.ini
//#define DEBUG_PRINT 1   
#ifdef DEBUG_PRINT
#pragma message("Info : DEBUG_PRINT=1")
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

#ifdef M5_COREINK
#include "m5coreink.h"
#endif

#ifdef ESP32_DEVKIT1
#pragma message("Info : ESP32_DEVKIT")
#define LED_GPIO 2
#endif

#ifdef ESP32_S2_MINI
#pragma message("Info : ESP32_S2_MINI")
#define LED_GPIO LED_BUILTIN    
#endif

#ifdef ESP32_S3_ZERO
 #pragma message("Info : ESP32_S3_ZERO")
 #define NEOPIXEL 21
#endif




#ifdef ESP32_S3_DEVKIT
 #pragma message("Info : ESP32_S3_DEVKIT")
 //#define NEOPIXEL 48
#endif


#ifdef ESP32_C3_SUPERMINI
 #pragma message("Info : ESP32_S3_DEVKIT")
 #define LED_GPIO 8
#endif

const char* SYS_Version = "V 1.0.0";
const char* SYS_CompileTime =  __DATE__;
static String  SYS_IP = "0.0.0.0";

#ifdef WEB_APP
// internal Webserver                                
AsyncWebServer webserver(80);
#endif

#if (defined SML_TIBBER) || (defined SML_TASMOTA)
SMLdecode smldecoder;
#endif

#if (defined EM1_UDP_SIMULATION) 
EMxSimulator shellyEMx;
#endif

#ifdef ECOTRACK_SIMULATION
EcoTrackSimulator ecotrak;
#endif

#ifdef MARSTEK_API
MarstekAPI marstek;
#endif


// fetch String;
String sFetch;
uint timeout_count = 0;

// ntp client
const char* TimeServerLocal = "192.168.2.1";
const char* TimeServer      = "europe.pool.ntp.org";
const char* TimeZone        = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
ESP32ntp ntpclient;
bool isSTA_MODE = true;  // false = AP-Mode


const unsigned long   TimerFastDuration = 200;
unsigned long   TimerFast = 0;
const unsigned long   TimerSlowDuration   = 1000;   
unsigned long   TimerSlow = 0;     

#if defined ESP32_S3_ZERO || defined ESP32_S3_DEVKIT
static char neopixel_color = 'w';
#define  setcolor(...) neopixel_color = __VA_ARGS__
#else
#define setcolor(...)
#endif

/// @brief  set builtin LED
/// @param i = HIGH / LOW
void setLED(uint8_t i)
{
#if defined ESP32_S3_ZERO || defined ESP32_S3_DEVKIT
if (i==0)
 {
    rgbLedWrite(NEOPIXEL,0,0,0);
    //neopixelWrite(NEOPIXEL,0,0,0); // off
 }
 else
 {
    switch (neopixel_color)
    {
    case 'r': 
      rgbLedWrite(NEOPIXEL,2,0,0);
      //neopixelWrite(NEOPIXEL,2,0,0); // red
      break;
    case 'g':
      rgbLedWrite(NEOPIXEL,0,2,0);
      //neopixelWrite(NEOPIXEL,0,2,0); // green
      break;
    case 'b':
      rgbLedWrite(NEOPIXEL,0,0,2);
      //neopixelWrite(NEOPIXEL,0,0,2); // blue
      break;
    case 'y':
      rgbLedWrite(NEOPIXEL,1,1,0);
       //neopixelWrite(NEOPIXEL,1,1,0); // yellow
      break;
    case 'w':
      rgbLedWrite(NEOPIXEL,1,1,1);
      //neopixelWrite(NEOPIXEL,1,1,1); // white
      break;
    default:
       break;
    }
 }
#else
 digitalWrite(LED_GPIO, i);
#endif
}


uint8_t blnk=0;
/// @brief  blink LED
static void blinkLED()
{
  blnk = !blnk;
  setLED(blnk);
}

////////////////////////////////////////////
/// @brief init builtin LED
////////////////////////////////////////////
inline void initLED()
{
 #if defined ESP32_S3_ZERO || defined ESP32_S3_DEVKIT
  neopixel_color='w'; 
#else
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, HIGH);
#endif
  setcolor('w'); //white
  setLED(1);
}


void inline initSPIFFS()
{
  debug_println("*** SPIFFS.begin");
  delay(300);
  if (!myFS.begin())
  {
   debug_println("*** ERROR: SPIFFS Mount failed");
  } 
  else
  {
   debug_println("* INFO: SPIFFS Mount succesfull");
  }
}
 

//////////////////////////////////////////////////////
/// @brief  expand Class "FileVarStore" with variables
//////////////////////////////////////////////////////
class VarStore final: public FileVarStore 
{
 public:  
  // Device-Parameter
   String varDEVICE_s_name  = "Tibber2Marstek";
  // Wifi-Parameter
   String varWIFI_s_mode    = "AP"; // STA=client connect with Router,  AP=Access-Point-Mode (needs no router)
   String varWIFI_s_password= "";
   String varWIFI_s_ssid    = "tibber2marstek";
#if defined SML_TASMOTA || defined SML_TIBBER
   String varSML_s_url      = "";
   String varSML_s_user     = "";
   String varSML_s_password = "";
#endif
#if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION) 
   uint16_t varEMX_i_port;
#endif
#if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION)  || (defined ECOTRACK_SIMULATION)
   float    varEMX_f_filterfactor; 
#endif
#ifdef MARSTEK_API
    int varMARSTEKAPI_i_port;
    String varMARSTEKAPI_s_url;
    uint varMARSTEKAPI_i_pollsec;
#endif


 protected:
   void GetVariables() 
   {
     varDEVICE_s_name      = GetVarString(GETVARNAME(varDEVICE_s_name));
     varWIFI_s_mode        = GetVarString(GETVARNAME(varWIFI_s_mode)); //STA or AP
     varWIFI_s_password    = GetVarString(GETVARNAME(varWIFI_s_password));
     varWIFI_s_ssid        = GetVarString(GETVARNAME(varWIFI_s_ssid));
#if defined SML_TASMOTA ||  defined SML_TIBBER
     varSML_s_url         = GetVarString(GETVARNAME(varSML_s_url));
     varSML_s_password     = GetVarString(GETVARNAME(varSML_s_password));
     varSML_s_user         = GetVarString(GETVARNAME(varSML_s_user));
#if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION)
     varEMX_i_port         = (uint16_t)GetVarInt(GETVARNAME(varEMX_i_port),2223);
     varEMX_f_filterfactor = GetVarFloat(GETVARNAME(varEMX_f_filterfactor), 0.8); 
     shellyEMx.setFilterFactor(varEMX_f_filterfactor);
#endif
#endif

#ifdef MARSTEK_API
     varMARSTEKAPI_s_url     = GetVarString(GETVARNAME(varMARSTEKAPI_s_url), "192.168.2.95");
     varMARSTEKAPI_i_port    = GetVarInt(GETVARNAME(varMARSTEKAPI_i_port),30000);
     varMARSTEKAPI_i_pollsec = GetVarInt(GETVARNAME(varMARSTEKAPI_i_pollsec),20);
     marstek.setPollRateSec(varMARSTEKAPI_i_pollsec);
#endif
   }
};

VarStore varStore;

#ifdef DISPLAY_OLED
/*
  U8glib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.

*/
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* reset=*/ 16);

void drawDisplay_init(const char* ssid, const char* ip_adr)
{

  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R2);
  //u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_logisoso16_tr);  // choose a suitable font
  u8g2.setFont(u8g2_font_8x13_tr);  // choose a suitable font
  u8g2.firstPage();
  //int i = 1;
  do
  {
    u8g2.drawStr(0,16,ssid);
    u8g2.drawStr(0,32, ip_adr);
  }while ( u8g2.nextPage() );
  delay(1000);
  u8g2.clearBuffer();
}


//uint16 page = 0;
void drawDisplay_loop()
{
   //if (page ==0)
    {
     //page = 1;
     u8g2.firstPage();
     do
     {
#if (defined SML_TIBBER)
      // 1. Zeile
      //u8g2.setFont(u8g2_font_logisoso16_tr);
      u8g2.setFont(u8g2_font_8x13_tr);
      u8g2.setCursor(0,16);
      u8g2.printf("%4dW", smldecoder.getWatt());
#endif
#if (defined MARSTEK_API)
      u8g2.setCursor(60,16);
      u8g2.printf("%2d%%",marstek.getSOC());
     
#endif
      // 2. Zeile 
      u8g2.setFont(u8g2_font_8x13_tr);
      u8g2.setCursor(0,32);
      //u8g2.printf(ntpclient.getTimeString());
      //u8g2.drawStr(40,32,":");
      //u8g2.setCursor(50,32);
      //u8g2.printf("%02d",ntpclient.getTimeInfo()->tm_sec);

       u8g2.printf("%4dW", marstek.getOnGridPower());
     } while( u8g2.nextPage() );
    }
}

#endif

void initFileVarStore()
{
  varStore.Load();
}       


/// @brief  Init WiFi
/// @return  false = AP-Mode, true= STA-Mode
bool initWiFi()
{
#ifdef MINI_32
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0); // brownout problems
#endif
   if (varStore.varWIFI_s_mode == "AP")
   {
    delay(100);
    Serial.println("INFO-WIFI:AP-Mode");
    WiFi.softAP(varStore.varDEVICE_s_name.c_str());   
    Serial.print("IP Address: ");
    SYS_IP = WiFi.softAPIP().toString();
    Serial.println(SYS_IP);
    return false;
   }
   else
   {
    debug_printf("INFO-WIFI:STA-Mode\r\n");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(varStore.varDEVICE_s_name.c_str());
    WiFi.begin(varStore.varWIFI_s_ssid.c_str(), varStore.varWIFI_s_password.c_str());
    #if defined ESP32_S3_ZERO || defined MINI_32 || defined M5_COREINK
    WiFi.setTxPower(WIFI_POWER_5dBm);// brownout problems with some boards or low battery load for M5_COREINK
    //WiFi.setTxPower(WIFI_POWER_15dBm);// Test 15dB
    #endif
    #if defined DEBUG_PRINT && (defined ESP32_RELAY_X4 || defined ESP32_RELAY_X2)
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); // decrease power over serial TTY-Adapter
    #endif
    int i = 0;
    debug_printf("SSID:%s connecting\r\n", varStore.varWIFI_s_ssid.c_str());
    ///debug_printf("Passwort:%s\r\n", varStore.varWIFI_s_Password);
    while (!WiFi.isConnected())
    {
        debug_print(".");
        blinkLED();
        i++;  
        delay(200);
        if (i > 40)
        {
          ESP.restart();
        }
    }    
    SYS_IP = WiFi.localIP().toString();
    debug_println("CONNECTED!");
    #ifdef ESP32
    debug_printf("WiFi-Power:%d\r\n",WiFi.getTxPower())
    #endif
    debug_printf("WiFi-RSSI:%d\r\n",WiFi.RSSI());
    debug_print("IP Address: ");
    debug_println(SYS_IP);
    if (!ntpclient.begin(TimeZone, TimeServerLocal, TimeServer))
    {
     ESP.restart();
    }
   }
  return true;
}     

void testWiFiReconnect()
{
  // Test if wifi is lost from router
  if ((varStore.varWIFI_s_mode == "STA") && (WiFi.status() != WL_CONNECTED))
    {
     debug_println("Reconnecting to WiFi...");
     if (!WiFi.reconnect())
     {
      delay(500);
      ESP.restart();
     } 
   }
}


/*
void initEcoTrackerSim()
{
  // Set up mDNS responder
  String baseName = "ecoctracker-" + WiFi.macAddress();
  if (!MDNS.begin(baseName.c_str()))
  {
    debug_println("Error setting up MDNS responder!");
  }

#ifdef ESP32
  MDNS.addService("_http", "_tcp", 80);
  MDNS.addService("_everhome","_tcp",80);
  //wird nicht gebraucht !
  //mdns_txt_item_t serviceTxtData[4] = 
  //{
  //  { "fw_id", SYS_Version},
  //  { "arch", "esp8266" },
  //  { "id", baseName.c_str() }
  //};
  
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
  debug_println("mDNS responder started");
}
*/

#ifdef WEB_APP
// -------------------- WEBSERVER -------------------------------------------
// --------------------------------------------------------------------------

/// @brief replace placeholder "%<variable>%" in HTML-Code
/// @param var 
/// @return String
String setHtmlVar(const String& var)
{
  debug_print("func:setHtmlVar: ");
  debug_println(var);
  sFetch = "";
 
  if (var == "CONFIG") // read config.txt
  {
    return varStore.GetBuffer();;
  } 

  else
  if (var== "DEVICEID")
  {
    return varStore.varDEVICE_s_name;
  }

  else
  if (var == "INFO")
  {
     sFetch =    "Version    :";
     sFetch += SYS_Version;  
  #ifdef ESP32
     sFetch += "\nPlatform   :";
     sFetch +=  ESP.getChipModel();
  #else
     sFetch += "\nPlatform   :ESP8266";
  #endif
     sFetch += "\nBuild-Date :";
     sFetch +=  F(__DATE__);
     sFetch += "\nIP-Addr    :";
     sFetch += SYS_IP;
     sFetch += "\nRSSI       :";
     sFetch += WiFi.RSSI();
  #ifdef ESP32
     sFetch += "\nTemp.      :";
     sFetch += String(temperatureRead());   
   
     sFetch += "\nMinFreeHeap:";
     sFetch += String(ESP.getMinFreeHeap());
     sFetch += "\nFree Sketch:";
     sFetch += String(ESP.getFreeSketchSpace());
     sFetch += "\nAppFreq Mhz:";
     sFetch += String(getApbFrequency()/ 1000000);

     sFetch += "\nTimeOutCount:";
     sFetch += timeout_count;;
  #else
     sFetch += "\nFreeHeap   :";
     sFetch += String(ESP.getFreeHeap());
  #endif
     return sFetch;
  }
  else
  {
    return "";
  }
 
}


void notFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "Not found");
}

void initWebServer()
{ 
 

#ifdef ECOTRACK_SIMULATION
  
   //Route json request
  webserver.on("/v1/json",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String s = ecotrak.buildJsonRequest();
    request->send(200, "application/json", s);
   
  });
#endif

  //Route for root / web page
  webserver.on("/",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (isSTA_MODE) 
    { 
      request->send(myFS, "/index.html", String(), false, setHtmlVar);
    }
    else
    { // "rescue page" for first run without valid Wifi-credentials or no valid data-system
      request->redirect("/ota_ap.html");
      //request->send(202, "text/plain", "goto: http://192.168.4.1/ota_ap.html");
    }
  });


  //Route for root /index web page
  webserver.on("/index.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(myFS, "/index.html", String(), false, setHtmlVar);
  });
 
  //Route for config web page
  webserver.on("/config.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(myFS, "/config.html", String(), false, setHtmlVar);
  });
  //Route for Info-page
  webserver.on("/info.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   String s = (String)
   varStore.SetVarString("varLogCount_s_val", ntpclient.getTimeString());
   request->send(myFS, "/info.html", String(), false, setHtmlVar);
  });

  //Route for Chart
  webserver.on("/gridchart.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(myFS, "/gridchart.html", String(), false, setHtmlVar);
  });

  // chart Icon
  webserver.on("/chart.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(myFS, "/chart.png", String(), false);
  });


  sFetch.reserve(150);
  webserver.on("/fetch", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (request->args() > 0)
    {
    debug_println("GET-Argument: " + request->argName(0));
    debug_print("Value: ");
    //uint8_t i = 0;
    //String s  = request->arg(i);
    //debug_println(s);
    }
                                                                 // Index:
    sFetch = ntpclient.getTimeString();            // 0 = Time: 00:00
  #if (defined SML_TIBBER)
    sFetch += ',';
    sFetch += smldecoder.getWatt();                // 1 = Grid Power [W]
    sFetch += ',';
    sFetch += smldecoder.getInputkWh();            // 2 = Grid Energy-In [kWh]
    sFetch += ',';
    sFetch += smldecoder.getOutputkWh();           // 3 = Grid Energy-Out[kWh]
  #endif
   sFetch += ',';
  #if defined EM1_UDP_SIMULATION || defined ECOTRACK_SIMULATION
   uint tmp_timeout = 0;
  #ifdef EM1_UDP_SIMULATION
   if ((uint)shellyEMx.getRequestTimeout() > 0)
   {tmp_timeout = 1;}
  #endif
  #ifdef ECOTRACK_SIMULATION
   if ((uint)ecotrak.getRequestTimeout() > 0 && tmp_timeout > 0)
   {tmp_timeout++;}
  #endif
    sFetch += tmp_timeout;
  #else
    sFetch += "0";
  #endif
  #ifdef MARSTEK_API
    sFetch += ',';
    sFetch += marstek.getSOC();                    // 5 = MARSTEK Bat SOC [%]
    sFetch += ',';
    sFetch += marstek.getOnGridPower();            // 6 = MARSTEK Bat Power +/- [W]
  #endif
    sFetch += ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,end";     // for more values  
  
    request->send(200, "text/plain", sFetch);
    //debug_println("server.on /fetch: "+ s);
  });

  // -------------------- POST --------------------------------------------------
  // config.html POST
  webserver.on("/config.html",          HTTP_POST, [](AsyncWebServerRequest *request)
  {
   //debug_println("Argument: " + request->argName(0));
   //debug_println("Value: ");
   uint8_t i = 0;
   String s  = request->arg(i);
   debug_println(request->arg(i));
   if (request->argName(0) == "saveconfig")
   {
       varStore.Save(s);
       //varStore.Load();
   }
   //debug_println("Request /index3.html");
   request->send(myFS, "/config.html", String(), false, setHtmlVar);
  });
  
   // init Webserver for libs
  AsyncWebLog.begin(&webserver);
  AsyncWebOTA.begin(&webserver);
  AsyncWebApp.begin(&webserver);

  webserver.onNotFound(notFound);
  webserver.begin();
}
#endif


////////////////////////////////////////////////////
/// @brief setup
////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(115200);                                     
  initLED();
  delay(500);
  Serial.println("***START***");    
  //Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram()); 
  delay(1000);
  initSPIFFS();
  initFileVarStore(); 
  setcolor('b'); // blue

  isSTA_MODE = initWiFi();
  initWebServer(); 

  #ifdef DISPLAY_OLED
    drawDisplay_init(varStore.varWIFI_s_ssid.c_str(), SYS_IP.c_str());
#endif

  setcolor('g'); // green
  setLED(1);
  delay(1000);
  if (!isSTA_MODE)
  {
    return;
  }

#if (defined SML_TIBBER)
  smldecoder.init(varStore.varSML_s_url.c_str(), varStore.varSML_s_user.c_str(), varStore.varSML_s_password.c_str());
#endif
#if (defined EM1_UDP_SIMULATION) 
   shellyEMx.init(varStore.varEMX_i_port, varStore.varEMX_f_filterfactor);
#endif
#ifdef MARSTEK_API
   marstek.init(varStore.varMARSTEKAPI_s_url, varStore.varMARSTEKAPI_i_port,varStore.varMARSTEKAPI_i_pollsec);
#endif

#ifdef ECOTRACK_SIMULATION
   ecotrak.begin(varStore.varEMX_f_filterfactor);
#endif
}




////////////////////////////////////////////////
/// @brief loop
////////////////////////////////////////////////
void loop() 
{

  
   if (millis() - TimerSlowDuration > TimerSlow) 
   {

    TimerSlow = millis();                      // Reset time for next event
    testWiFiReconnect();

    ntpclient.update();   
    //debug_printf("Time: %s\r\n", ntpclient.getTimeString());

#ifdef DISPLAY_OLED
    drawDisplay_loop();
#endif
    
   if(!isSTA_MODE)
   {
    delay(1);
    return;
   }

#if defined SML_TASMOTA || defined SML_TIBBER
    smldecoder.read();
#ifdef WEB_APP
    AsyncWebLog.printf("[SML]W:%d In:%.1f Out:%.1f\r\n", smldecoder.getWatt(), smldecoder.getInputkWh(), smldecoder.getOutputkWh());
#endif
#if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION)
    shellyEMx.setData_AllPhase(smldecoder.getWatt(),smldecoder.getInputkWh(),smldecoder.getOutputkWh());
#endif
#ifdef ECOTRACK_SIMULATION
    ecotrak.setData_AllPhase(smldecoder.getWatt(),smldecoder.getInputkWh(),smldecoder.getOutputkWh());
#endif

#endif 

  }//TimerSlowDuration

  // TimerFastDuration
  if (millis() - TimerFastDuration > TimerFast)
  {
    TimerFast = millis();
#if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION)
    if (shellyEMx.getRequestTimeout() && ecotrak.getRequestTimeout())
    {
      setLED(1);
      timeout_count++;
      delay (500);
      //ESP.restart();
    }
    else
#endif
    {
      blinkLED();
    }
 
  } // TimerFastDuration


 #if (defined EM1_UDP_SIMULATION) || (defined EM3_UDP_SIMULATION)
    shellyEMx.loop();
 #endif
  #ifdef MARSTEK_API 
    marstek.loop();
#endif
delay(1);
}

