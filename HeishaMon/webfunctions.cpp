#include "webfunctions.h"
#include "decode.h"
#include "version.h"
#include "htmlcode.h"
#include "commands.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define UPTIME_OVERFLOW 4294967295 // Uptime overflow value

static int numSsid = 0;

void log_message(char* string);

void getWifiScanResults(int networksFound) {
  numSsid = networksFound;
}

int dBmToQuality(int dBm) {
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

int getWifiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  return dBmToQuality(WiFi.RSSI());
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

void loadSettings(SettingsStruct *heishamonSettings) {
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
          #ifdef DEBUG_ESP_PORT
            DEBUG_ESP_PORT.println(F("Loading settings from JSON:"));
            serializeJsonPretty(jsonDoc, DEBUG_ESP_PORT);
          #endif
          heishamonSettings->fromJson(jsonDoc);
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
          heishamonSettings->SmartControlSettings.enableHeatCurve =( jsonDoc["enableHeatCurve"] == "enabled" ) ? true : false;
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

void setupWifi(SettingsStruct *heishamonSettings) {

  log_message((char *)"Wifi reconnecting with new configuration...");
  //no sleep wifi
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true);
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
    WiFi.softAP(heishamonSettings->hotspot_ssid); // Initial hotspot is always unsecured
  }

  if (heishamonSettings->wifi_hostname[0] == '\0') {
    //Set hostname on wifi rather than ESP_xxxxx
    WiFi.hostname("HeishaMon");
  } else {
    WiFi.hostname(heishamonSettings->wifi_hostname);
  }
  //initiate a wifi scan at boot to fill the wifi scan list
  WiFi.scanNetworksAsync(getWifiScanResults);
}

void handleRoot(ESP8266WebServer *httpServer, float readpercentage, int mqttReconnects, SettingsStruct *heishamonSettings) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent(Html::leftMenu(httpServer->uri()));

  // Tabs
  httpServer->sendContent_P(webBodyRootHeatpumpTab);
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

  httpServer->sendContent_P(webBodyRootHeatpumpHeader);
  heishamonSettings->show_all_topics = httpServer->hasArg(Settings::show_all_topics); 
  if (heishamonSettings->show_all_topics || heishamonSettings->selected_topics_count == 0) {
    httpServer->sendContent(F("Showing all topics. "));
    if (heishamonSettings->selected_topics_count > 0) {
      httpServer->sendContent(F("<a href='?show_selected'>Show selected topics.</a>"));
    }
  } else {
    httpServer->sendContent(F("Showing <a href='/topics'>selected topics</a>. <a href='?show_all_topics'>Show all topics.</a>"));
  }
  httpServer->sendContent_P(webBodyRootHeatpumpTable);

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

void handleTableRefresh(ESP8266WebServer *httpServer, String actData[], SettingsStruct& settings) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  if (httpServer->hasArg("1wire")) {
    httpServer->sendContent(dallasTableOutput());
  } else if (httpServer->hasArg("s0")) {
    httpServer->sendContent(s0TableOutput());
  } else {
    bool onlySelectedTopics = !settings.show_all_topics && (settings.selected_topics_count > 0);
    int i = 0;
    for (unsigned int topic = 0 ; topic < NUMBER_OF_TOPICS ; topic++) {
      if (onlySelectedTopics) {
        if (i == settings.selected_topics_count) break;
        if (topic != settings.selected_topics[i]) continue;
        i++;
      }
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

void saveJsonToConfig(DynamicJsonDocument &jsonDoc) {
  if (LittleFS.begin()) {
    File configFile = LittleFS.open("/config.json", "w");
    if (configFile) {
      #ifdef DEBUG_ESP_PORT
      DEBUG_ESP_PORT.println("Writing JSON to config file:");
      serializeJsonPretty(jsonDoc, DEBUG_ESP_PORT);
      #endif
      serializeJson(jsonDoc, configFile);
      configFile.close();
    }
  }
}

bool handleSettings(ESP8266WebServer *httpServer, SettingsStruct *heishamonSettings) {
  //check if POST was made with save settings
  if (httpServer->args()) {
    bool reconnectWiFi = false;

    DynamicJsonDocument jsonDoc(1024);
    heishamonSettings->toJson(jsonDoc);  //stores current settings in a json document

    //then overwrite with new settings
    if (httpServer->hasArg(Settings::hotspot_mode)) {
      jsonDoc[Settings::hotspot_mode] = httpServer->arg(Settings::hotspot_mode);
    }
    if (httpServer->hasArg(Settings::wifi_hostname)) {
      jsonDoc[Settings::wifi_hostname] = httpServer->arg(Settings::wifi_hostname);
    }
    if (httpServer->hasArg(Settings::wifi_ssid) && httpServer->hasArg(Settings::wifi_password)) {
      if (strcmp(jsonDoc[Settings::wifi_ssid], httpServer->arg(Settings::wifi_ssid).c_str())
        || strcmp(jsonDoc[Settings::wifi_password], httpServer->arg(Settings::wifi_password).c_str()) != 0) {
        reconnectWiFi = true;
      }
    }
    if (httpServer->hasArg(Settings::wifi_ssid)) {
      jsonDoc[Settings::wifi_ssid] = httpServer->arg(Settings::wifi_ssid).c_str();
    }
    if (httpServer->hasArg(Settings::wifi_password)) {
      jsonDoc[Settings::wifi_password] = httpServer->arg(Settings::wifi_password).c_str();
    }
    if (httpServer->hasArg("new_ota_password") && (httpServer->arg("new_ota_password") != NULL) && (httpServer->arg("current_ota_password") != NULL) ) {
      if (httpServer->hasArg("current_ota_password") && (strcmp(heishamonSettings->ota_password, httpServer->arg("current_ota_password").c_str()) == 0 )) {
        jsonDoc[Settings::ota_password] = httpServer->arg("new_ota_password");
      }
      else {
        httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
        httpServer->send(200, "text/html", "");
        httpServer->sendContent_P(webHeader);
        httpServer->sendContent_P(webCSS);
        httpServer->sendContent_P(webBodyStart);
        httpServer->sendContent(Html::leftMenu(httpServer->uri()));
        httpServer->sendContent_P(webBodySettingsResetPasswordWarning);
        httpServer->sendContent_P(refreshMeta);
        httpServer->sendContent_P(webFooter);
        httpServer->sendContent("");
        httpServer->client().stop();
        return true;
      }
    }
    if (httpServer->hasArg(Settings::mqtt_topic_base)) {
      jsonDoc[Settings::mqtt_topic_base] = httpServer->arg(Settings::mqtt_topic_base);
    }
    if (httpServer->hasArg(Settings::mqtt_server)) {
      jsonDoc[Settings::mqtt_server] = httpServer->arg(Settings::mqtt_server);
    }
    if (httpServer->hasArg(Settings::mqtt_port)) {
      jsonDoc[Settings::mqtt_port] = httpServer->arg(Settings::mqtt_port);
    }
    if (httpServer->hasArg(Settings::mqtt_username)) {
      jsonDoc[Settings::mqtt_username] = httpServer->arg(Settings::mqtt_username);
    }
    if (httpServer->hasArg(Settings::mqtt_password)) {
      jsonDoc[Settings::mqtt_password] = httpServer->arg(Settings::mqtt_password);
    }
    jsonDoc[Settings::use_1wire] = Settings::toString(httpServer->hasArg(Settings::use_1wire));
    jsonDoc[Settings::use_s0] = Settings::toString(httpServer->hasArg(Settings::use_s0));
    if (httpServer->hasArg(Settings::use_s0)) {
      if (httpServer->hasArg(Settings::s0_1_gpio)) jsonDoc[Settings::s0_1_gpio] = httpServer->arg(Settings::s0_1_gpio);
      if (httpServer->hasArg(Settings::s0_1_ppkwh)) jsonDoc[Settings::s0_1_ppkwh] = httpServer->arg(Settings::s0_1_ppkwh);
      if (httpServer->hasArg(Settings::s0_1_interval)) jsonDoc[Settings::s0_1_interval] = httpServer->arg(Settings::s0_1_interval);
      if (httpServer->hasArg(Settings::s0_2_gpio)) jsonDoc[Settings::s0_2_gpio] = httpServer->arg(Settings::s0_2_gpio);
      if (httpServer->hasArg(Settings::s0_2_ppkwh)) jsonDoc[Settings::s0_2_ppkwh] = httpServer->arg(Settings::s0_2_ppkwh);
      if (httpServer->hasArg(Settings::s0_2_interval)) jsonDoc[Settings::s0_2_interval] = httpServer->arg(Settings::s0_2_interval);
    }
    jsonDoc[Settings::listenonly] = Settings::toString(httpServer->hasArg(Settings::listenonly));
    jsonDoc[Settings::logMqtt] = Settings::toString(httpServer->hasArg(Settings::logMqtt));
    jsonDoc[Settings::logHexdump] = Settings::toString(httpServer->hasArg(Settings::logHexdump));
    jsonDoc[Settings::logSerial1] = Settings::toString(httpServer->hasArg(Settings::logSerial1));
    jsonDoc[Settings::optionalPCB] = Settings::toString(httpServer->hasArg(Settings::optionalPCB));
    if (httpServer->hasArg(Settings::waitTime)) {
      jsonDoc[Settings::waitTime] = httpServer->arg(Settings::waitTime);
    }
    if (httpServer->hasArg(Settings::waitDallasTime)) {
      jsonDoc[Settings::waitDallasTime] = httpServer->arg(Settings::waitDallasTime);
    }
    if (httpServer->hasArg(Settings::updateAllTime)) {
      jsonDoc[Settings::updateAllTime] = httpServer->arg(Settings::updateAllTime);
    }
    if (httpServer->hasArg(Settings::updateAllDallasTime)) {
      jsonDoc[Settings::updateAllDallasTime] = httpServer->arg(Settings::updateAllDallasTime);
    }

    saveJsonToConfig(jsonDoc); //save to config file
    loadSettings(heishamonSettings); //load config file to current settings

    if (reconnectWiFi) {
      httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
      httpServer->send(200, "text/html", "");
      httpServer->sendContent_P(webHeader);
      httpServer->sendContent_P(webCSS);
      httpServer->sendContent_P(webBodyStart);
      httpServer->sendContent(Html::leftMenu(httpServer->uri()));
      httpServer->sendContent_P(webBodySettingsNewWifiWarning);
      httpServer->sendContent_P(refreshMeta);
      httpServer->sendContent_P(webFooter);
      httpServer->sendContent("");
      httpServer->client().stop();
      setupWifi(heishamonSettings);
      return true;
    }
  }

  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent(Html::leftMenu(httpServer->uri()));

  String html = F("<div class=\"w3-container w3-center\">");
  html += F("<h2>Settings</h2>");
  html += F("<form enctype=\"multipart/form-data\" accept-charset=\"UTF-8\" action=\"/settings\" method=\"POST\">");
  html += F("<table style=\"width:100%\">");
  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Hostname:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::wifi_hostname, heishamonSettings->wifi_hostname);
  html += F("</td></tr>");
  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Wifi SSID:</td><td style=\"text-align:left\">");
  // wifi scan select box
  html += F("<input list=\"available_ssid\" name=\"wifi_ssid\" id=\"wifi_ssid_id\" value=\"");
  html += heishamonSettings->wifi_ssid;
  html += F("\">");
  html += F("<select id=\"wifi_ssid_select\" style=\"display:none\" onchange=\"changewifissid()\">");
  html += F("<option hidden selected value=\"\">Select SSID</option>");
  html += F("</select>");
  //
  html += F("</td></tr>");
  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Wifi password:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::wifi_password, heishamonSettings->wifi_password, "password");
  html += F("</td></tr>");

  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("What if WiFi connection is lost?</td><td style=\"text-align:left\">");
  html += Html::radioButton(Settings::hotspot_mode, F("Wait until it comes back"), HotspotMode::None, heishamonSettings->hotspot_mode);
  html += F("<br/>");
  html += Html::radioButton(Settings::hotspot_mode, F("Open secured hotspot"), HotspotMode::Secured, heishamonSettings->hotspot_mode);
  html += F("<br/>");
  html += Html::radioButton(Settings::hotspot_mode, F("Open unsecured hotspot"), HotspotMode::Unsecured, heishamonSettings->hotspot_mode);
  html += F("</td></tr>");

  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Update username:</td><td style=\"text-align:left\">");
  html += F("<label name=\"username\">admin</label>");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Current update password:</td><td style=\"text-align:left\">");
  html += Html::textBox("current_ota_password", "", "password");
  html += F(" default password: \"heisha\"");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("New update password:</td><td style=\"text-align:left\">");
  html += Html::textBox("new_ota_password", "", "password");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");

  httpServer->sendContent(html);

  html = F("Mqtt topic base:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::mqtt_topic_base, heishamonSettings->mqtt_topic_base);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Mqtt server:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::mqtt_server, heishamonSettings->mqtt_server);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Mqtt port:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::mqtt_port, heishamonSettings->mqtt_port);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Mqtt username:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::mqtt_username, heishamonSettings->mqtt_username);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Mqtt password:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::mqtt_password, heishamonSettings->mqtt_password, "password");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("How often new values are collected from heatpump:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::waitTime, String(heishamonSettings->waitTime), "number");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("How often all heatpump values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::updateAllTime, String(heishamonSettings->updateAllTime), "number");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");

  httpServer->sendContent(html);

  html = F("Listen only mode:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::listenonly, heishamonSettings->listenonly);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Debug log to MQTT topic from start:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::logMqtt, heishamonSettings->logMqtt);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Debug log hexdump enable from start:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::logHexdump, heishamonSettings->logHexdump);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Debug log to serial1 (GPIO2):</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::logSerial1, heishamonSettings->logSerial1);
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("Emulate optional PCB:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::optionalPCB, heishamonSettings->optionalPCB);
  html += F("</td></tr>");
  html += F("</table>");

  httpServer->sendContent(html);

  // 1wire
  html = F("<table style=\"width:100%\">");
  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Use 1wire DS18b20:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::use_1wire, heishamonSettings->use_1wire, "onclick='ShowHideDallasTable(this)'");
  html += F("</td></tr>");
  html += F("</table>");
  html += F("<table id=\"dallassettings\" style=\"display:");
  if (heishamonSettings->use_1wire) html += F("table"); else html += F("none");
  html += F("; width:100%\">");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("How often new values are collected from 1wire:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::waitDallasTime, String(heishamonSettings->waitDallasTime), "number");
  html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
  html += F("How often all 1wire values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">");
  html += Html::textBox(Settings::updateAllDallasTime, String(heishamonSettings->updateAllDallasTime), "number");
  html += F("</td></tr>");
  html += F("</table>");

  httpServer->sendContent(html);
  // s0
  html = F("<table style=\"width:100%\">");
  html += F("<tr><td style=\"text-align:right; width: 50%\">");
  html += F("Use s0 kWh metering:</td><td style=\"text-align:left\">");
  html += Html::checkbox(Settings::use_s0, heishamonSettings->use_s0, "onclick='ShowHideS0Table(this)'");
  html += F("</td></tr>");
  html += F("</table>");
  html += F("<table id=\"s0settings\" style=\"display:");
  if (heishamonSettings->use_s0) html += F("table"); else html += F("none");
  html += F("; width:100%\">");
  //begin default S0 pins hack
  if (heishamonSettings->s0Settings[0].gpiopin == 255) heishamonSettings->s0Settings[0].gpiopin = DEFAULT_S0_PIN_1;
  if (heishamonSettings->s0Settings[1].gpiopin == 255) heishamonSettings->s0Settings[1].gpiopin = DEFAULT_S0_PIN_2;
  //end default S0 pins hack
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    String s0PortLabel = F("S0 port ");
    s0PortLabel += i + 1;
    String s0Prefix = F("s0_");
    s0Prefix += i + 1;
    html += F("<tr><td style=\"text-align:right; width: 50%\">");
    html += s0PortLabel + F(" GPIO:</td><td style=\"text-align:left\">");
    html += Html::textBox(s0Prefix + F("_gpio"), String(heishamonSettings->s0Settings[i].gpiopin), "number");
    html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    html += s0PortLabel + F(" imp/kwh:</td><td style=\"text-align:left\">");
    html += Html::textBox(s0Prefix + F("_ppkwh"), String(heishamonSettings->s0Settings[i].ppkwh), "number");
    html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    html += s0PortLabel + F(" reporting interval during standby/low power usage:</td><td style=\"text-align:left\">");
    html += Html::textBox(s0Prefix + F("_interval"), String(heishamonSettings->s0Settings[i].lowerPowerInterval), "number");
    html += F("</td></tr><tr><td style=\"text-align:right; width: 50%\">");
    html += s0PortLabel + F(" standby/low power usage threshold:</td><td style=\"text-align:left\"><label id=\"s0_minwatt_") + (i + 1) + F("\">") + (int) round((3600 * 1000 / heishamonSettings->s0Settings[i].ppkwh) / heishamonSettings->s0Settings[i].lowerPowerInterval) + F("</label> Watt");
    html += F("</td></tr>");
  }
  html += F("</table>");

  html += F("<br><br>");
  html += F("<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save\">");
  html += F("</form>");
  html += F("<br><a href=\"/factoryreset\" class=\"w3-red w3-button\" onclick=\"return confirm('Are you sure?')\" >Factory reset</a>");
  html += F("</div>");
  httpServer->sendContent(html);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(settingsJS);
  httpServer->sendContent_P(populatescanwifiJS);
  httpServer->sendContent_P(changewifissidJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();

  /*
   * need to reload some settings in main loop if save was done
   */
  return httpServer->args();
}

void handleTopicSelection(ESP8266WebServer& httpServer, SettingsStruct& settings) {
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "text/html", "");
  httpServer.sendContent_P(webHeader);
  httpServer.sendContent_P(webCSS);
  httpServer.sendContent_P(webBodyStart);
  httpServer.sendContent(Html::leftMenu(httpServer.uri()));

  if (httpServer.args() > 0) {
    int selectedTopicsCount = 0;
    for (int i = 0; i < httpServer.args(); i++) {
      String argName = httpServer.argName(i);
      if (!argName.startsWith("TOP")) continue;
      if (selectedTopicsCount == MAX_SELECTED_TOPICS) break;
      settings.selected_topics[selectedTopicsCount++] = argName.substring(3).toInt();
    }
    settings.selected_topics_count = selectedTopicsCount;

    String html = F("<p>Selected ");
    html += selectedTopicsCount;
    html += F(" topics.</p>");
    httpServer.sendContent(html);

    DynamicJsonDocument jsonDoc(1024);
    settings.toJson(jsonDoc);
    saveJsonToConfig(jsonDoc);
  }

  httpServer.sendContent(F("<form method='POST' action=''>"));
  httpServer.sendContent(F("<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Name</th></tr></thead><tbody>"));

  int i = 0;
  for (int topic = 0; topic < NUMBER_OF_TOPICS; topic++) {
    bool isSelected = false;
    if (i < settings.selected_topics_count) {
      isSelected = (topic == settings.selected_topics[i]);
      if (isSelected) i++;
    }

    String topicId = F("TOP");
    topicId += topic;

    String htmlTableRow = F("<tr><td>");
    htmlTableRow += Html::checkbox(topicId, isSelected);
    htmlTableRow += topicId;
    htmlTableRow += F("</td><td>");
    htmlTableRow += topics[topic];
    htmlTableRow += F("</td></tr>");
    httpServer.sendContent(htmlTableRow);
  }

  httpServer.sendContent(F("</tbody></table>"));
  httpServer.sendContent(F("<input class='w3-green w3-button' type='submit' value='Save'>"));
  httpServer.sendContent(F("</form>"));

  httpServer.sendContent_P(menuJS);
  httpServer.sendContent_P(webFooter);
  httpServer.sendContent("");
  httpServer.client().stop();
}

void handleSmartcontrol(ESP8266WebServer *httpServer, SettingsStruct *heishamonSettings, String actData[]) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webCSS);
  httpServer->sendContent_P(webBodyStart);
  httpServer->sendContent(Html::leftMenu(httpServer->uri()));
  httpServer->sendContent_P(webBodySmartcontrol2);

  String html = F("<form action=\"/smartcontrol\" method=\"POST\">");
  httpServer->sendContent(html);
  httpServer->sendContent_P(webBodyEndDiv);

  //Heating curve
  httpServer->sendContent_P(webBodySmartcontrolHeatingcurve1);

  //check if POST was made with save settings, if yes then save and reboot
  if (httpServer->args()) {
    DynamicJsonDocument jsonDoc(1024);
    //set jsonDoc with current settings
    jsonDoc["enableHeatCurve"] = Settings::toString(heishamonSettings->SmartControlSettings.enableHeatCurve);
    jsonDoc["avgHourHeatCurve"] = heishamonSettings->SmartControlSettings.avgHourHeatCurve;
    jsonDoc["heatCurveTargetHigh"] = heishamonSettings->SmartControlSettings.heatCurveTargetHigh;
    jsonDoc["heatCurveTargetLow"] = heishamonSettings->SmartControlSettings.heatCurveTargetLow;
    jsonDoc["heatCurveOutHigh"] = heishamonSettings->SmartControlSettings.heatCurveOutHigh;
    jsonDoc["heatCurveOutLow"] = heishamonSettings->SmartControlSettings.heatCurveOutLow;
    for (unsigned int i = 0 ; i < 36 ; i++) {
      jsonDoc["heatCurveLookup"][i] = heishamonSettings->SmartControlSettings.heatCurveLookup[i];
    }

    //then overwrite with new settings
    jsonDoc["enableHeatCurve"] = Settings::toString(httpServer->hasArg("heatingcurve"));
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
    for (int i = 0; i < 36; i++) {
      String argName = F("lookup");
      argName += i;
      if (httpServer->hasArg(argName)) {
        jsonDoc["heatCurveLookup"][i] = httpServer->arg(argName);
      }
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

  String heatingModeStr = actData[76];
  int heatingMode = (heatingModeStr.length() > 0) ? heatingModeStr.toInt() : 1;
  if (heatingMode == 1) {
    html = F("<div class=\"w3-row-padding\"><div class=\"w3-half\">");
    html += Html::checkbox(F("heatingcurve"), heishamonSettings->SmartControlSettings.enableHeatCurve, "class='w3-check'");
    html += F("<label>Enable smart heating curve</label></div><div class=\"w3-half\"><select class=\"w3-select\" name=\"average-time\">");
    for (int i = 0; i <= 48; i += 12) {
      String label;
      if (i == 0) label = F("No average on outside temperature");
      else {
        label = F("Average outside temperature over last ");
        label += i;
        label += " hours";
      }      
      html += Html::option(String(i), label, heishamonSettings->SmartControlSettings.avgHourHeatCurve == i);
    }
    html += F("</select></div></div><br><div class=\"w3-row-padding\"><div class=\"w3-half\"><label>Heating Curve Target High Temp</label>");
    html += Html::textBox("hcth", String(heishamonSettings->SmartControlSettings.heatCurveTargetHigh), "number", "id='hcth' class='w3-input w3-border' min='20' max='60' required");
    html += F("</div><div class=\"w3-half\"><label>Heating Curve Target Low Temp</label>");
    html += Html::textBox("hctl", String(heishamonSettings->SmartControlSettings.heatCurveTargetLow), "number", "id='hctl' class='w3-input w3-border' min='20' max='60' required");
    html += F("</div></div><div class=\"w3-row-padding\"><div class=\"w3-half\"><label>Heating Curve Outside High Temp</label>");
    html += Html::textBox("hcoh", String(heishamonSettings->SmartControlSettings.heatCurveOutHigh), "number", "id='hcoh' class='w3-input w3-border' min='-20' max='15' required");
    html += F("</div><div class=\"w3-half\"><label>Heating Curve Outside Low Temp</label>");
    html += Html::textBox("hcol", String(heishamonSettings->SmartControlSettings.heatCurveOutLow), "number", "id='hcol' class='w3-input w3-border' min='-20' max='15' required");
    html += F("</div></div><br><br>");
    html += F("<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save and reboot\">");
    html += F("<div class=\"w3-panel w3-red\">");
    html += F("<p>");
    html += getAvgOutsideTemp();
    html += F("</p>");
    httpServer->sendContent(html);
    httpServer->sendContent_P(webBodyEndDiv);

    httpServer->sendContent_P(webBodySmartcontrolHeatingcurveSVG);
    httpServer->sendContent_P(webBodySmartcontrolHeatingcurve2);
    httpServer->sendContent_P(webBodyEndDiv);
  } else {
    html = F("Heating mode must be \"direct heating\" to enable this option");
    httpServer->sendContent(html);
    httpServer->sendContent_P(webBodyEndDiv);
  }

  httpServer->sendContent_P(webBodyEndDiv);

  //Other example
  //  httpServer->sendContent_P(webBodySmartcontrolOtherexample);
  //  html = "...Loading...";
  //  html += "";
  //  httpServer->sendContent(html);
  //  httpServer->sendContent_P(webBodyEndDiv);

  html = "</form>";
  httpServer->sendContent(html);
  httpServer->sendContent_P(webBodyEndDiv);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(selectJS);
  // httpServer->sendContent_P(heatingCurveJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleWifiScan(ESP8266WebServer *httpServer) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->sendHeader("Access-Control-Allow-Origin", "*");
  httpServer->send(200, "application/json", "");

  if (numSsid > 0) { //found wifi networks
    String html = "[";
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
      if (indexes[i] == -1) continue;
      if (!firstSSID) {
        html += ",";
      }
      html += "{\"ssid\":\"" + WiFi.SSID(indexes[i]) + "\", \"rssi\": \"" + dBmToQuality(WiFi.RSSI(indexes[i])) + "%\"}";
      firstSSID = false;
    }
    html += "]";
    httpServer->sendContent(html);
  }
  httpServer->sendContent("");
  httpServer->client().stop();

  //initatie a new async scan for next try
  WiFi.scanNetworksAsync(getWifiScanResults);
}


bool send_command(byte* command, int length);

void handleREST(ESP8266WebServer *httpServer, bool optionalPCB) {

  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->sendHeader("Access-Control-Allow-Origin", "*");
  httpServer->send(200, "text/plain", "");

  String html = "";
  if (httpServer->method() == HTTP_GET) {
    for (uint8_t i = 0; i < httpServer->args(); i++) {
      unsigned char cmd[256] = { 0 };
      char log_msg[256] = { 0 };
      unsigned int len = 0;

      for (uint8_t x = 0; x < sizeof(commands) / sizeof(commands[0]); x++) {
        if (strcmp(httpServer->argName(i).c_str(), commands[x].name) == 0) {
          len = commands[x].func((char *)httpServer->arg(i).c_str(), cmd, log_msg);
          html += log_msg;
          html += F("\n");
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
            html += log_msg;
            html += F("\n");
            log_message(log_msg);
          }
        }
      }
    }
  }

  httpServer->sendContent(html);
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
