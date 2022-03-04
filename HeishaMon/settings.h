#include "s0.h"
#include "gpio.h"
#include "smartcontrol.h"
#include "ArduinoJson.h"

struct Settings {
  // Constants for names of settings (used in JSON and HTTP POST)
  static const char* hotspot_mode;
  static const char* wifi_ssid;
  static const char* wifi_password;
  static const char* wifi_hostname;
  static const char* ota_password;
  static const char* mqtt_topic_base;
  static const char* mqtt_server;
  static const char* mqtt_port;
  static const char* mqtt_username;
  static const char* mqtt_password;
  static const char* use_1wire;
  static const char* use_s0;
  static const char* listenonly;
  static const char* logMqtt;
  static const char* logHexdump;
  static const char* logSerial1;
  static const char* optionalPCB;
  static const char* waitTime;
  static const char* waitDallasTime;
  static const char* updateAllTime;
  static const char* updateAllDallasTime;
  static const char* s0_1_gpio;
  static const char* s0_1_ppkwh;
  static const char* s0_1_interval;
  static const char* s0_2_gpio;
  static const char* s0_2_ppkwh;
  static const char* s0_2_interval;

  // Conversion of boolean value to/from string
  static const char* toString(bool value);
  static bool toBool(const char* value);
};

enum HotspotMode {
  None = 0,
  Secured = 1,
  Unsecured = 2,  
};

// The settings themselves
struct SettingsStruct {
  unsigned int waitTime = 5; // how often data is read from heatpump
  unsigned int waitDallasTime = 5; // how often temps are read from 1wire
  unsigned int updateAllTime = 300; // how often all data is resend to mqtt
  unsigned int updateAllDallasTime = 300; //how often all 1wire data is resent to mqtt

  const char* hotspot_ssid = "HeishaMon-Setup";
  HotspotMode hotspot_mode = HotspotMode::Unsecured;
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

  void fromJson(DynamicJsonDocument& jsonDoc);
  void toJson(DynamicJsonDocument& jsonDoc);
};
