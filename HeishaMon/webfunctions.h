#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DoubleResetDetect.h>
#include <ArduinoJson.h>
#include "dallas.h"
#include "s0.h"
#include "gpio.h"
#include "smartcontrol.h"

static IPAddress apIP(192, 168, 4, 1);

struct settingsStruct {
  unsigned int waitTime = 5; // how often data is read from heatpump
  unsigned int waitDallasTime = 5; // how often temps are read from 1wire
  unsigned int updateAllTime = 300; // how often all data is resend to mqtt
  unsigned int updataAllDallasTime = 300; //how often all 1wire data is resent to mqtt

  const char* update_path = "/firmware";
  const char* update_username = "admin";
  char wifi_ssid[40] = "";
  char wifi_password[40] = "";
  char wifi_hostname[40] = "HeishaMon";
  char ota_password[40] = "heisha";
  char mqtt_server[40];
  char mqtt_port[6] = "1883";
  char mqtt_username[64];
  char mqtt_password[64];
  char mqtt_topic_base[40] = "panasonic_heat_pump";

  bool listenonly = false; //listen only so heishamon can be installed parallel to cz-taw1, set commands will not work though
  bool optionalPCB = false; //do we emulate an optional PCB?
  bool use_1wire = false; //1wire enabled?
  bool use_s0 = false; //s0 enabled?
  bool logMqtt = false; //log to mqtt from start
  bool logHexdump = false; //log hexdump from start
  bool logSerial1 = true; //log to serial1 (gpio2) from start

  s0SettingsStruct s0Settings[NUM_S0_COUNTERS];
  gpioSettingsStruct gpioSettings;
  SmartControlSettingsStruct SmartControlSettings;
};

String getUptime(void);
void setupWifi(DoubleResetDetect &drd, settingsStruct *heishamonSettings);
int getWifiQuality(void);
int getFreeMemory(void);

void handleRoot(ESP8266WebServer *httpServer, float readpercentage, int mqttReconnects, settingsStruct *heishamonSettings);
void handleTableRefresh(ESP8266WebServer *httpServer, String actData[]);
void handleJsonOutput(ESP8266WebServer *httpServer, String actData[]);
void handleFactoryReset(ESP8266WebServer *httpServer);
void handleReboot(ESP8266WebServer *httpServer);
void handleDebug(ESP8266WebServer *httpServer, char *hex, byte hex_len);
void handleSettings(DoubleResetDetect &drd, ESP8266WebServer *httpServer, settingsStruct *heishamonSettings);
void handleSmartcontrol(ESP8266WebServer *httpServer, settingsStruct *heishamonSettings, String actData[]);
void handleREST(ESP8266WebServer *httpServer, bool optionalPCB);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
