#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include "webfunctions.h"
#include "decode.h"
#include "version.h"
#include "htmlcode.h"
#include <LittleFS.h>
#include "commands.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define UPTIME_OVERFLOW 4294967295 // Uptime overflow value

void log_message(char* string);


int getWifiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

int getFreeMemory() {
  //store total memory at boot time
  static uint32_t total_memory = 0;
  if ( 0 == total_memory ) total_memory = ESP.getFreeHeap();

  uint32_t free_memory   = ESP.getFreeHeap();
  return (100 * free_memory / total_memory ) ; // as a %
}

// returns system uptime in seconds
String getUptime() {
  static uint32_t last_uptime      = 0;
  static uint8_t  uptime_overflows = 0;

  if (millis() < last_uptime) {
    ++uptime_overflows;
  }
  last_uptime             = millis();
  uint32_t t = uptime_overflows * (UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

  char     uptime[200];
  uint8_t  d   = t / 86400L;
  uint8_t  h   = ((t % 86400L) / 3600L) % 60;
  uint32_t rem = t % 3600L;
  uint8_t  m   = rem / 60;
  uint8_t  sec = rem % 60;
  sprintf_P(uptime, PSTR("%d day%s %d hour%s %d minute%s %d second%s"), d, (d == 1) ? "" : "s", h, (h == 1) ? "" : "s", m, (m == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
  return String(uptime);
}

void setupWifi(DoubleResetDetect &drd, settingsStruct *heishamonSettings) {

  //first get total memory before we do anything
  getFreeMemory();

  //set boottime
  getUptime();

  if (drd.detect()) {
    log_message("Double reset detected, clearing config.");
    Serial1.begin(115200);
    LittleFS.begin();
    LittleFS.format();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    log_message("Config cleared. Please reset to configure this device...");
    //initiate debug led indication for factory reset
    pinMode(2, FUNCTION_0); //set it as gpio
    pinMode(2, OUTPUT);
    while (true) {
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
      delay(100);
    }

  } else {
    //read configuration from FS json
    log_message("mounting FS...");

    if (LittleFS.begin()) {
      log_message((char *)"mounted file system");
      if (LittleFS.exists("/config.json")) {
        //file exists, reading and loading
        log_message((char *)"reading config file");
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          log_message((char *)"opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument jsonDoc(1024);
          DeserializationError error = deserializeJson(jsonDoc, buf.get());
          char log_msg[512];
          serializeJson(jsonDoc, log_msg);
          log_message(log_msg);
          if (!error) {
            log_message((char *)"\nparsed json");
            //read updated parameters, make sure no overflow
            if ( jsonDoc["wifi_ssid"] ) strncpy(heishamonSettings->wifi_ssid, jsonDoc["wifi_ssid"], sizeof(heishamonSettings->wifi_ssid));
            if ( jsonDoc["wifi_password"] ) strncpy(heishamonSettings->wifi_password, jsonDoc["wifi_password"], sizeof(heishamonSettings->wifi_password));
            if ( jsonDoc["wifi_hostname"] ) strncpy(heishamonSettings->wifi_hostname, jsonDoc["wifi_hostname"], sizeof(heishamonSettings->wifi_hostname));
            if ( jsonDoc["ota_password"] ) strncpy(heishamonSettings->ota_password, jsonDoc["ota_password"], sizeof(heishamonSettings->ota_password));
            if ( jsonDoc["mqtt_topic_base"] ) strncpy(heishamonSettings->mqtt_topic_base, jsonDoc["mqtt_topic_base"], sizeof(heishamonSettings->mqtt_topic_base));
            if ( jsonDoc["mqtt_server"] ) strncpy(heishamonSettings->mqtt_server, jsonDoc["mqtt_server"], sizeof(heishamonSettings->mqtt_server));
            if ( jsonDoc["mqtt_port"] ) strncpy(heishamonSettings->mqtt_port, jsonDoc["mqtt_port"], sizeof(heishamonSettings->mqtt_port));
            if ( jsonDoc["mqtt_username"] ) strncpy(heishamonSettings->mqtt_username, jsonDoc["mqtt_username"], sizeof(heishamonSettings->mqtt_username));
            if ( jsonDoc["mqtt_password"] ) strncpy(heishamonSettings->mqtt_password, jsonDoc["mqtt_password"], sizeof(heishamonSettings->mqtt_password));
            if ( jsonDoc["use_1wire"] == "enabled" ) heishamonSettings->use_1wire = true;
            if ( jsonDoc["use_s0"] == "enabled" ) {
              heishamonSettings->use_s0 = true;
              if (jsonDoc["s0_1_gpio"]) heishamonSettings->s0Settings[0].gpiopin = jsonDoc["s0_1_gpio"];
              if (jsonDoc["s0_1_ppkwh"]) heishamonSettings->s0Settings[0].ppkwh = jsonDoc["s0_1_ppkwh"];
              if (jsonDoc["s0_1_interval"]) heishamonSettings->s0Settings[0].lowerPowerInterval = jsonDoc["s0_1_interval"];
              if (jsonDoc["s0_2_gpio"]) heishamonSettings->s0Settings[1].gpiopin = jsonDoc["s0_2_gpio"];
              if (jsonDoc["s0_2_ppkwh"]) heishamonSettings->s0Settings[1].ppkwh = jsonDoc["s0_2_ppkwh"];
              if (jsonDoc["s0_2_interval"] ) heishamonSettings->s0Settings[1].lowerPowerInterval = jsonDoc["s0_2_interval"];
            }
            if ( jsonDoc["listenonly"] == "enabled" ) heishamonSettings->listenonly = true;
            if ( jsonDoc["logMqtt"] == "enabled" ) heishamonSettings->logMqtt = true;
            if ( jsonDoc["logHexdump"] == "enabled" ) heishamonSettings->logHexdump = true;
            if ( jsonDoc["logSerial1"] == "disabled" ) heishamonSettings->logSerial1 = false; //default is true so this one is different
            if ( jsonDoc["optionalPCB"] == "enabled" ) heishamonSettings->optionalPCB = true;
            if ( jsonDoc["waitTime"]) heishamonSettings->waitTime = jsonDoc["waitTime"];
            if (heishamonSettings->waitTime < 5) heishamonSettings->waitTime = 5;
            if ( jsonDoc["waitDallasTime"]) heishamonSettings->waitDallasTime = jsonDoc["waitDallasTime"];
            if (heishamonSettings->waitDallasTime < 5) heishamonSettings->waitDallasTime = 5;
            if ( jsonDoc["updateAllTime"]) heishamonSettings->updateAllTime = jsonDoc["updateAllTime"];
            if (heishamonSettings->updateAllTime < heishamonSettings->waitTime) heishamonSettings->updateAllTime = heishamonSettings->waitTime;
            if ( jsonDoc["updataAllDallasTime"]) heishamonSettings->updataAllDallasTime = jsonDoc["updataAllDallasTime"];
            if (heishamonSettings->updataAllDallasTime < heishamonSettings->waitDallasTime) heishamonSettings->updataAllDallasTime = heishamonSettings->waitDallasTime;
          } else {
            log_message("Failed to load json config, forcing config reset.");
            WiFi.persistent(true);
            WiFi.disconnect();
            WiFi.persistent(false);
          }
          configFile.close();
        }
      }
      else {
        log_message("No config.json exists! Forcing a config reset.");
        WiFi.persistent(true);
        WiFi.disconnect();
        WiFi.persistent(false);
      }

      if (LittleFS.exists("/heatcurve.json")) {
        //file exists, reading and loading
        log_message("reading heatingcurve file");
        File configFile = LittleFS.open("/heatcurve.json", "r");
        if (configFile) {
          log_message("opened heating curve config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument jsonDoc(1024);
          DeserializationError error = deserializeJson(jsonDoc, buf.get());
          serializeJson(jsonDoc, Serial);
          if (!error) {
            if ( jsonDoc["enableHeatCurve"] == "enabled" ) heishamonSettings->SmartControlSettings.enableHeatCurve = true;
            if ( jsonDoc["avgHourHeatCurve"]) heishamonSettings->SmartControlSettings.avgHourHeatCurve = jsonDoc["avgHourHeatCurve"];
            if ( jsonDoc["heatCurveTargetHigh"]) heishamonSettings->SmartControlSettings.heatCurveTargetHigh = jsonDoc["heatCurveTargetHigh"];
            if ( jsonDoc["heatCurveTargetLow"]) heishamonSettings->SmartControlSettings.heatCurveTargetLow = jsonDoc["heatCurveTargetLow"];
            if ( jsonDoc["heatCurveOutHigh"]) heishamonSettings->SmartControlSettings.heatCurveOutHigh = jsonDoc["heatCurveOutHigh"];
            if ( jsonDoc["heatCurveOutLow"]) heishamonSettings->SmartControlSettings.heatCurveOutLow = jsonDoc["heatCurveOutLow"];
            for (unsigned int i = 0 ; i < 36 ; i++) {
              if ( jsonDoc["heatCurveLookup"][i]) heishamonSettings->SmartControlSettings.heatCurveLookup[i] = jsonDoc["heatCurveLookup"][i];
            }
          }
          configFile.close();
        }
      }
    } else {
      log_message("failed to mount FS");
    }
    //end read
  }


  log_message((char *)"Wifi reconnecting with new configuration...");
  //no sleep wifi
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPdisconnect(true);
  if (heishamonSettings->wifi_ssid[0] != '\0') {
    log_message((char *)"Wifi client mode...");
    WiFi.persistent(true);
    if (heishamonSettings->wifi_password[0] == '\0') {
      WiFi.begin(heishamonSettings->wifi_ssid);
    } else {
      WiFi.begin(heishamonSettings->wifi_ssid, heishamonSettings->wifi_password);
    }
  }
  else {
    log_message((char *)"Wifi hotspot mode...");
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("HeishaMon-Setup");
  }

  if (heishamonSettings->wifi_hostname[0] == '\0') {
    //Set hostname on wifi rather than ESP_xxxxx
    WiFi.hostname("HeishaMon");
  } else {
    WiFi.hostname(heishamonSettings->wifi_hostname);
  }

}

void handleRoot(ESP8266WebServer *httpServer, float readpercentage, int mqttReconnects, settingsStruct *heishamonSettings) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent_P(webBodyRoot1);
  httpServer->sendContent(heishamon_version);
  httpServer->sendContent_P(webBodyRoot2);

  if (heishamonSettings->use_1wire) httpServer->sendContent_P(webBodyRootDallasTab);
  if (heishamonSettings->use_s0) httpServer->sendContent_P(webBodyRootS0Tab);
  httpServer->sendContent_P(webBodyRootConsoleTab);
  httpServer->sendContent_P(webBodyEndDiv);

  httpServer->sendContent_P(webBodyRootStatusWifi);
  httpServer->sendContent(String(getWifiQuality()));
  httpServer->sendContent_P(webBodyRootStatusMemory);
  httpServer->sendContent(String(getFreeMemory()));
  httpServer->sendContent_P(webBodyRootStatusReceived);
  httpServer->sendContent(String(readpercentage));
  httpServer->sendContent_P(webBodyRootStatusReconnects);
  httpServer->sendContent(String(mqttReconnects));
  httpServer->sendContent_P(webBodyRootStatusUptime);
  httpServer->sendContent(getUptime());
  httpServer->sendContent_P(webBodyEndDiv);

  httpServer->sendContent_P(webBodyRootHeatpumpValues);
  if (heishamonSettings->use_1wire)httpServer->sendContent_P(webBodyRootDallasValues);
  if (heishamonSettings->use_s0)  httpServer->sendContent_P(webBodyRootS0Values);
  httpServer->sendContent_P(webBodyRootConsole);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(refreshJS);
  httpServer->sendContent_P(selectJS);
  httpServer->sendContent_P(websocketJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleTableRefresh(ESP8266WebServer *httpServer, String actData[]) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  if (httpServer->hasArg("1wire")) {
    httpServer->sendContent(dallasTableOutput());
  } else if (httpServer->hasArg("s0")) {
    httpServer->sendContent(s0TableOutput());
  } else {
    for (unsigned int topic = 0 ; topic < NUMBER_OF_TOPICS ; topic++) {
      String topicdesc;
      const char *valuetext = "value";
      if (strcmp_P(valuetext, topicDescription[topic][0]) == 0) {
        topicdesc = topicDescription[topic][1];
      }
      else {
        int value = actData[topic].toInt();
        int maxvalue = atoi(topicDescription[topic][0]);
        if ((value < 0) || (value > maxvalue)) {
          topicdesc = "unknown";
        }
        else {
          topicdesc = topicDescription[topic][value + 1]; //plus one, because 0 is the maxvalue container
        }
      }
      String tabletext = "<tr>";
      tabletext = tabletext + "<td>TOP" + topic + "</td>";
      tabletext = tabletext + "<td>" + topics[topic] + "</td>";
      tabletext = tabletext + "<td>" + actData[topic] + "</td>";
      tabletext = tabletext + "<td>" + topicdesc + "</td>";
      tabletext = tabletext + "</tr>";
      httpServer->sendContent(tabletext);
    }
  }
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleJsonOutput(ESP8266WebServer *httpServer, String actData[]) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->sendHeader("Access-Control-Allow-Origin", "*");
  httpServer->send(200, "application/json", "");
  //begin json
  String tabletext = F("{");
  //heatpump values in json
  tabletext = tabletext + F("\"heatpump\":[");
  httpServer->sendContent(tabletext);
  for (unsigned int topic = 0 ; topic < NUMBER_OF_TOPICS ; topic++) {
    String topicdesc;
    const char *valuetext = "value";
    if (strcmp_P(valuetext, topicDescription[topic][0]) == 0) {
      topicdesc = topicDescription[topic][1];
    }
    else {
      int value = actData[topic].toInt();
      int maxvalue = atoi(topicDescription[topic][0]);
      if ((value < 0) || (value > maxvalue)) {
        topicdesc = "unknown";
      }
      else {
        topicdesc = topicDescription[topic][value + 1]; //plus one, because 0 is the maxvalue container
      }
    }
    tabletext = F("{");
    tabletext = tabletext + F("\"Topic\": \"TOP") + topic + F("\",");
    tabletext = tabletext + F("\"Name\": \"") + topics[topic] + F("\",");
    tabletext = tabletext + F("\"Value\": \"") + actData[topic] + F("\",");
    tabletext = tabletext + F("\"Description\": \"") + topicdesc + F("\"");
    tabletext = tabletext + F("}");
    if (topic < NUMBER_OF_TOPICS - 1) {
      tabletext = tabletext + F(",");
    }
    httpServer->sendContent(tabletext);
  }
  tabletext = F("]");
  httpServer->sendContent(tabletext);
  //1wire data in json
  tabletext = F(",\"1wire\":");
  tabletext = tabletext + dallasJsonOutput();
  httpServer->sendContent(tabletext);
  //s0 data in json
  tabletext = F(",\"s0\":");
  tabletext = tabletext + s0JsonOutput();
  httpServer->sendContent(tabletext);
  //end json string
  tabletext = F("}");
  httpServer->sendContent(tabletext);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleFactoryReset(ESP8266WebServer *httpServer) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(refreshMeta);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent_P(webBodyFactoryResetWarning);
  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
  delay(1000);
  LittleFS.begin();
  LittleFS.format();
  WiFi.disconnect(true);
  delay(1000);
  ESP.restart();
}

void handleReboot(ESP8266WebServer *httpServer) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(refreshMeta);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent_P(webBodyRebootWarning);
  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
  delay(1000);
  ESP.restart();
}

void settingsToJson(DynamicJsonDocument &jsonDoc, settingsStruct *heishamonSettings) {
  //set jsonDoc with current settings
  jsonDoc["wifi_hostname"] = heishamonSettings->wifi_hostname;
  jsonDoc["wifi_password"] = heishamonSettings->wifi_password;
  jsonDoc["wifi_ssid"] = heishamonSettings->wifi_ssid;
  jsonDoc["ota_password"] = heishamonSettings->ota_password;
  jsonDoc["mqtt_topic_base"] = heishamonSettings->mqtt_topic_base;
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
  jsonDoc["waitTime"] = heishamonSettings->waitTime;
  jsonDoc["waitDallasTime"] = heishamonSettings->waitDallasTime;
  jsonDoc["updateAllTime"] = heishamonSettings->updateAllTime;
  jsonDoc["updataAllDallasTime"] = heishamonSettings->updataAllDallasTime;
}

void saveJsonToConfig(DynamicJsonDocument &jsonDoc) {
  if (LittleFS.begin()) {
    File configFile = LittleFS.open("/config.json", "w");
    if (configFile) {
      serializeJson(jsonDoc, configFile);
      configFile.close();
    }
  }
}

void handleSettings(DoubleResetDetect &drd, ESP8266WebServer *httpServer, settingsStruct *heishamonSettings) {
  //check if POST was made with save settings, if yes then save and reboot
  if (httpServer->args()) {
    bool reconnectWiFi = false;
    DynamicJsonDocument jsonDoc(1024);

    settingsToJson(jsonDoc, heishamonSettings); //stores current settings in a json document

    //then overwrite with new settings
    if (httpServer->hasArg("wifi_hostname")) {
      jsonDoc["wifi_hostname"] = httpServer->arg("wifi_hostname");
    }
    if (httpServer->hasArg("wifi_ssid") && httpServer->hasArg("wifi_password")) {
      if (strcmp(jsonDoc["wifi_ssid"], httpServer->arg("wifi_ssid").c_str()) != 0 || strcmp(jsonDoc["wifi_password"], httpServer->arg("wifi_password").c_str()) != 0) {
        reconnectWiFi = true;
      }
    }
    if (httpServer->hasArg("wifi_ssid")) {
      jsonDoc["wifi_ssid"] = httpServer->arg("wifi_ssid").c_str();
    }
    if (httpServer->hasArg("wifi_password")) {
      jsonDoc["wifi_password"] = httpServer->arg("wifi_password").c_str();
    }
    if (httpServer->hasArg("new_ota_password") && (httpServer->arg("new_ota_password") != NULL) && (httpServer->arg("current_ota_password") != NULL) ) {
      if (httpServer->hasArg("current_ota_password") && (strcmp(heishamonSettings->ota_password, httpServer->arg("current_ota_password").c_str()) == 0 )) {
        jsonDoc["ota_password"] = httpServer->arg("new_ota_password");
      }
      else {
        httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
        httpServer->send(200, "text/html", "");
        httpServer->sendContent_P(webHeader);
        httpServer->sendContent_P(webCSS);
        httpServer->sendContent_P(webBodyStart);
        httpServer->sendContent_P(webBodySettings1);
        httpServer->sendContent_P(webBodySettingsResetPasswordWarning);
        httpServer->sendContent_P(refreshMeta);
        httpServer->sendContent_P(webFooter);
        httpServer->sendContent("");
        httpServer->client().stop();
        return;
      }
    }
    if (httpServer->hasArg("mqtt_topic_base")) {
      jsonDoc["mqtt_topic_base"] = httpServer->arg("mqtt_topic_base");
    }
    if (httpServer->hasArg("mqtt_server")) {
      jsonDoc["mqtt_server"] = httpServer->arg("mqtt_server");
    }
    if (httpServer->hasArg("mqtt_port")) {
      jsonDoc["mqtt_port"] = httpServer->arg("mqtt_port");
    }
    if (httpServer->hasArg("mqtt_username")) {
      jsonDoc["mqtt_username"] = httpServer->arg("mqtt_username");
    }
    if (httpServer->hasArg("mqtt_password")) {
      jsonDoc["mqtt_password"] = httpServer->arg("mqtt_password");
    }
    if (httpServer->hasArg("use_1wire")) {
      jsonDoc["use_1wire"] = "enabled";
    } else {
      jsonDoc["use_1wire"] = "disabled";
    }
    if (httpServer->hasArg("use_s0")) {
      jsonDoc["use_s0"] = "enabled";
      if (httpServer->hasArg("s0_1_gpio")) jsonDoc["s0_1_gpio"] = httpServer->arg("s0_1_gpio");
      if (httpServer->hasArg("s0_1_ppkwh")) jsonDoc["s0_1_ppkwh"] = httpServer->arg("s0_1_ppkwh");
      if (httpServer->hasArg("s0_1_interval")) jsonDoc["s0_1_interval"] = httpServer->arg("s0_1_interval");
      if (httpServer->hasArg("s0_2_gpio")) jsonDoc["s0_2_gpio"] = httpServer->arg("s0_2_gpio");
      if (httpServer->hasArg("s0_2_ppkwh")) jsonDoc["s0_2_ppkwh"] = httpServer->arg("s0_2_ppkwh");
      if (httpServer->hasArg("s0_2_interval")) jsonDoc["s0_2_interval"] = httpServer->arg("s0_2_interval");
    } else {
      jsonDoc["use_s0"] = "disabled";
    }
    if (httpServer->hasArg("listenonly")) {
      jsonDoc["listenonly"] = "enabled";
    } else {
      jsonDoc["listenonly"] = "disabled";
    }
    if (httpServer->hasArg("logMqtt")) {
      jsonDoc["logMqtt"] = "enabled";
    } else {
      jsonDoc["logMqtt"] = "disabled";
    }
    if (httpServer->hasArg("logHexdump")) {
      jsonDoc["logHexdump"] = "enabled";
    } else {
      jsonDoc["logHexdump"] = "disabled";
    }
    if (httpServer->hasArg("logSerial1")) {
      jsonDoc["logSerial1"] = "enabled";
    } else {
      jsonDoc["logSerial1"] = "disabled";
    }
    if (httpServer->hasArg("optionalPCB")) {
      jsonDoc["optionalPCB"] = "enabled";
    } else {
      jsonDoc["optionalPCB"] = "disabled";
    }
    if (httpServer->hasArg("waitTime")) {
      jsonDoc["waitTime"] = httpServer->arg("waitTime");
    }
    if (httpServer->hasArg("waitDallasTime")) {
      jsonDoc["waitDallasTime"] = httpServer->arg("waitDallasTime");
    }
    if (httpServer->hasArg("updateAllTime")) {
      jsonDoc["updateAllTime"] = httpServer->arg("updateAllTime");
    }
    if (httpServer->hasArg("updataAllDallasTime")) {
      jsonDoc["updataAllDallasTime"] = httpServer->arg("updataAllDallasTime");
    }

    saveJsonToConfig(jsonDoc); //save to config file

    if (reconnectWiFi) {
      httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
      httpServer->send(200, "text/html", "");
      httpServer->sendContent_P(webHeader);
      httpServer->sendContent_P(webCSS);
      httpServer->sendContent_P(webBodyStart);
      httpServer->sendContent_P(webBodySettings1);
      httpServer->sendContent_P(webBodySettingsNewWifiWarning);
      httpServer->sendContent_P(refreshMeta);
      httpServer->sendContent_P(webFooter);
      httpServer->sendContent("");
      httpServer->client().stop();
      setupWifi(drd, heishamonSettings);
      return;
    }
  }

  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent_P(webBodySettings1);

  String httptext = F("<div class=\"w3-container w3-center\">");
  httptext = httptext + F("<h2>Settings</h2>");
  httptext = httptext + F("<form enctype=\"multipart/form-data\" accept-charset=\"UTF-8\" action=\"/settings\" method=\"POST\">");
  httptext = httptext + F("<table style=\"width:100%\">");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Hostname:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"text\" name=\"wifi_hostname\" value=\"") + heishamonSettings->wifi_hostname + F("\">");
  httptext = httptext + F("</td></tr>");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Wifi SSID:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"text\" name=\"wifi_ssid\" value=\"") + heishamonSettings->wifi_ssid + F("\">");
  httptext = httptext + F("</td></tr>");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Wifi password:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"password\" name=\"wifi_password\" value=\"") + heishamonSettings->wifi_password + F("\">");
  httptext = httptext + F("</td></tr>");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Update username:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<label name=\"username\">admin</label>");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Current update password:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"password\" name=\"current_ota_password\" value=\"\"> default password: \"heisha\"");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("New update password:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"password\" name=\"new_ota_password\" value=\"\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Mqtt topic base:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"text\" name=\"mqtt_topic_base\" value=\"") + heishamonSettings->mqtt_topic_base + F("\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Mqtt server:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"text\" name=\"mqtt_server\" value=\"") + heishamonSettings->mqtt_server + F("\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Mqtt port:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"number\" name=\"mqtt_port\" value=\"") + heishamonSettings->mqtt_port + F("\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Mqtt username:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"text\" name=\"mqtt_username\" value=\"") + heishamonSettings->mqtt_username + F("\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Mqtt password:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"password\" name=\"mqtt_password\" value=\"") + heishamonSettings->mqtt_password + F("\">");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("How often new values are collected from heatpump:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"number\" name=\"waitTime\" value=\"") + heishamonSettings->waitTime + F("\"> seconds  (min 5 sec)");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("How often all heatpump values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"number\" name=\"updateAllTime\" value=\"") + heishamonSettings->updateAllTime + F("\"> seconds");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");

  httpServer->sendContent(httptext);
  httptext = F("Listen only mode:</td><td style=\"text-align:left\">");
  if (heishamonSettings->listenonly) {
    httptext = httptext + F("<input type=\"checkbox\" name=\"listenonly\" value=\"enabled\" checked >");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" name=\"listenonly\" value=\"enabled\">");
  }
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Debug log to MQTT topic from start:</td><td style=\"text-align:left\">");
  if (heishamonSettings->logMqtt) {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logMqtt\" value=\"enabled\" checked >");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logMqtt\" value=\"enabled\">");
  }
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Debug log hexdump enable from start:</td><td style=\"text-align:left\">");
  if (heishamonSettings->logHexdump) {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logHexdump\" value=\"enabled\" checked >");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logHexdump\" value=\"enabled\">");
  }
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Debug log to serial1 (GPIO2):</td><td style=\"text-align:left\">");
  if (heishamonSettings->logSerial1) {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logSerial1\" value=\"enabled\" checked >");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" name=\"logSerial1\" value=\"enabled\">");
  }
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Emulate optional PCB:</td><td style=\"text-align:left\">");
  if (heishamonSettings->optionalPCB) {
    httptext = httptext + F("<input type=\"checkbox\" name=\"optionalPCB\" value=\"enabled\" checked >");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" name=\"optionalPCB\" value=\"enabled\">");
  }
  httptext = httptext + F("</td></tr>");
  httptext = httptext + F("</table>");

  httpServer->sendContent(httptext);

  // 1wire
  httptext = F("<table style=\"width:100%\">");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Use 1wire DS18b20:</td><td style=\"text-align:left\">");
  if (heishamonSettings->use_1wire) {
    httptext = httptext + F("<input type=\"checkbox\" onclick=\"ShowHideDallasTable(this)\" name=\"use_1wire\" value=\"enabled\" checked >");
    httptext = httptext + F("</td></tr>");
    httptext = httptext + F("</table>");
    httptext = httptext + F("<table id=\"dallassettings\" style=\"display: table; width:100%\">");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" onclick=\"ShowHideDallasTable(this)\" name=\"use_1wire\" value=\"enabled\">");
    httptext = httptext + F("</td></tr>");
    httptext = httptext + F("</table>");
    httptext = httptext + F("<table id=\"dallassettings\" style=\"display: none; width:100%\">");
  }
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("How often new values are collected from 1wire:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"number\" name=\"waitDallasTime\" value=\"") + heishamonSettings->waitDallasTime + F("\"> seconds (min 5 sec)");
  httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("How often all 1wire values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">");
  httptext = httptext + F("<input type=\"number\" name=\"updataAllDallasTime\" value=\"") + heishamonSettings->updataAllDallasTime + F("\"> seconds");
  httptext = httptext + F("</td></tr>");
  httptext = httptext + F("</table>");

  httpServer->sendContent(httptext);
  // s0
  httptext = F("<table style=\"width:100%\">");
  httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
  httptext = httptext + F("Use s0 kWh metering:</td><td style=\"text-align:left\">");
  if (heishamonSettings->use_s0) {
    httptext = httptext + F("<input type=\"checkbox\" onclick=\"ShowHideS0Table(this)\" name=\"use_s0\" value=\"enabled\" checked >");
    httptext = httptext + F("</td></tr>");
    httptext = httptext + F("</table>");
    httptext = httptext + F("<table id=\"s0settings\" style=\"display: table; width:100%\">");
  } else {
    httptext = httptext + F("<input type=\"checkbox\" onclick=\"ShowHideS0Table(this)\" name=\"use_s0\" value=\"enabled\">");
    httptext = httptext + F("</td></tr>");
    httptext = httptext + F("</table>");
    httptext = httptext + F("<table id=\"s0settings\" style=\"display: none; width:100%\">");
  }
  //begin default S0 pins hack
  if (heishamonSettings->s0Settings[0].gpiopin == 255) heishamonSettings->s0Settings[0].gpiopin = DEFAULT_S0_PIN_1;
  if (heishamonSettings->s0Settings[1].gpiopin == 255) heishamonSettings->s0Settings[1].gpiopin = DEFAULT_S0_PIN_2;
  //end default S0 pins hack
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    httptext = httptext + F("<tr><td style=\"text-align:right; width: 50%\">");
    httptext = httptext + F("S0 port ") + (i + 1) + F(" GPIO:</td><td style=\"text-align:left\">");
    httptext = httptext + F("<input type=\"number\" name=\"s0_") + (i + 1) + F("_gpio\" value=\"") + heishamonSettings->s0Settings[i].gpiopin + F("\">");
    httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    httptext = httptext + F("S0 port ") + (i + 1) + F(" imp/kwh:</td><td style=\"text-align:left\">");
    httptext = httptext + F("<input type=\"number\" id=\"s0_ppkwh_") + (i + 1) + F("\" onchange=\"changeMinWatt(") + (i + 1) + F(")\" name=\"s0_") + (i + 1) + F("_ppkwh\" value=\"") + (heishamonSettings->s0Settings[i].ppkwh) + F("\">");
    httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    httptext = httptext + F("S0 port ") + (i + 1) + F(" reporting interval during standby/low power usage:</td><td style=\"text-align:left\">");
    httptext = httptext + F("<input type=\"number\" id=\"s0_interval_") + (i + 1) + F("\" onchange=\"changeMinWatt(") + (i + 1) + F(")\" name=\"s0_") + (i + 1) + F("_interval\" value=\"") + (heishamonSettings->s0Settings[i].lowerPowerInterval) + F("\"> seconds");
    httptext = httptext + F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    httptext = httptext + F("S0 port ") + (i + 1) + F(" standby/low power usage threshold:</td><td style=\"text-align:left\"><label id=\"s0_minwatt_") + (i + 1) + F("\">") + (int) round((3600 * 1000 / heishamonSettings->s0Settings[i].ppkwh) / heishamonSettings->s0Settings[i].lowerPowerInterval) + F("</label> Watt");
    httptext = httptext + F("</td></tr>");
  }
  httptext = httptext + F("</table>");

  httptext = httptext + F("<br><br>");
  httptext = httptext + F("<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save\">");
  httptext = httptext + F("</form>");
  httptext = httptext + F("<br><a href=\"/factoryreset\" class=\"w3-red w3-button\" onclick=\"return confirm('Are you sure?')\" >Factory reset</a>");
  httptext = httptext + F("</div>");
  httpServer->sendContent(httptext);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(settingsJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleSmartcontrol(ESP8266WebServer *httpServer, settingsStruct *heishamonSettings, String actData[]) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent_P(webBodySmartcontrol1);
  httpServer->sendContent_P(webBodySmartcontrol2);

  String httptext = F("<form action=\"/smartcontrol\" method=\"POST\">");
  httpServer->sendContent(httptext);
  httpServer->sendContent_P(webBodyEndDiv);

  //Heating curve
  httpServer->sendContent_P(webBodySmartcontrolHeatingcurve1);

  //check if POST was made with save settings, if yes then save and reboot
  if (httpServer->args()) {
    DynamicJsonDocument jsonDoc(1024);
    //set jsonDoc with current settings
    if ( heishamonSettings->SmartControlSettings.enableHeatCurve) {
      jsonDoc["enableHeatCurve"] = "enabled";
    } else {
      jsonDoc["enableHeatCurve"] = "disabled";
    }
    jsonDoc["avgHourHeatCurve"] = heishamonSettings->SmartControlSettings.avgHourHeatCurve;
    jsonDoc["heatCurveTargetHigh"] = heishamonSettings->SmartControlSettings.heatCurveTargetHigh;
    jsonDoc["heatCurveTargetLow"] = heishamonSettings->SmartControlSettings.heatCurveTargetLow;
    jsonDoc["heatCurveOutHigh"] = heishamonSettings->SmartControlSettings.heatCurveOutHigh;
    jsonDoc["heatCurveOutLow"] = heishamonSettings->SmartControlSettings.heatCurveOutLow;
    for (unsigned int i = 0 ; i < 36 ; i++) {
      jsonDoc["heatCurveLookup"][i] = heishamonSettings->SmartControlSettings.heatCurveLookup[i];
    }

    //then overwrite with new settings
    if (httpServer->hasArg("heatingcurve")) {
      jsonDoc["enableHeatCurve"] = "enabled";
    } else {
      jsonDoc["enableHeatCurve"] = "disabled";
    }
    if (httpServer->hasArg("average-time")) {
      jsonDoc["avgHourHeatCurve"] = httpServer->arg("average-time");
    }
    if (httpServer->hasArg("hcth")) {
      jsonDoc["heatCurveTargetHigh"] = httpServer->arg("hcth");
    }
    if (httpServer->hasArg("hctl")) {
      jsonDoc["heatCurveTargetLow"] = httpServer->arg("hctl");
    }
    if (httpServer->hasArg("hcoh")) {
      jsonDoc["heatCurveOutHigh"] = httpServer->arg("hcoh");
    }
    if (httpServer->hasArg("hcol")) {
      jsonDoc["heatCurveOutLow"] = httpServer->arg("hcol");
    }
    if (httpServer->hasArg("lookup0")) {
      jsonDoc["heatCurveLookup"][0] = httpServer->arg("lookup0");
    }
    if (httpServer->hasArg("lookup1")) {
      jsonDoc["heatCurveLookup"][1] = httpServer->arg("lookup1");
    }
    if (httpServer->hasArg("lookup2")) {
      jsonDoc["heatCurveLookup"][2] = httpServer->arg("lookup2");
    }
    if (httpServer->hasArg("lookup3")) {
      jsonDoc["heatCurveLookup"][3] = httpServer->arg("lookup3");
    }
    if (httpServer->hasArg("lookup4")) {
      jsonDoc["heatCurveLookup"][4] = httpServer->arg("lookup4");
    }
    if (httpServer->hasArg("lookup5")) {
      jsonDoc["heatCurveLookup"][5] = httpServer->arg("lookup5");
    }
    if (httpServer->hasArg("lookup6")) {
      jsonDoc["heatCurveLookup"][6] = httpServer->arg("lookup6");
    }
    if (httpServer->hasArg("lookup7")) {
      jsonDoc["heatCurveLookup"][7] = httpServer->arg("lookup7");
    }
    if (httpServer->hasArg("lookup8")) {
      jsonDoc["heatCurveLookup"][8] = httpServer->arg("lookup8");
    }
    if (httpServer->hasArg("lookup9")) {
      jsonDoc["heatCurveLookup"][9] = httpServer->arg("lookup9");
    }
    if (httpServer->hasArg("lookup10")) {
      jsonDoc["heatCurveLookup"][10] = httpServer->arg("lookup10");
    }
    if (httpServer->hasArg("lookup11")) {
      jsonDoc["heatCurveLookup"][11] = httpServer->arg("lookup11");
    }
    if (httpServer->hasArg("lookup12")) {
      jsonDoc["heatCurveLookup"][12] = httpServer->arg("lookup12");
    }
    if (httpServer->hasArg("lookup13")) {
      jsonDoc["heatCurveLookup"][13] = httpServer->arg("lookup13");
    }
    if (httpServer->hasArg("lookup14")) {
      jsonDoc["heatCurveLookup"][14] = httpServer->arg("lookup14");
    }
    if (httpServer->hasArg("lookup15")) {
      jsonDoc["heatCurveLookup"][15] = httpServer->arg("lookup15");
    }
    if (httpServer->hasArg("lookup16")) {
      jsonDoc["heatCurveLookup"][16] = httpServer->arg("lookup16");
    }
    if (httpServer->hasArg("lookup17")) {
      jsonDoc["heatCurveLookup"][17] = httpServer->arg("lookup17");
    }
    if (httpServer->hasArg("lookup18")) {
      jsonDoc["heatCurveLookup"][18] = httpServer->arg("lookup18");
    }
    if (httpServer->hasArg("lookup19")) {
      jsonDoc["heatCurveLookup"][19] = httpServer->arg("lookup19");
    }
    if (httpServer->hasArg("lookup20")) {
      jsonDoc["heatCurveLookup"][20] = httpServer->arg("lookup20");
    }
    if (httpServer->hasArg("lookup21")) {
      jsonDoc["heatCurveLookup"][21] = httpServer->arg("lookup21");
    }
    if (httpServer->hasArg("lookup22")) {
      jsonDoc["heatCurveLookup"][22] = httpServer->arg("lookup22");
    }
    if (httpServer->hasArg("lookup23")) {
      jsonDoc["heatCurveLookup"][23] = httpServer->arg("lookup23");
    }
    if (httpServer->hasArg("lookup24")) {
      jsonDoc["heatCurveLookup"][24] = httpServer->arg("lookup24");
    }
    if (httpServer->hasArg("lookup25")) {
      jsonDoc["heatCurveLookup"][25] = httpServer->arg("lookup25");
    }
    if (httpServer->hasArg("lookup26")) {
      jsonDoc["heatCurveLookup"][26] = httpServer->arg("lookup26");
    }
    if (httpServer->hasArg("lookup27")) {
      jsonDoc["heatCurveLookup"][27] = httpServer->arg("lookup27");
    }
    if (httpServer->hasArg("lookup28")) {
      jsonDoc["heatCurveLookup"][28] = httpServer->arg("lookup28");
    }
    if (httpServer->hasArg("lookup29")) {
      jsonDoc["heatCurveLookup"][29] = httpServer->arg("lookup29");
    }
    if (httpServer->hasArg("lookup30")) {
      jsonDoc["heatCurveLookup"][30] = httpServer->arg("lookup30");
    }
    if (httpServer->hasArg("lookup31")) {
      jsonDoc["heatCurveLookup"][31] = httpServer->arg("lookup31");
    }
    if (httpServer->hasArg("lookup32")) {
      jsonDoc["heatCurveLookup"][32] = httpServer->arg("lookup32");
    }
    if (httpServer->hasArg("lookup33")) {
      jsonDoc["heatCurveLookup"][33] = httpServer->arg("lookup33");
    }
    if (httpServer->hasArg("lookup34")) {
      jsonDoc["heatCurveLookup"][34] = httpServer->arg("lookup34");
    }
    if (httpServer->hasArg("lookup35")) {
      jsonDoc["heatCurveLookup"][35] = httpServer->arg("lookup35");
    }

    if (LittleFS.begin()) {
      File configFile = LittleFS.open("/heatcurve.json", "w");
      if (configFile) {
        serializeJson(jsonDoc, configFile);
        configFile.close();
        delay(1000);

        httpServer->sendContent_P(webBodySettingsSaveMessage);
        httpServer->sendContent_P(refreshMeta);
        httpServer->sendContent_P(webFooter);
        httpServer->sendContent("");
        httpServer->client().stop();
        delay(1000);
        ESP.restart();
      }
    }
  }

  int heatingMode = actData[76].toInt();
  if (heatingMode == 1) {
    httptext = F("<div class=\"w3-row-padding\"><div class=\"w3-half\">");
    if (heishamonSettings->SmartControlSettings.enableHeatCurve == true) {
      httptext = httptext + F("<input class=\"w3-check\" type=\"checkbox\" name=\"heatingcurve\" checked>");
    } else {
      httptext = httptext + F("<input class=\"w3-check\" type=\"checkbox\" name=\"heatingcurve\">");
    }
    httptext = httptext + F("<label>Enable smart heating curve</label></div><div class=\"w3-half\"><select class=\"w3-select\" name=\"average-time\">");
    if (heishamonSettings->SmartControlSettings.avgHourHeatCurve == 0) {
      httptext = httptext + F("<option value=\"0\" selected>No average on outside temperature</option>");
    } else {
      httptext = httptext + F("<option value=\"0\">No average on outside temperature</option>");
    }
    if (heishamonSettings->SmartControlSettings.avgHourHeatCurve == 12) {
      httptext = httptext + F("<option value=\"12\" selected>Average outside temperature over last 12 hours</option>");
    } else {
      httptext = httptext + F("<option value=\"12\">Average outside temperature over last 12 hours</option>");
    }
    if (heishamonSettings->SmartControlSettings.avgHourHeatCurve == 24) {
      httptext = httptext + F("<option value=\"24\" selected>Average outside temperature over last 24 hours</option>");
    } else {
      httptext = httptext + F("<option value=\"24\">Average outside temperature over last 24 hours</option>");
    }
    if (heishamonSettings->SmartControlSettings.avgHourHeatCurve == 36) {
      httptext = httptext + F("<option value=\"36\" selected>Average outside temperature over last 36 hours</option>");
    } else {
      httptext = httptext + F("<option value=\"36\">Average outside temperature over last 36 hours</option>");
    }
    if (heishamonSettings->SmartControlSettings.avgHourHeatCurve == 48) {
      httptext = httptext + F("<option value=\"48\" selected>Average outside temperature over last 48 hours</option>");
    } else {
      httptext = httptext + F("<option value=\"48\">Average outside temperature over last 48 hours</option>");
    }
    httptext = httptext + F("</select></div></div><br><div class=\"w3-row-padding\"><div class=\"w3-half\"><label>Heating Curve Target High Temp</label>");
    httptext = httptext + F("<input class=\"w3-input w3-border\" type=\"number\" name=\"hcth\" id=\"hcth\" value=\"") + heishamonSettings->SmartControlSettings.heatCurveTargetHigh + F("\" min=\"20\" max=\"60\" required>");
    httptext = httptext + F("</div><div class=\"w3-half\"><label>Heating Curve Target Low Temp</label>");
    httptext = httptext + F("<input class=\"w3-input w3-border\" type=\"number\" name=\"hctl\" id=\"hctl\" value=\"") + heishamonSettings->SmartControlSettings.heatCurveTargetLow + F("\" min=\"20\" max=\"60\" required>");
    httptext = httptext + F("</div></div><div class=\"w3-row-padding\"><div class=\"w3-half\"><label>Heating Curve Outside High Temp</label>");
    httptext = httptext + F("<input class=\"w3-input w3-border\" type=\"number\" name=\"hcoh\" id=\"hcoh\" value=\"") + heishamonSettings->SmartControlSettings.heatCurveOutHigh + F("\" min=\"-20\" max=\"15\" required>");
    httptext = httptext + F("</div><div class=\"w3-half\"><label>Heating Curve Outside Low Temp</label>");
    httptext = httptext + F("<input class=\"w3-input w3-border\" type=\"number\" name=\"hcol\" id=\"hcol\" value=\"") + heishamonSettings->SmartControlSettings.heatCurveOutLow + F("\" min=\"-20\" max=\"15\" required>");
    httptext = httptext + F("</div></div><br><br>");
    httptext = httptext + F("<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save and reboot\">");
    httptext = httptext + F("<div class=\"w3-panel w3-red\">");
    httptext = httptext + F("<p>");
    httptext = httptext + getAvgOutsideTemp();
    httptext = httptext + F("</p>");
    httpServer->sendContent(httptext);
    httpServer->sendContent_P(webBodyEndDiv);

    httpServer->sendContent_P(webBodySmartcontrolHeatingcurveSVG);
    httpServer->sendContent_P(webBodySmartcontrolHeatingcurve2);
    httpServer->sendContent_P(webBodyEndDiv);
  } else {
    httptext = F("Heating mode must be \"direct heating\" to enable this option");
    httpServer->sendContent(httptext);
    httpServer->sendContent_P(webBodyEndDiv);
  }

  httpServer->sendContent_P(webBodyEndDiv);

  //Other example
  //  httpServer->sendContent_P(webBodySmartcontrolOtherexample);
  //  httptext = "...Loading...";
  //  httptext = httptext + "";
  //  httpServer->sendContent(httptext);
  //  httpServer->sendContent_P(webBodyEndDiv);

  httptext = "</form>";
  httpServer->sendContent(httptext);
  httpServer->sendContent_P(webBodyEndDiv);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(selectJS);
  // httpServer->sendContent_P(heatingCurveJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}

bool send_command(byte* command, int length);

void handleREST(ESP8266WebServer *httpServer, bool optionalPCB) {

  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->sendHeader("Access-Control-Allow-Origin", "*");
  httpServer->send(200, "text/plain", "");

  String httptext = "";
  if (httpServer->method() == HTTP_GET) {
    for (uint8_t i = 0; i < httpServer->args(); i++) {
      unsigned char cmd[256] = { 0 };
      char log_msg[256] = { 0 };
      unsigned int len = 0;

      for (uint8_t x = 0; x < sizeof(commands) / sizeof(commands[0]); x++) {
        if (strcmp(httpServer->argName(i).c_str(), commands[x].name) == 0) {
          len = commands[x].func((char *)httpServer->arg(i).c_str(), cmd, log_msg);
          httptext = httptext + log_msg + "\n";
          log_message(log_msg);
          send_command(cmd, len);
        }
      }
    }
    if (optionalPCB) {
      //optional commands
      for (uint8_t i = 0; i < httpServer->args(); i++) {
        unsigned char cmd[256] = { 0 };
        char log_msg[256] = { 0 };
        unsigned int len = 0;

        for (uint8_t x = 0; x < sizeof(optionalCommands) / sizeof(optionalCommands[0]); x++) {
          if (strcmp(httpServer->argName(i).c_str(), optionalCommands[x].name) == 0) {
            len = optionalCommands[x].func((char *)httpServer->arg(i).c_str(), log_msg);
            httptext = httptext + log_msg + "\n";
            log_message(log_msg);
          }
        }
      }
    }
  }

  httpServer->sendContent(httptext);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleDebug(ESP8266WebServer *httpServer, char *hex, byte hex_len) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->sendHeader("Access-Control-Allow-Origin", "*");
  httpServer->send(200, "text/plain", "");
  char log_msg[256];

#define LOGHEXBYTESPERLINE 32
  for (int i = 0; i < hex_len; i += LOGHEXBYTESPERLINE) {
    char buffer [(LOGHEXBYTESPERLINE * 3) + 1];
    buffer[LOGHEXBYTESPERLINE * 3] = '\0';
    for (int j = 0; ((j < LOGHEXBYTESPERLINE) && ((i + j) < hex_len)); j++) {
      sprintf(&buffer[3 * j], PSTR("%02X "), hex[i + j]);
    }
    sprintf_P(log_msg, PSTR("data: %s"), buffer ); httpServer->sendContent(log_msg); httpServer->sendContent("\n");
  }

  httpServer->sendContent("");
  httpServer->client().stop();
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
