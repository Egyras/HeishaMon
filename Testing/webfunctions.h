#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DoubleResetDetect.h> 
#include <ArduinoJson.h>


void setupWifi(DoubleResetDetect &drd, char* wifi_hostname, char* ota_password, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password);
void handleRoot(ESP8266WebServer *httpServer, DynamicJsonDocument *actData);
void handleFactoryReset(ESP8266WebServer *httpServer);
