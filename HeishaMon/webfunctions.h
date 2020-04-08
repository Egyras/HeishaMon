#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DoubleResetDetect.h> 
#include <ArduinoJson.h>
#include "featureboard.h"


String getUptime(void);
void setupWifi(DoubleResetDetect &drd, char* wifi_hostname, char* ota_password, char* mqtt_topic_base, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password, bool &use_1wire, bool &use_s0, bool &listenonly, s0Data actS0Data[]);
void handleRoot(ESP8266WebServer *httpServer, float readpercentage, bool &use_1wire, bool &use_s0);
void handleTableRefresh(ESP8266WebServer *httpServer, String actData[], dallasData actDallasData[], s0Data actS0Data[]);
void handleJsonOutput(ESP8266WebServer *httpServer, String actData[], dallasData actDallasData[], s0Data actS0Data[]);
void handleFactoryReset(ESP8266WebServer *httpServer);
void handleReboot(ESP8266WebServer *httpServer);
void handleSettings(ESP8266WebServer *httpServer, char* wifi_hostname, char* ota_password, char* mqtt_topic_base, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password, bool &use_1wire, bool &use_s0, bool &listenonly, s0Data actS0Data[]);
