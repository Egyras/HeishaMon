#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DoubleResetDetect.h> 
#include <ArduinoJson.h>
#include "featureboard.h"

struct settingsStruct {
  unsigned int waitTime; // how often data is read from heatpump
  unsigned int updateAllTime; // how often all data is resend to mqtt
  unsigned int updataAllDallasTime; //how often all 1wire data is resent to mqtt
  
  const char* update_path = "/firmware";
  const char* update_username = "admin";
  char wifi_hostname[40] = "HeishaMon";
  char ota_password[40] = "heisha";
  char mqtt_server[40];
  char mqtt_port[6] = "1883";
  char mqtt_username[40];
  char mqtt_password[40];
  char mqtt_topic_base[40] = "panasonic_heat_pump";

  bool listenonly = false; //listen only so heishamon can be installed parallel to cz-taw1, set commands will not work though
  bool use_1wire = false; //1wire enabled?
  bool use_s0 = false; //s0 enabled?
};


String getUptime(void);
void setupWifi(DoubleResetDetect &drd, settingsStruct *heishamonSettings, s0Data actS0Data[]);
void handleRoot(ESP8266WebServer *httpServer, float readpercentage, settingsStruct *heishamonSettings);
void handleTableRefresh(ESP8266WebServer *httpServer, String actData[], dallasData actDallasData[], s0Data actS0Data[]);
void handleJsonOutput(ESP8266WebServer *httpServer, String actData[], dallasData actDallasData[], s0Data actS0Data[]);
void handleFactoryReset(ESP8266WebServer *httpServer);
void handleReboot(ESP8266WebServer *httpServer);
void handleSettings(ESP8266WebServer *httpServer, settingsStruct *heishamonSettings, s0Data actS0Data[]);
