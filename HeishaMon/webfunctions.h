#define LWIP_INTERNAL

#include <ESP8266WiFi.h>
#include <ESP8266WiFiGratuitous.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "src/common/webserver.h"
#include "dallas.h"
#include "s0.h"
#include "gpio.h"

void log_message(char* string);

static IPAddress apIP(192, 168, 4, 1);

struct settingsStruct {
  unsigned int waitTime = 5; // how often data is read from heatpump
  unsigned int waitDallasTime = 5; // how often temps are read from 1wire
  unsigned int dallasResolution = 12; // dallas temp resolution (9 to 12)
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
};

void setupConditionals();
int getFreeMemory(void);
char *getUptime(void);
void setupWifi(settingsStruct *heishamonSettings);
int getWifiQuality(void);
int getFreeMemory(void);

void log_message(char *string);
int8_t webserver_cb(struct webserver_t *client, void *data);
void getWifiScanResults(int numSsid);
int handleRoot(struct webserver_t *client, float readpercentage, int mqttReconnects, settingsStruct *heishamonSettings);
int handleTableRefresh(struct webserver_t *client, String actData[]);
int handleJsonOutput(struct webserver_t *client, String actData[]);
int handleFactoryReset(struct webserver_t *client);
int handleReboot(struct webserver_t *client);
int handleDebug(struct webserver_t *client, char *hex, byte hex_len);
void settingsToJson(DynamicJsonDocument &jsonDoc, settingsStruct *heishamonSettings);
void saveJsonToConfig(DynamicJsonDocument &jsonDoc);
void loadSettings(settingsStruct *heishamonSettings);
int getSettings(struct webserver_t *client, settingsStruct *heishamonSettings);
int handleSettings(struct webserver_t *client);
int saveSettings(struct webserver_t *client, settingsStruct *heishamonSettings);
int settingsReconnectWifi(struct webserver_t *client, settingsStruct *heishamonSettings);
int settingsNewPassword(struct webserver_t *client, settingsStruct *heishamonSettings);
int cacheSettings(struct webserver_t *client, struct arguments_t * args);
int handleWifiScan(struct webserver_t *client);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
int showFirmware(struct webserver_t *client);
int showFirmwareSuccess(struct webserver_t *client);
int showFirmwareFail(struct webserver_t *client);
