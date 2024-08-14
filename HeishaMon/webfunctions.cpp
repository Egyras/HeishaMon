#include "webfunctions.h"
#include "decode.h"
#include "version.h"
#include "htmlcode.h"
#include "commands.h"
#include "src/common/progmem.h"
#include "src/common/webserver.h"
#include "src/common/timerqueue.h"

#include "lwip/apps/sntp.h"
#include "lwip/dns.h"

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <time.h>

#define UPTIME_OVERFLOW 4294967295 // Uptime overflow value

static String wifiJsonList = "";

static uint8_t ntpservers = 0;

void log_message(char* string);
void log_message(const __FlashStringHelper *msg);

int dBmToQuality(int dBm) {
  if (dBm == 31)
    return -1;
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}


void getWifiScanResults(int numSsid) {
  if (numSsid > 0) { //found wifi networks
    wifiJsonList = "[";
    int indexes[numSsid];
    for (int i = 0; i < numSsid; i++) { //fill the sorted list with normal indexes first
      indexes[i] = i;
    }
    for (int i = 0; i < numSsid; i++) { //then sort
      for (int j = i + 1; j < numSsid; j++) {
        if (WiFi.RSSI(indexes[j]) > WiFi.RSSI(indexes[i])) {
          int temp = indexes[j];
          indexes[j] = indexes[i];
          indexes[i] = temp;
        }
      }
    }
    String ssid;
    for (int i = 0; i < numSsid; i++) { //then remove duplicates
      if (indexes[i] == -1) continue;
      ssid = WiFi.SSID(indexes[i]);
      for (int j = i + 1; j < numSsid; j++) {
        if (ssid == WiFi.SSID(indexes[j])) {
          indexes[j] = -1;
        }
      }
    }
    bool firstSSID = true;
    for (int i = 0; i < numSsid; i++) { //then output json
      if (indexes[i] == -1) {
        continue;
      }
      if (!firstSSID) {
        wifiJsonList = wifiJsonList + ",";
      }
      wifiJsonList = wifiJsonList + "{\"ssid\":\"" + WiFi.SSID(indexes[i]) + "\", \"rssi\": \"" + dBmToQuality(WiFi.RSSI(indexes[i])) + "%\"}";
      firstSSID = false;
    }
    wifiJsonList = wifiJsonList + "]";
  }
}

int getWifiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  return dBmToQuality(WiFi.RSSI());
}

int getFreeMemory() {
  //store total memory at boot time
  static uint32_t total_memory = 0;  
#if defined(ESP8266)
  if ( 0 == total_memory ) total_memory = ESP.getFreeHeap();
#else
  //on esp32 we have the total heap size
  if ( 0 == total_memory ) total_memory = ESP.getHeapSize();
#endif

  uint32_t free_memory   = ESP.getFreeHeap();
  return (100 * free_memory / total_memory ) ; // as a %
}

// returns system uptime in seconds
char *getUptime(void) {
  static uint32_t last_uptime      = 0;
  static uint8_t  uptime_overflows = 0;

  if (millis() < last_uptime) {
    ++uptime_overflows;
  }
  last_uptime             = millis();
  uint32_t t = uptime_overflows * (UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

  uint8_t  d   = t / 86400L;
  uint8_t  h   = ((t % 86400L) / 3600L) % 60;
  uint32_t rem = t % 3600L;
  uint8_t  m   = rem / 60;
  uint8_t  sec = rem % 60;

  unsigned int len = snprintf_P(NULL, 0, PSTR("%d day%s %d hour%s %d minute%s %d second%s"), d, (d == 1) ? "" : "s", h, (h == 1) ? "" : "s", m, (m == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");

  char *str = (char *)malloc(len + 2);
  if (str == NULL) {
    Serial1.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
    ESP.restart();
    exit(-1);
  }

  memset(str, 0, len + 2);
  snprintf_P(str, len + 1, PSTR("%d day%s %d hour%s %d minute%s %d second%s"), d, (d == 1) ? "" : "s", h, (h == 1) ? "" : "s", m, (m == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
  return str;
}

#if defined(ESP8266)
void ntp_dns_found(const char *name, const ip4_addr *addr, void *arg) {
  sntp_stop();
  sntp_setserver(ntpservers++, addr);
  sntp_init();
}
#elif defined(ESP32)
void ntp_dns_found(const char *name, const ip_addr_t *addr, void *arg) {
  sntp_stop();
  sntp_setserver(ntpservers++, addr);
  sntp_init();
}
#endif


void ntpReload(settingsStruct *heishamonSettings) {
  ip_addr_t addr;
  uint8_t len = strlen(heishamonSettings->ntp_servers);
  uint8_t ptr = 0, i = 0;
  ntpservers = 0;
  for (i = 0; i <= len; i++) {
    if (heishamonSettings->ntp_servers[i] == ',') {
      heishamonSettings->ntp_servers[i] = 0;

      uint8_t err = dns_gethostbyname(&heishamonSettings->ntp_servers[ptr], &addr, ntp_dns_found, 0);
      if (err == ERR_OK) {
        sntp_stop();
        sntp_setserver(ntpservers++, &addr);
        sntp_init();
      }
      heishamonSettings->ntp_servers[i++] = ',';
      while (heishamonSettings->ntp_servers[i] == ' ') {
        i++;
      }
      ptr = i;
    }
  }

  uint8_t err = dns_gethostbyname(&heishamonSettings->ntp_servers[ptr], &addr, ntp_dns_found, 0);
  if (err == ERR_OK) {
    sntp_stop();
    sntp_setserver(ntpservers++, &addr);
    sntp_init();
  }

  sntp_stop();
  tzStruct tz;
  memcpy_P(&tz, &tzdata[heishamonSettings->timezone], sizeof(tz));
#if defined(ESP8266)
  setTZ(tz.value);
#elif defined(ESP32)
  setenv("TZ",tz.value,1);
  tzset();
#endif
  sntp_init();
}

void loadSettings(settingsStruct *heishamonSettings) {
  //read configuration from FS json
  log_message(_F("mounting FS..."));

  if (LittleFS.begin()) {
    log_message(_F("mounted file system"));
    if (LittleFS.exists("/config.json")) {
      //file exists, reading and loading
      log_message(_F("reading config file"));
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        log_message(_F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        JsonDocument jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, buf.get());
        char log_msg[1024];
        serializeJson(jsonDoc, log_msg);
        log_message(log_msg);
        if (!error) {
          log_message(_F("parsed json"));
          //read updated parameters, make sure no overflow
          if ( jsonDoc["wifi_ssid"] ) strlcpy(heishamonSettings->wifi_ssid, jsonDoc["wifi_ssid"], sizeof(heishamonSettings->wifi_ssid));
          if ( jsonDoc["wifi_password"] ) strlcpy(heishamonSettings->wifi_password, jsonDoc["wifi_password"], sizeof(heishamonSettings->wifi_password));
          if ( jsonDoc["wifi_hostname"] ) strlcpy(heishamonSettings->wifi_hostname, jsonDoc["wifi_hostname"], sizeof(heishamonSettings->wifi_hostname));
          if ( jsonDoc["ota_password"] ) strlcpy(heishamonSettings->ota_password, jsonDoc["ota_password"], sizeof(heishamonSettings->ota_password));
          if ( jsonDoc["mqtt_topic_base"] ) strlcpy(heishamonSettings->mqtt_topic_base, jsonDoc["mqtt_topic_base"], sizeof(heishamonSettings->mqtt_topic_base));
          if ( jsonDoc["mqtt_topic_listen"] ) strlcpy(heishamonSettings->mqtt_topic_listen, jsonDoc["mqtt_topic_listen"], sizeof(heishamonSettings->mqtt_topic_listen));
          if ( jsonDoc["mqtt_server"] ) strlcpy(heishamonSettings->mqtt_server, jsonDoc["mqtt_server"], sizeof(heishamonSettings->mqtt_server));
          if ( jsonDoc["mqtt_port"] ) strlcpy(heishamonSettings->mqtt_port, jsonDoc["mqtt_port"], sizeof(heishamonSettings->mqtt_port));
          if ( jsonDoc["mqtt_username"] ) strlcpy(heishamonSettings->mqtt_username, jsonDoc["mqtt_username"], sizeof(heishamonSettings->mqtt_username));
          if ( jsonDoc["mqtt_password"] ) strlcpy(heishamonSettings->mqtt_password, jsonDoc["mqtt_password"], sizeof(heishamonSettings->mqtt_password));
          if ( jsonDoc["ntp_servers"] ) strlcpy(heishamonSettings->ntp_servers, jsonDoc["ntp_servers"], sizeof(heishamonSettings->ntp_servers));
          if ( jsonDoc["timezone"]) heishamonSettings->timezone = jsonDoc["timezone"];
          heishamonSettings->use_1wire = ( jsonDoc["use_1wire"] == "enabled" ) ? true : false;
          heishamonSettings->use_s0 = ( jsonDoc["use_s0"] == "enabled" ) ? true : false;
          heishamonSettings->listenonly = ( jsonDoc["listenonly"] == "enabled" ) ? true : false;
          heishamonSettings->listenmqtt = ( jsonDoc["listenmqtt"] == "enabled" ) ? true : false;
          heishamonSettings->logMqtt = ( jsonDoc["logMqtt"] == "enabled" ) ? true : false;
          heishamonSettings->logHexdump = ( jsonDoc["logHexdump"] == "enabled" ) ? true : false;
          heishamonSettings->logSerial1 = ( jsonDoc["logSerial1"] == "enabled" ) ? true : false;
          heishamonSettings->optionalPCB = ( jsonDoc["optionalPCB"] == "enabled" ) ? true : false;
          heishamonSettings->opentherm = ( jsonDoc["opentherm"] == "enabled" ) ? true : false;
#ifdef ESP32          
          heishamonSettings->proxy = ( jsonDoc["proxy"] == "enabled" ) ? true : false;
#endif          
          if ( jsonDoc["waitTime"]) heishamonSettings->waitTime = jsonDoc["waitTime"];
          if (heishamonSettings->waitTime < 5) heishamonSettings->waitTime = 5;
          if ( jsonDoc["waitDallasTime"]) heishamonSettings->waitDallasTime = jsonDoc["waitDallasTime"];
          if (heishamonSettings->waitDallasTime < 5) heishamonSettings->waitDallasTime = 5;
          if ( jsonDoc["dallasResolution"]) heishamonSettings->dallasResolution = jsonDoc["dallasResolution"];
          if ((heishamonSettings->dallasResolution < 9) || (heishamonSettings->dallasResolution > 12) ) heishamonSettings->dallasResolution = 12;
          if ( jsonDoc["updateAllTime"]) heishamonSettings->updateAllTime = jsonDoc["updateAllTime"];
          if (heishamonSettings->updateAllTime < heishamonSettings->waitTime) heishamonSettings->updateAllTime = heishamonSettings->waitTime;
          if ( jsonDoc["updataAllDallasTime"]) heishamonSettings->updataAllDallasTime = jsonDoc["updataAllDallasTime"];
          if (heishamonSettings->updataAllDallasTime < heishamonSettings->waitDallasTime) heishamonSettings->updataAllDallasTime = heishamonSettings->waitDallasTime;
          if (jsonDoc["s0_1_gpio"]) heishamonSettings->s0Settings[0].gpiopin = jsonDoc["s0_1_gpio"];
          if (jsonDoc["s0_1_ppkwh"]) heishamonSettings->s0Settings[0].ppkwh = jsonDoc["s0_1_ppkwh"];
          if (jsonDoc["s0_1_interval"]) heishamonSettings->s0Settings[0].lowerPowerInterval = jsonDoc["s0_1_interval"];
          if (jsonDoc["s0_1_minpulsewidth"]) heishamonSettings->s0Settings[0].minimalPulseWidth = jsonDoc["s0_1_minpulsewidth"];
          if (jsonDoc["s0_1_maxpulsewidth"]) heishamonSettings->s0Settings[0].maximalPulseWidth = jsonDoc["s0_1_maxpulsewidth"];
          if (jsonDoc["s0_2_gpio"]) heishamonSettings->s0Settings[1].gpiopin = jsonDoc["s0_2_gpio"];
          if (jsonDoc["s0_2_ppkwh"]) heishamonSettings->s0Settings[1].ppkwh = jsonDoc["s0_2_ppkwh"];
          if (jsonDoc["s0_2_interval"] ) heishamonSettings->s0Settings[1].lowerPowerInterval = jsonDoc["s0_2_interval"];
          if (jsonDoc["s0_2_minpulsewidth"]) heishamonSettings->s0Settings[1].minimalPulseWidth = jsonDoc["s0_2_minpulsewidth"];
          if (jsonDoc["s0_2_maxpulsewidth"]) heishamonSettings->s0Settings[1].maximalPulseWidth = jsonDoc["s0_2_maxpulsewidth"];
          ntpReload(heishamonSettings);
        } else {
          log_message(_F("Failed to load json config, forcing config reset."));
          WiFi.persistent(true);
          WiFi.disconnect();
          WiFi.persistent(false);
        }
        configFile.close();
      }
    }
    else {
      log_message(_F("No config.json exists! Forcing a config reset."));
      WiFi.persistent(true);
      WiFi.disconnect();
      WiFi.persistent(false);
    }
  } else {
    log_message(_F("failed to mount FS"));
  }
  //end read

}

void setupWifi(settingsStruct *heishamonSettings) {
  log_message(_F("Wifi reconnecting with new configuration..."));
#if defined(ESP8266)
  //no sleep wifi
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);

  if (heishamonSettings->wifi_ssid[0] != '\0') {
    log_message(_F("Wifi client mode..."));
    //WiFi.persistent(true); //breaks stuff

    if (heishamonSettings->wifi_password[0] == '\0') {
      WiFi.begin(heishamonSettings->wifi_ssid);
    } else {
      WiFi.begin(heishamonSettings->wifi_ssid, heishamonSettings->wifi_password);
    }
  }
  else {
    log_message(_F("Wifi hotspot mode..."));
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(_F("HeishaMon-Setup"));
    
  }

  if (heishamonSettings->wifi_hostname[0] == '\0') {
    //Set hostname on wifi rather than ESP_xxxxx
    WiFi.hostname(_F("HeishaMon"));
  } else {
    WiFi.hostname(heishamonSettings->wifi_hostname);
  }
#elif defined(ESP32)
  //WiFi.setTxPower(WIFI_POWER_8_5dBm); //fix for bad chips
  WiFi.setSleep(false);
  WiFi.softAPdisconnect(true); 
  delay(100);  // must delay to avoid error 
  WiFi.disconnect(true); 
  //sethostname on ESP32 before wifi.begin
  if (heishamonSettings->wifi_hostname[0] == '\0') {
    //Set hostname on wifi rather than ESP_xxxxx
    WiFi.setHostname("HeishaMon");
  } else {
    WiFi.setHostname(heishamonSettings->wifi_hostname);
  }
  if (heishamonSettings->wifi_ssid[0] != '\0') {
     log_message(_F("Wifi client mode..."));
    if (heishamonSettings->wifi_password[0] == '\0') {
        WiFi.begin(heishamonSettings->wifi_ssid);
      } else {
        WiFi.begin(heishamonSettings->wifi_ssid, heishamonSettings->wifi_password);
      }
  }
  else {
    log_message(_F("Wifi hotspot mode..."));
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); 
    WiFi.softAP("HeishaMon-Setup");
  }


#endif
}

int handleFactoryReset(struct webserver_t *client) {
  switch (client->content) {
    case 0: {
        webserver_send(client, 200, (char *)"text/html", 0);
        webserver_send_content_P(client, webHeader, strlen_P(webHeader));
        webserver_send_content_P(client, webCSS, strlen_P(webCSS));
        webserver_send_content_P(client, refreshMeta, strlen_P(refreshMeta));
      } break;
    case 1: {
        webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
        webserver_send_content_P(client, webBodyRebootWarning, strlen_P(webBodyRebootWarning));
        webserver_send_content_P(client, menuJS, strlen_P(menuJS));
        webserver_send_content_P(client, webFooter, strlen_P(webFooter));
      } break;
    case 2: {
        timerqueue_insert(1, 0, -1); // Start reboot sequence
      } break;
  }

  return 0;
}

int handleReboot(struct webserver_t *client) {
  switch (client->content) {
    case 0: {
        webserver_send(client, 200, (char *)"text/html", 0);
        webserver_send_content_P(client, webHeader, strlen_P(webHeader));
        webserver_send_content_P(client, webCSS, strlen_P(webCSS));
        webserver_send_content_P(client, refreshMeta, strlen_P(refreshMeta));
      } break;
    case 1: {
        webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
        webserver_send_content_P(client, webBodyRebootWarning, strlen_P(webBodyRebootWarning));
        webserver_send_content_P(client, menuJS, strlen_P(menuJS));
        webserver_send_content_P(client, webFooter, strlen_P(webFooter));
      } break;
    case 2: {
        timerqueue_insert(5, 0, -2); // Start reboot sequence
      } break;
  }

  return 0;
}

void settingsToJson(JsonDocument &jsonDoc, settingsStruct *heishamonSettings) {
  //set jsonDoc with current settings
  jsonDoc["wifi_hostname"] = heishamonSettings->wifi_hostname;
  jsonDoc["wifi_password"] = heishamonSettings->wifi_password;
  jsonDoc["wifi_ssid"] = heishamonSettings->wifi_ssid;
  jsonDoc["ota_password"] = heishamonSettings->ota_password;
  jsonDoc["mqtt_topic_base"] = heishamonSettings->mqtt_topic_base;
  jsonDoc["mqtt_topic_listen"] = heishamonSettings->mqtt_topic_listen;
  jsonDoc["mqtt_server"] = heishamonSettings->mqtt_server;
  jsonDoc["mqtt_port"] = heishamonSettings->mqtt_port;
  jsonDoc["mqtt_username"] = heishamonSettings->mqtt_username;
  jsonDoc["mqtt_password"] = heishamonSettings->mqtt_password;
  if (heishamonSettings->use_1wire) {
    jsonDoc["use_1wire"] = "enabled";
  } else {
    jsonDoc["use_1wire"] = "disabled";
  }
  if (heishamonSettings->use_s0) {
    jsonDoc["use_s0"] = "enabled";
  } else {
    jsonDoc["use_s0"] = "disabled";
  }
  if (heishamonSettings->listenonly) {
    jsonDoc["listenonly"] = "enabled";
  } else {
    jsonDoc["listenonly"] = "disabled";
  }
  if (heishamonSettings->listenmqtt) {
    jsonDoc["listenmqtt"] = "enabled";
  } else {
    jsonDoc["listenmqtt"] = "disabled";
  }
  if (heishamonSettings->logMqtt) {
    jsonDoc["logMqtt"] = "enabled";
  } else {
    jsonDoc["logMqtt"] = "disabled";
  }
  if (heishamonSettings->logHexdump) {
    jsonDoc["logHexdump"] = "enabled";
  } else {
    jsonDoc["logHexdump"] = "disabled";
  }
  if (heishamonSettings->logSerial1) {
    jsonDoc["logSerial1"] = "enabled";
  } else {
    jsonDoc["logSerial1"] = "disabled";
  }
  if (heishamonSettings->optionalPCB) {
    jsonDoc["optionalPCB"] = "enabled";
  } else {
    jsonDoc["optionalPCB"] = "disabled";
  }
  if (heishamonSettings->opentherm) {
    jsonDoc["opentherm"] = "enabled";
  } else {
    jsonDoc["opentherm"] = "disabled";
  }
#ifdef ESP32  
  if (heishamonSettings->proxy) {
    jsonDoc["proxy"] = "enabled";
  } else {
    jsonDoc["proxy"] = "disabled";
  }
#endif 
  jsonDoc["waitTime"] = heishamonSettings->waitTime;
  jsonDoc["waitDallasTime"] = heishamonSettings->waitDallasTime;
  jsonDoc["dallasResolution"] = heishamonSettings->dallasResolution;
  jsonDoc["updateAllTime"] = heishamonSettings->updateAllTime;
  jsonDoc["updataAllDallasTime"] = heishamonSettings->updataAllDallasTime;
}

void saveJsonToConfig(JsonDocument &jsonDoc) {
  if (LittleFS.begin()) {
    File configFile = LittleFS.open("/config.json", "w");
    if (configFile) {
      serializeJson(jsonDoc, configFile);
      configFile.close();
    }
  }
}

int saveSettings(struct webserver_t *client, settingsStruct *heishamonSettings) {
  const char *wifi_ssid = NULL;
  const char *wifi_password = NULL;
  const char *new_ota_password = NULL;
  const char *current_ota_password = NULL;
  const char *use_s0 = NULL;

  bool reconnectWiFi = false;
  bool wrongPassword = false;
  JsonDocument jsonDoc;

  settingsToJson(jsonDoc, heishamonSettings); //stores current settings in a json document

  jsonDoc["listenonly"] = String("");
  jsonDoc["listenmqtt"] = String("");
  jsonDoc["logMqtt"] = String("");
  jsonDoc["logHexdump"] = String("");
  jsonDoc["logSerial1"] = String("");
  jsonDoc["optionalPCB"] = String("");
  jsonDoc["opentherm"] = String("");
#ifdef ESP32  
  jsonDoc["proxy"] = String("");
#endif  
  jsonDoc["use_1wire"] = String("");
  jsonDoc["use_s0"] = String("");

  struct websettings_t *tmp = (struct websettings_t *)client->userdata;
  while (tmp) {
    if (strcmp(tmp->name.c_str(), "wifi_hostname") == 0) {
      jsonDoc["wifi_hostname"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_topic_base") == 0) {
      jsonDoc["mqtt_topic_base"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_topic_listen") == 0) {
      jsonDoc["mqtt_topic_listen"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_server") == 0) {
      jsonDoc["mqtt_server"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_port") == 0) {
      jsonDoc["mqtt_port"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_username") == 0) {
      jsonDoc["mqtt_username"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "mqtt_password") == 0) {
      jsonDoc["mqtt_password"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "use_1wire") == 0) {
      jsonDoc["use_1wire"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "use_s0") == 0) {
      jsonDoc["use_s0"] = tmp->value;
      if (strcmp(tmp->value.c_str(), "enabled") == 0) {
        use_s0 = tmp->value.c_str();
      }
    } else if (strcmp(tmp->name.c_str(), "listenonly") == 0) {
      jsonDoc["listenonly"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "listenmqtt") == 0) {
      jsonDoc["listenmqtt"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "logMqtt") == 0) {
      jsonDoc["logMqtt"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "logHexdump") == 0) {
      jsonDoc["logHexdump"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "logSerial1") == 0) {
      jsonDoc["logSerial1"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "optionalPCB") == 0) {
      jsonDoc["optionalPCB"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "opentherm") == 0) {
      jsonDoc["opentherm"] = tmp->value;
#ifdef ESP32      
    } else if (strcmp(tmp->name.c_str(), "proxy") == 0) {
      jsonDoc["proxy"] = tmp->value;
#endif      
    } else if (strcmp(tmp->name.c_str(), "ntp_servers") == 0) {
      jsonDoc["ntp_servers"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "timezone") == 0) {
      jsonDoc["timezone"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "waitTime") == 0) {
      jsonDoc["waitTime"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "waitDallasTime") == 0) {
      jsonDoc["waitDallasTime"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "updateAllTime") == 0) {
      jsonDoc["updateAllTime"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "dallasResolution") == 0) {
      jsonDoc["dallasResolution"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "updataAllDallasTime") == 0) {
      jsonDoc["updataAllDallasTime"] = tmp->value;
    } else if (strcmp(tmp->name.c_str(), "wifi_ssid") == 0) {
      wifi_ssid = tmp->value.c_str();
    } else if (strcmp(tmp->name.c_str(), "wifi_password") == 0) {
      wifi_password = tmp->value.c_str();
    } else if (strcmp(tmp->name.c_str(), "new_ota_password") == 0) {
      new_ota_password = tmp->value.c_str();
    } else if (strcmp(tmp->name.c_str(), "current_ota_password") == 0) {
      current_ota_password = tmp->value.c_str();
    }
    tmp = tmp->next;
  }

  tmp = (struct websettings_t *)client->userdata;
  while (tmp) {
    if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_1_gpio") == 0) {
      jsonDoc["s0_1_gpio"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_1_ppkwh") == 0) {
      jsonDoc["s0_1_ppkwh"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_1_interval") == 0) {
      jsonDoc["s0_1_interval"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_1_minpulsewidth") == 0) {
      jsonDoc["s0_1_minpulsewidth"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_1_maxpulsewidth") == 0) {
      jsonDoc["s0_1_maxpulsewidth"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_gpio") == 0) {
      jsonDoc["s0_2_gpio"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_ppkwh") == 0) {
      jsonDoc["s0_2_ppkwh"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_ppkwh") == 0) {
      jsonDoc["s0_2_ppkwh"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_interval") == 0) {
      jsonDoc["s0_2_interval"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_minpulsewidth") == 0) {
      jsonDoc["s0_2_minpulsewidth"] = tmp->value;
    } else if (use_s0 != NULL && strcmp(tmp->name.c_str(), "s0_2_maxpulsewidth") == 0) {
      jsonDoc["s0_2_maxpulsewidth"] = tmp->value;
    }
    tmp = tmp->next;
  }

  if (new_ota_password != NULL && strlen(new_ota_password) > 0 && current_ota_password != NULL && strlen(current_ota_password) > 0) {
    if (strcmp(heishamonSettings->ota_password, current_ota_password) == 0) {
      jsonDoc["ota_password"] = new_ota_password;
    } else {
      wrongPassword = true;
    }
  }

  if (wifi_password != NULL && wifi_ssid != NULL && strlen(wifi_ssid) > 0 && strlen(wifi_password) > 0) {
    if (strcmp(jsonDoc["wifi_ssid"], wifi_ssid) != 0 || strcmp(jsonDoc["wifi_password"], wifi_password) != 0) {
      reconnectWiFi = true;
    }
  }
  if (wifi_ssid != NULL) {
    jsonDoc["wifi_ssid"] = String(wifi_ssid);
  }
  if (wifi_password != NULL) {
    jsonDoc["wifi_password"] = String(wifi_password);
  }

  saveJsonToConfig(jsonDoc); //save to config file
  loadSettings(heishamonSettings); //load config file to current settings

  while (client->userdata) {
    tmp = (struct websettings_t *)client->userdata;
    client->userdata = ((struct websettings_t *)(client->userdata))->next;
    delete tmp;
  }

  if (wrongPassword) {
    client->route = 111;
    return 0;
  }

  if (reconnectWiFi) {
    client->route = 112;
    return 0;
  }

  client->route = 113;
  return 0;
}

int cacheSettings(struct webserver_t *client, struct arguments_t * args) {
  struct websettings_t *tmp = (struct websettings_t *)client->userdata;
  while (tmp) {
    /*
        this part is useless as websettings is always NULL at start of a new POST
        it will only interrate over already POSTed args which are pushed on the list below
        we only need to find the tail of the list
        /

      if (strcmp(tmp->name.c_str(), (char *)args->name) == 0) {
        char *cpy = (char *)malloc(args->len + 1);
        memset(cpy, 0, args->len + 1);
        memcpy(cpy, args->value, args->len);
        tmp->value += cpy;
        free(cpy);
        break;
      }
    */
    tmp = tmp->next;
  }
  if (tmp == NULL) {
    websettings_t *node = new websettings_t;
    if (node == NULL) {
      Serial1.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
      ESP.restart();
      exit(-1);
    }
    node->next = NULL;
    node->name += (char *)args->name;
    if (args->value != NULL) {
      char *cpy = (char *)malloc(args->len + 1);
      if (node == NULL) {
        Serial1.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
        ESP.restart();
        exit(-1);
      }
      memset(cpy, 0, args->len + 1);
      strncpy(cpy, (char *)args->value, args->len);
      node->value += cpy;
      free(cpy);
    }

    node->next = (struct websettings_t *)client->userdata;
    client->userdata = node;
  }

  return 0;
}

int settingsNewPassword(struct webserver_t *client, settingsStruct *heishamonSettings) {
  switch (client->content) {
    case 0: {
        webserver_send(client, 200, (char *)"text/html", 0);
        webserver_send_content_P(client, webHeader, strlen_P(webHeader));
        webserver_send_content_P(client, webCSS, strlen_P(webCSS));
        webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
      } break;
    case 1: {
        webserver_send_content_P(client, webBodySettings1, strlen_P(webBodySettings1));
        webserver_send_content_P(client, webBodySettingsResetPasswordWarning, strlen_P(webBodySettingsResetPasswordWarning));
      } break;
    case 2: {
        webserver_send_content_P(client, refreshMeta, strlen_P(refreshMeta));
        webserver_send_content_P(client, webFooter, strlen_P(webFooter));
      } break;
    case 3: {
        setupConditionals();
      } break;
  }

  return 0;
}

int settingsReconnectWifi(struct webserver_t *client, settingsStruct *heishamonSettings) {
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"text/html", 0);
    webserver_send_content_P(client, webHeader, strlen_P(webHeader));
    webserver_send_content_P(client, webCSS, strlen_P(webCSS));
    webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
    webserver_send_content_P(client, webBodySettings1, strlen_P(webBodySettings1));
  } else if (client->content == 2) {
    webserver_send_content_P(client, menuJS, strlen_P(menuJS));
    webserver_send_content_P(client, webBodySettingsNewWifiWarning, strlen_P(webBodySettingsNewWifiWarning));
    webserver_send_content_P(client, refreshMeta, strlen_P(refreshMeta));
    webserver_send_content_P(client, webFooter, strlen_P(webFooter));
    timerqueue_insert(5, 0, -3); //handle wifi reconnect after 5 sec to make sure all above data is sent to client so no memory leak is introduced
  }

  return 0;
}

int getSettings(struct webserver_t *client, settingsStruct *heishamonSettings) {
  switch (client->content) {
    case 0: {
        webserver_send(client, 200, (char *)"application/json", 0);
        webserver_send_content_P(client, PSTR("{\"wifi_hostname\":\""), 18);
        webserver_send_content(client, heishamonSettings->wifi_hostname, strlen(heishamonSettings->wifi_hostname));
        webserver_send_content_P(client, PSTR("\",\"wifi_ssid\":\""), 15);
        webserver_send_content(client, heishamonSettings->wifi_ssid, strlen(heishamonSettings->wifi_ssid));
      } break;
    case 1: {
        webserver_send_content_P(client, PSTR("\",\"wifi_password\":\""), 19);
        webserver_send_content(client, heishamonSettings->wifi_password, strlen(heishamonSettings->wifi_password));
        webserver_send_content_P(client, PSTR("\",\"current_ota_password\":\""), 26);
        webserver_send_content_P(client, PSTR("\",\"new_ota_password\":\""), 22);
      } break;
    case 2: {
        webserver_send_content_P(client, PSTR("\",\"mqtt_topic_base\":\""), 21);
        webserver_send_content(client, heishamonSettings->mqtt_topic_base, strlen(heishamonSettings->mqtt_topic_base));
        webserver_send_content_P(client, PSTR("\",\"mqtt_server\":\""), 17);
        webserver_send_content(client, heishamonSettings->mqtt_server, strlen(heishamonSettings->mqtt_server));
      } break;
    case 3: {
        webserver_send_content_P(client, PSTR("\",\"mqtt_port\":\""), 15);
        webserver_send_content(client, heishamonSettings->mqtt_port, strlen(heishamonSettings->mqtt_port));
        webserver_send_content_P(client, PSTR("\",\"mqtt_username\":\""), 19);
        webserver_send_content(client, heishamonSettings->mqtt_username, strlen(heishamonSettings->mqtt_username));
      } break;
    case 4: {
        webserver_send_content_P(client, PSTR("\",\"mqtt_password\":\""), 19);
        webserver_send_content(client, heishamonSettings->mqtt_password, strlen(heishamonSettings->mqtt_password));
        webserver_send_content_P(client, PSTR("\",\"ntp_servers\":\""), 17);
        webserver_send_content(client, heishamonSettings->ntp_servers, strlen(heishamonSettings->ntp_servers));
        webserver_send_content_P(client, PSTR("\",\"timezone\":"), 13);

        {
          char str[20];
          itoa(heishamonSettings->timezone, str, 10);
          webserver_send_content(client, str, strlen(str));
        }

        webserver_send_content_P(client, PSTR(",\"waitTime\":"), 12);

        {
          char str[20];
          itoa(heishamonSettings->waitTime, str, 10);
          webserver_send_content(client, str, strlen(str));
        }
      } break;
    case 5: {
        char str[20];
        webserver_send_content_P(client, PSTR(",\"updateAllTime\":"), 17);

        itoa(heishamonSettings->updateAllTime, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"listenonly\":"), 14);

        itoa(heishamonSettings->listenonly, str, 10);
        webserver_send_content(client, str, strlen(str));
      } break;
    case 6: {
        char str[20];
        webserver_send_content_P(client, PSTR(",\"listenmqtt\":"), 14);

        itoa(heishamonSettings->listenmqtt, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"mqtt_topic_listen\":\""), 22);
        webserver_send_content(client, heishamonSettings->mqtt_topic_listen, strlen(heishamonSettings->mqtt_topic_listen));
      } break;
    case 7: {
        char str[20];
        webserver_send_content_P(client, PSTR("\",\"logMqtt\":"), 12);

        itoa(heishamonSettings->logMqtt, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"logHexdump\":"), 14);

        itoa(heishamonSettings->logHexdump, str, 10);
        webserver_send_content(client, str, strlen(str));
      } break;
    case 8: {
        char str[20];
        webserver_send_content_P(client, PSTR(",\"logSerial1\":"), 14);
        itoa(heishamonSettings->logSerial1, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"optionalPCB\":"), 15);
        itoa(heishamonSettings->optionalPCB, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"opentherm\":"), 13);
        itoa(heishamonSettings->opentherm, str, 10);
        webserver_send_content(client, str, strlen(str));
      } break;
    case 9: {
        char str[20];
#ifdef ESP32        
        webserver_send_content_P(client, PSTR(",\"proxy\":"), 9);
        itoa(heishamonSettings->proxy, str, 10);
        webserver_send_content(client, str, strlen(str));
#endif      
        webserver_send_content_P(client, PSTR(",\"use_1wire\":"), 13);
        itoa(heishamonSettings->use_1wire, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"waitDallasTime\":"), 18);
        itoa(heishamonSettings->waitDallasTime, str, 10);
        webserver_send_content(client, str, strlen(str));
      } break;
    case 10: {
        char str[20];
        webserver_send_content_P(client, PSTR(",\"updataAllDallasTime\":"), 23);

        itoa(heishamonSettings->updataAllDallasTime, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"dallasResolution\":"), 20);

        itoa(heishamonSettings->dallasResolution , str, 10);
        webserver_send_content(client, str, strlen(str));
      } break;
    case 11: {
        char str[20];
        webserver_send_content_P(client, PSTR(",\"use_s0\":"), 10);

        itoa(heishamonSettings->use_s0, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_gpio\":"), 13);

        int i = 0;

        if (heishamonSettings->s0Settings[i].gpiopin == 255) heishamonSettings->s0Settings[i].gpiopin = DEFAULT_S0_PIN_1;  //dirty hack
        itoa(heishamonSettings->s0Settings[i].gpiopin, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_ppkwh\":"), 14);

        itoa(heishamonSettings->s0Settings[i].ppkwh, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_interval\":"), 17);

        itoa(heishamonSettings->s0Settings[i].lowerPowerInterval, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_minpulsewidth\":"), 22);

        itoa(heishamonSettings->s0Settings[i].minimalPulseWidth, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_maxpulsewidth\":"), 22);

        itoa(heishamonSettings->s0Settings[i].maximalPulseWidth, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_1_minwatt\":"), 16);

        itoa((int) round((3600 * 1000 / heishamonSettings->s0Settings[i].ppkwh) / heishamonSettings->s0Settings[i].lowerPowerInterval), str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_gpio\":"), 13);
      } break;
    case 12: {
        char str[20];
        int i = 1;

        if (heishamonSettings->s0Settings[i].gpiopin == 255) heishamonSettings->s0Settings[i].gpiopin = DEFAULT_S0_PIN_2;  //dirty hack
        itoa(heishamonSettings->s0Settings[i].gpiopin, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_ppkwh\":"), 14);

        itoa(heishamonSettings->s0Settings[i].ppkwh, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_interval\":"), 17);

        itoa(heishamonSettings->s0Settings[i].lowerPowerInterval, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_minpulsewidth\":"), 22);

        itoa(heishamonSettings->s0Settings[i].minimalPulseWidth, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_maxpulsewidth\":"), 22);

        itoa(heishamonSettings->s0Settings[i].maximalPulseWidth, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR(",\"s0_2_minwatt\":"), 16);

        itoa((int) round((3600 * 1000 / heishamonSettings->s0Settings[i].ppkwh) / heishamonSettings->s0Settings[i].lowerPowerInterval), str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR("}"), 1);
      } break;
  }
  return 0;
}

int handleSettings(struct webserver_t *client) {
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"text/html", 0);
    webserver_send_content_P(client, webHeader, strlen_P(webHeader));
    webserver_send_content_P(client, webCSS, strlen_P(webCSS));
    webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
  } else if (client->content == 1) {
    webserver_send_content_P(client, webBodySettings1, strlen_P(webBodySettings1));
    webserver_send_content_P(client, settingsForm1, strlen_P(settingsForm1));
    webserver_send_content_P(client, tzDataOptions, strlen_P(tzDataOptions));
  } else if (client->content == 2) {
    webserver_send_content_P(client, settingsForm2, strlen_P(settingsForm2));
    webserver_send_content_P(client, menuJS, strlen_P(menuJS));
    webserver_send_content_P(client, settingsJS, strlen_P(settingsJS));
    webserver_send_content_P(client, populategetsettingsJS, strlen_P(populategetsettingsJS));
  } else if (client->content == 3) {
    webserver_send_content_P(client, populatescanwifiJS, strlen_P(populatescanwifiJS));
    webserver_send_content_P(client, changewifissidJS, strlen_P(changewifissidJS));
    webserver_send_content_P(client, webFooter, strlen_P(webFooter));
  }

  return 0;
}

int handleWifiScan(struct webserver_t *client) {
#if defined(ESP32) 
  //first get result from previous scan
  int numSSID = WiFi.scanComplete();
  if (numSSID > 0) { 
    getWifiScanResults(numSSID);
  }
#endif
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"application/json", 0);
    char *str = (char *)wifiJsonList.c_str();
    webserver_send_content(client, str, strlen(str));
  }
  //initatie a new async scan for next try
#if defined(ESP8266)
  WiFi.scanNetworksAsync(getWifiScanResults);
#elif defined(ESP32)
  WiFi.scanNetworks(true);
#endif
  return 0;
}

int handleDebug(struct webserver_t *client, char *hex, byte hex_len) {
  {
#define LOGHEXBYTESPERLINE 32
    char log_msg[254];
    for (int i = 0; i < hex_len; i += LOGHEXBYTESPERLINE) {
      char buffer [(LOGHEXBYTESPERLINE * 3) + 1];
      buffer[LOGHEXBYTESPERLINE * 3] = '\0';
      for (int j = 0; ((j < LOGHEXBYTESPERLINE) && ((i + j) < hex_len)); j++) {
        sprintf(&buffer[3 * j], PSTR("%02X "), hex[i + j]);
      }
      uint8_t len = sprintf_P(log_msg, PSTR("[%03d]: %s\n"), i, buffer);
      webserver_send_content(client, log_msg, len);
    }
  }
  return 0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED: {
      } break;
    case WStype_TEXT:
      break;
    case WStype_BIN:
      break;
    case WStype_PONG: {
      } break;
    default:
      break;
  }
}

int handleRoot(struct webserver_t *client, float readpercentage, int mqttReconnects, settingsStruct *heishamonSettings) {
  switch (client->content) {
    case 0: {
        webserver_send(client, 200, (char *)"text/html", 0);
        webserver_send_content_P(client, webHeader, strlen_P(webHeader));
        webserver_send_content_P(client, webCSS, strlen_P(webCSS));
        webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
        webserver_send_content_P(client, webBodyRoot1, strlen_P(webBodyRoot1));
      } break;
    case 1: {
        webserver_send_content_P(client, heishamon_version, strlen_P(heishamon_version));
        webserver_send_content_P(client, webBodyRoot2, strlen_P(webBodyRoot2));
        if (heishamonSettings->use_1wire) {
          webserver_send_content_P(client, webBodyRootDallasTab, strlen_P(webBodyRootDallasTab));
        }
        if (heishamonSettings->use_s0) {
          webserver_send_content_P(client, webBodyRootS0Tab, strlen_P(webBodyRootS0Tab));
        }
        if (heishamonSettings->opentherm) {
          webserver_send_content_P(client, webBodyRootOpenthermTab, strlen_P(webBodyRootOpenthermTab));
        }
        webserver_send_content_P(client, webBodyRootConsoleTab, strlen_P(webBodyRootConsoleTab));
      } break;
    case 2: {
        webserver_send_content_P(client, webBodyEndDiv, strlen_P(webBodyEndDiv));
        webserver_send_content_P(client, webBodyRootStatusWifi, strlen_P(webBodyRootStatusWifi));
        char str[200];
        itoa(getWifiQuality(), str, 10);
        webserver_send_content(client, (char *)str, strlen(str));
#ifdef ESP32
        webserver_send_content_P(client, webBodyRootStatusEthernet, strlen_P(webBodyRootStatusEthernet));
        if (ETH.phyAddr() != 0) {        
          if (ETH.connected()) {
            if (ETH.hasIP()) {
              webserver_send_content_P(client, PSTR("connected"), 9);
            } else {
              webserver_send_content_P(client, PSTR("connected - no IP"), 17);
            }
          } 
          else {
            webserver_send_content_P(client, PSTR("not connected"), 13);
          }
        } else {
          webserver_send_content_P(client, PSTR("not installed"), 13);
        }
#endif
      } break;
    case 3: {
        webserver_send_content_P(client, webBodyRootStatusMemory, strlen_P(webBodyRootStatusMemory));
        char str[200];
        itoa(getFreeMemory(), str, 10);
        webserver_send_content(client, (char *)str, strlen(str));
        webserver_send_content_P(client, webBodyRootStatusReceived, strlen_P(webBodyRootStatusReceived));
        str[200];
        itoa(readpercentage, str, 10);
        webserver_send_content(client, (char *)str, strlen(str));
      } break;
    case 4: {
        webserver_send_content_P(client, webBodyRootStatusReconnects, strlen_P(webBodyRootStatusReconnects));
        char str[200];
        itoa(mqttReconnects, str, 10);
        webserver_send_content(client, (char *)str, strlen(str));
        webserver_send_content_P(client, webBodyRootStatusUptime, strlen_P(webBodyRootStatusUptime));
        char *up = getUptime();
        webserver_send_content(client, up, strlen(up));
        free(up);
        if (heishamonSettings->listenonly) {
          webserver_send_content_P(client, webBodyRootStatusListenOnly, strlen_P(webBodyRootStatusListenOnly));
        }
      } break;
    case 5: {
        webserver_send_content_P(client, webBodyEndDiv, strlen_P(webBodyEndDiv));
        webserver_send_content_P(client, webBodyRootHeatpumpValues, strlen_P(webBodyRootHeatpumpValues));
        if (heishamonSettings->use_1wire) {
          webserver_send_content_P(client, webBodyRootDallasValues, strlen_P(webBodyRootDallasValues));
        }
        if (heishamonSettings->use_s0) {
          webserver_send_content_P(client, webBodyRootS0Values, strlen_P(webBodyRootS0Values));
        }
        if (heishamonSettings->opentherm) {
          webserver_send_content_P(client, webBodyRootOpenthermValues, strlen_P(webBodyRootOpenthermValues));
        }
        webserver_send_content_P(client, webBodyRootConsole, strlen_P(webBodyRootConsole));
        webserver_send_content_P(client, menuJS, strlen_P(menuJS));
      } break;
    case 6: {
        webserver_send_content_P(client, refreshJS, strlen_P(refreshJS));
        webserver_send_content_P(client, selectJS, strlen_P(selectJS));
        webserver_send_content_P(client, websocketJS, strlen_P(websocketJS));
        webserver_send_content_P(client, webFooter, strlen_P(webFooter));
      } break;
  }
  return 0;
}

int handleTableRefresh(struct webserver_t *client, char* actData, char* actDataExtra, bool extraDataBlockAvailable) {
  int ret = 0;
  int extraTopics = extraDataBlockAvailable ? NUMBER_OF_TOPICS_EXTRA : 0; //set to 0 if there is no datablock so we don't run table data for it
  if (client->route == 11) {
    if (client->content == 0) {
      webserver_send(client, 200, (char *)"text/html", 0);
      dallasTableOutput(client);
    }
  } else if (client->route == 12) {
    if (client->content == 0) {
      webserver_send(client, 200, (char *)"text/html", 0);
      s0TableOutput(client);
    }
  } else if (client->route == 13) {
    if (client->content == 0) {
      webserver_send(client, 200, (char *)"text/html", 0);
      openthermTableOutput(client);
    }
  } else if (client->route == 10) {
    if (client->content == 0) {
      webserver_send(client, 200, (char *)"text/html", 0);
    }
    if (client->content < NUMBER_OF_TOPICS) {
      for (uint8_t topic = client->content; topic < NUMBER_OF_TOPICS && topic < client->content + 4; topic++) {

        webserver_send_content_P(client, PSTR("<tr><td>TOP"), 11);

        char str[12];
        itoa(topic, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR("</td><td>"), 9);
        webserver_send_content_P(client, topics[topic], strlen_P(topics[topic]));
        webserver_send_content_P(client, PSTR("</td><td>"), 9);

        {
          String dataValue = actData[0] == '\0' ? "" : getDataValue(actData, topic);
          char* str = (char *)dataValue.c_str();
          webserver_send_content(client, str, strlen(str));
        }

        webserver_send_content_P(client, PSTR("</td><td>"), 9);

        int maxvalue = atoi(topicDescription[topic][0]);
        int value = actData[0] == '\0' ? 0 : getDataValue(actData, topic).toInt();
        if (maxvalue == 0) { //this takes the special case where the description is a real value description instead of a mode, so value should take first index (= 0 + 1)
          value = 0;
        }
        if ((value < 0) || (value > maxvalue)) {
          webserver_send_content_P(client, _unknown, strlen_P(_unknown));
        }
        else {
          webserver_send_content_P(client, topicDescription[topic][value + 1], strlen_P(topicDescription[topic][value + 1]));

        }

        webserver_send_content_P(client, PSTR("</td></tr>"), 10);
        client->content++;
      }
      client->content--; // The webserver also increases by 1
    } else if (client->content - NUMBER_OF_TOPICS < extraTopics ) {
      for (uint8_t topic = client->content  - NUMBER_OF_TOPICS ; topic < extraTopics && topic < (client->content - NUMBER_OF_TOPICS + 4 ); topic++) {

        webserver_send_content_P(client, PSTR("<tr><td>XTOP"), 12);

        char str[12];
        itoa(topic, str, 10);
        webserver_send_content(client, str, strlen(str));

        webserver_send_content_P(client, PSTR("</td><td>"), 9);
        webserver_send_content_P(client, xtopics[topic], strlen_P(xtopics[topic]));
        webserver_send_content_P(client, PSTR("</td><td>"), 9);

        {
          String dataValue = actData[0] == '\0' ? "" : getDataValueExtra(actDataExtra, topic);
          char* str = (char *)dataValue.c_str();
          webserver_send_content(client, str, strlen(str));
        }

        webserver_send_content_P(client, PSTR("</td><td>"), 9);

        int maxvalue = atoi(xtopicDescription[topic][0]);
        int value = actData[0] == '\0' ? 0 : getDataValueExtra(actDataExtra, topic).toInt();
        if (maxvalue == 0) { //this takes the special case where the description is a real value description instead of a mode, so value should take first index (= 0 + 1)
          value = 0;
        }
        if ((value < 0) || (value > maxvalue)) {
          webserver_send_content_P(client, _unknown, strlen_P(_unknown));
        }
        else {
          webserver_send_content_P(client, xtopicDescription[topic][value + 1], strlen_P(xtopicDescription[topic][value + 1]));

        }

        webserver_send_content_P(client, PSTR("</td></tr>"), 10);
        client->content++;
      }
      client->content--; // The webserver also increases by 1
    }

  }
  return 0;
}



int handleJsonOutput(struct webserver_t *client, char* actData, char* actDataExtra, settingsStruct *heishamonSettings, bool extraDataBlockAvailable) {
  int extraTopics = extraDataBlockAvailable ? NUMBER_OF_TOPICS_EXTRA : 0; //set to 0 if there is no datablock so we don't run json data for it
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"application/json", 0);
    webserver_send_content_P(client, PSTR("{\"heatpump\":["), 13);
  } else if ((client->content - 1) < NUMBER_OF_TOPICS) {
    for (uint8_t topic = client->content - 1; topic < NUMBER_OF_TOPICS && topic < client->content + 4 ; topic++) {  //5 TOPS per webserver run (because content was 1 at start, so makes 5)

      webserver_send_content_P(client, PSTR("{\"Topic\":\"TOP"), 13);

      {
        char str[12];
        itoa(topic, str, 10);
        webserver_send_content(client, str, strlen(str));
      }

      webserver_send_content_P(client, PSTR("\",\"Name\":\""), 10);

      webserver_send_content_P(client, topics[topic], strlen_P(topics[topic]));

      if (topic != 44) { //ERROR topic #44 is only one to be a string value
        webserver_send_content_P(client, PSTR("\",\"Value\":"), 10);
      }
      else {
        webserver_send_content_P(client, PSTR("\",\"Value\":\""), 11);
      }

      {
        String dataValue = getDataValue(actData, topic);
        char* str = (char *)dataValue.c_str();
        webserver_send_content(client, str, strlen(str));
      }

      if (topic != 44) { //ERROR topic #44 is only one to be a string value
        webserver_send_content_P(client, PSTR(",\"Description\":\""), 16);
      } else {
        webserver_send_content_P(client, PSTR("\",\"Description\":\""), 17);
      }

      int maxvalue = atoi(topicDescription[topic][0]);
      int value = actData[0] == '\0' ? 0 : getDataValue(actData, topic).toInt();
      if (maxvalue == 0) { //this takes the special case where the description is a real value description instead of a mode, so value should take first index (= 0 + 1)
        value = 0;
      }
      if ((value < 0) || (value > maxvalue)) {
        webserver_send_content_P(client, _unknown, strlen_P(_unknown));
      }
      else {
        webserver_send_content_P(client, topicDescription[topic][value + 1], strlen_P(topicDescription[topic][value + 1]));
      }

      webserver_send_content_P(client, PSTR("\"}"), 2);

      if (topic < NUMBER_OF_TOPICS - 1) {
        webserver_send_content_P(client, PSTR(","), 1);
      }
      client->content++;
    }
    client->content--; // The webserver also increases by 1
  } else if ((client->content - NUMBER_OF_TOPICS - 1) < extraTopics) {
    if (client->content == NUMBER_OF_TOPICS + 1) {
      webserver_send_content_P(client, PSTR("],\"heatpump extra\":["), 20);
    }
    for (uint8_t topic = (client->content - NUMBER_OF_TOPICS - 1); topic < extraTopics && topic < (client->content - NUMBER_OF_TOPICS + 4) ; topic++) {

      webserver_send_content_P(client, PSTR("{\"Topic\":\"XTOP"), 14);

      {
        char str[12];
        itoa(topic, str, 10);
        webserver_send_content(client, str, strlen(str));
      }

      webserver_send_content_P(client, PSTR("\",\"Name\":\""), 10);

      webserver_send_content_P(client, xtopics[topic], strlen_P(xtopics[topic]));

      webserver_send_content_P(client, PSTR("\",\"Value\":\""), 11);

      {
        String dataValue = getDataValueExtra(actDataExtra, topic);
        char* str = (char *)dataValue.c_str();
        webserver_send_content(client, str, strlen(str));
      }

      webserver_send_content_P(client, PSTR("\",\"Description\":\""), 17);

      int maxvalue = atoi(xtopicDescription[topic][0]);
      int value = actDataExtra[0] == '\0' ? 0 : getDataValueExtra(actDataExtra, topic).toInt();
      if (maxvalue == 0) { //this takes the special case where the description is a real value description instead of a mode, so value should take first index (= 0 + 1)
        value = 0;
      }
      if ((value < 0) || (value > maxvalue)) {
        webserver_send_content_P(client, _unknown, strlen_P(_unknown));
      }
      else {
        webserver_send_content_P(client, xtopicDescription[topic][value + 1], strlen_P(xtopicDescription[topic][value + 1]));
      }

      webserver_send_content_P(client, PSTR("\"}"), 2);

      if (topic < (extraTopics - 1)) {
        webserver_send_content_P(client, PSTR(","), 1);
      }
      client->content++;
    }
    client->content--; // The webserver also increases by 1
  } else if (client->content == (NUMBER_OF_TOPICS + extraTopics + 1)) {
    webserver_send_content_P(client, PSTR("]"), 1);
    if (heishamonSettings->use_1wire) {
      webserver_send_content_P(client, PSTR(",\"1wire\":"), 9);
      dallasJsonOutput(client);
    }
    if (heishamonSettings->use_s0 ) {
      webserver_send_content_P(client, PSTR(",\"s0\":"), 6);
      s0JsonOutput(client);
    }
    if (heishamonSettings->opentherm) {
      webserver_send_content_P(client, PSTR(",\"opentherm\":"), 13);
      openthermJsonOutput(client);
    }
    webserver_send_content_P(client, PSTR("}"), 1);
  }
  return 0;
}


int showRules(struct webserver_t *client) {
  uint16_t len = 0, len1 = 0;

  if (client->content == 0) {
    webserver_send(client, 200, (char *)"text/html", 0);
    webserver_send_content_P(client, webHeader, strlen_P(webHeader));
    webserver_send_content_P(client, webCSS, strlen_P(webCSS));
    webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
    webserver_send_content_P(client, showRulesPage1, strlen_P(showRulesPage1));
    if (LittleFS.begin()) {
      client->userdata = new fs::File(LittleFS.open("/rules.txt", "r"));
    }
  } else if (client->userdata != NULL) {
#define BUFFER_SIZE 128
    File *f = (File *)client->userdata;
    char content[BUFFER_SIZE];
    memset(content, 0, BUFFER_SIZE);
    if (f && *f) {
      len = f->size();
    }

    if (len > 0) {
      f->seek((client->content - 1)*BUFFER_SIZE, SeekSet);
      if (client->content * BUFFER_SIZE <= len) {
        f->readBytes(content, BUFFER_SIZE);
        len1 = BUFFER_SIZE;
      } else if ((client->content * BUFFER_SIZE) >= len && (client->content * BUFFER_SIZE) <= len + BUFFER_SIZE) {
        f->readBytes(content, len - ((client->content - 1)*BUFFER_SIZE));
        len1 = len - ((client->content - 1) * BUFFER_SIZE);
      } else {
        len1 = 0;
      }

      if (len1 > 0) {
        webserver_send_content(client, content, len1);
        if (len1 < BUFFER_SIZE || client->content * BUFFER_SIZE == len) {
          if (f) {
            if (*f) {
              f->close();
            } 
            delete f;
          }
          client->userdata = NULL;
          webserver_send_content_P(client, showRulesPage2, strlen_P(showRulesPage2));
          webserver_send_content_P(client, menuJS, strlen_P(menuJS));
          webserver_send_content_P(client, webFooter, strlen_P(webFooter));
        }
      } else if (client->content == 1) {
        if (f) {
          if (*f) {
            f->close();
          }
          delete f;
        }
        client->userdata = NULL;
        webserver_send_content_P(client, showRulesPage2, strlen_P(showRulesPage2));
        webserver_send_content_P(client, menuJS, strlen_P(menuJS));
        webserver_send_content_P(client, webFooter, strlen_P(webFooter));
      }
    } else if (client->content == 1) {
      if (f) {
        if (*f) {
          f->close();
        }
        delete f;
      }
      client->userdata = NULL;
      webserver_send_content_P(client, showRulesPage2, strlen_P(showRulesPage2));
      webserver_send_content_P(client, menuJS, strlen_P(menuJS));
      webserver_send_content_P(client, webFooter, strlen_P(webFooter));
    }
  } else if (client->content == 1) {
    webserver_send_content_P(client, showRulesPage2, strlen_P(showRulesPage2));
    webserver_send_content_P(client, menuJS, strlen_P(menuJS));
    webserver_send_content_P(client, webFooter, strlen_P(webFooter));
  }

  return 0;
}

int showFirmware(struct webserver_t *client) {
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"text/html", 0);
    webserver_send_content_P(client, webHeader, strlen_P(webHeader));
    webserver_send_content_P(client, webCSS, strlen_P(webCSS));
    webserver_send_content_P(client, webBodyStart, strlen_P(webBodyStart));
  } else  if (client->content == 1) {
    webserver_send_content_P(client, showFirmwarePage, strlen_P(showFirmwarePage));
    webserver_send_content_P(client, menuJS, strlen_P(menuJS));
    webserver_send_content_P(client, webFooter, strlen_P(webFooter));
  }

  return 0;
}

int showFirmwareSuccess(struct webserver_t *client) {
  if (client->content == 0) {
    webserver_send(client, 200, (char *)"text/html", strlen_P(firmwareSuccessResponse));
    webserver_send_content_P(client, firmwareSuccessResponse, strlen_P(firmwareSuccessResponse));
  }
  return 0;
}

static void printUpdateError(char **out, uint8_t size) {
  uint8_t len = 0;
  len = snprintf_P(*out, size, PSTR("ERROR[%u]: "), Update.getError());
  if (Update.getError() == UPDATE_ERROR_OK) {
    snprintf_P(&(*out)[len], size - len, PSTR("No Error"));
  } else if (Update.getError() == UPDATE_ERROR_WRITE) {
    snprintf_P(&(*out)[len], size - len, PSTR("Flash Write Failed"));
  } else if (Update.getError() == UPDATE_ERROR_ERASE) {
    snprintf_P(&(*out)[len], size - len, PSTR("Flash Erase Failed"));
  } else if (Update.getError() == UPDATE_ERROR_READ) {
    snprintf_P(&(*out)[len], size - len, PSTR("Flash Read Failed"));
  } else if (Update.getError() == UPDATE_ERROR_SPACE) {
    snprintf_P(&(*out)[len], size - len, PSTR("Not Enough Space"));
  } else if (Update.getError() == UPDATE_ERROR_SIZE) {
    snprintf_P(&(*out)[len], size - len, PSTR("Bad Size Given"));
  } else if (Update.getError() == UPDATE_ERROR_STREAM) {
    snprintf_P(&(*out)[len], size - len, PSTR("Stream Read Timeout"));
#ifdef UPDATE_ERROR_NO_DATA
  } else if (Update.getError() == UPDATE_ERROR_NO_DATA) {
    snprintf_P(&(*out)[len], size - len, PSTR("No data supplied"));
#endif
  } else if (Update.getError() == UPDATE_ERROR_MD5) {
    snprintf_P(&(*out)[len], size - len, PSTR("MD5 Failed\n"));
#if defined(ESP8266)
  } else if (Update.getError() == UPDATE_ERROR_SIGN) {
    snprintf_P(&(*out)[len], size - len, PSTR("Signature verification failed"));
  } else if (Update.getError() == UPDATE_ERROR_FLASH_CONFIG) {
    snprintf_P(&(*out)[len], size - len, PSTR("Flash config wrong real: %d IDE: %d\n"), ESP.getFlashChipRealSize(), ESP.getFlashChipSize());
  } else if (Update.getError() == UPDATE_ERROR_NEW_FLASH_CONFIG) {
    snprintf_P(&(*out)[len], size - len, PSTR("new Flash config wrong real: %d\n"), ESP.getFlashChipRealSize());
  } else if (Update.getError() == UPDATE_ERROR_MAGIC_BYTE) {
    snprintf_P(&(*out)[len], size - len, PSTR("Magic byte is wrong, not 0xE9"));
  } else if (Update.getError() == UPDATE_ERROR_BOOTSTRAP) {
    snprintf_P(&(*out)[len], size - len, PSTR("Invalid bootstrapping state, reset ESP8266 before updating"));
  } else {
    snprintf_P(&(*out)[len], size - len, PSTR("UNKNOWN"));
  }
}
#elif defined(ESP32)
  } else if (Update.getError() == UPDATE_ERROR_MAGIC_BYTE) {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("Wrong Magic Byte, not 0xE9"));   //####ESP32
  } else if (Update.getError() == UPDATE_ERROR_ACTIVATE) {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("Could Not Activate The Firmwaren"));   //####ESP32
  } else if (Update.getError() == UPDATE_ERROR_NO_PARTITION) {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("Partition Could Not be Found"));   //####ESP32
  } else if (Update.getError() == UPDATE_ERROR_BAD_ARGUMENT) {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("Bad Argument"));   //####ESP32
  } else if (Update.getError() == UPDATE_ERROR_ABORT) {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("Aborted , Invalid bootstrapping state, reset ESP32 before updating"));   //####ESP32
  } else {   //####ESP32
    snprintf_P(&(*out)[len], size - len, PSTR("UNKNOWN"));   //####ESP32
  }
}
#endif


int showFirmwareFail(struct webserver_t *client) {
  if (client->content == 0) {
    char str[255] = { '\0' }, *p = str;
    printUpdateError(&p, sizeof(str));

    webserver_send(client, 200, (char *)"text/html", strlen_P(firmwareFailResponse) + strlen(str));
    webserver_send_content_P(client, firmwareFailResponse, strlen_P(firmwareFailResponse));
    webserver_send_content(client, str, strlen(str));
  }
  return 0;
}
