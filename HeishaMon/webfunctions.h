#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiGratuitous.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "dallas.h"
#include "settings.h"

static IPAddress apIP(192, 168, 4, 1);

int getFreeMemory(void);
String getUptime(void);
void setupWifi(SettingsStruct *heishamonSettings);
int getWifiQuality(void);
int getFreeMemory(void);

void handleRoot(ESP8266WebServer *httpServer, float readpercentage, int mqttReconnects, SettingsStruct *heishamonSettings);
void handleTableRefresh(ESP8266WebServer *httpServer, String actData[]);
void handleJsonOutput(ESP8266WebServer *httpServer, String actData[]);
void handleFactoryReset(ESP8266WebServer *httpServer);
void handleReboot(ESP8266WebServer *httpServer);
void handleDebug(ESP8266WebServer *httpServer, char *hex, byte hex_len);
void saveJsonToConfig(DynamicJsonDocument &jsonDoc);
void loadSettings(SettingsStruct *heishamonSettings);
bool handleSettings(ESP8266WebServer *httpServer, SettingsStruct *heishamonSettings);
void handleWifiScan(ESP8266WebServer *httpServer);
void handleSmartcontrol(ESP8266WebServer *httpServer, SettingsStruct *heishamonSettings, String actData[]);
void handleREST(ESP8266WebServer *httpServer, bool optionalPCB);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
