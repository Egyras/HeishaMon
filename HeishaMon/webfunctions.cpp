#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include "webfunctions.h"
#include "decode.h"
#include "version.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define UPTIME_OVERFLOW 4294967295 // Uptime overflow value


//flag for saving data
bool shouldSaveConfig = false;




static const char webHeader[] PROGMEM  =
  "<!DOCTYPE html>"
  "<html>"
  "<title>Heisha monitor</title>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3.css\">"
  "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3pro.css\">"
  "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/lib/w3-theme-red.css\">"
  "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3.css\">"
  "<style>"
  "	.w3-btn { margin-bottom:10px; }"
  "</style>";

static const char refreshMeta[] PROGMEM = "<meta http-equiv=\"refresh\" content=\"5; url=/\" />";
static const char webBodyStart[] PROGMEM =
  "<body>"
  "<button class=\"w3-button w3-red w3-xlarge w3-left\" onclick=\"openLeftMenu()\">&#9776;</button>"
  "<header class=\"w3-container w3-card w3-theme\"><h1>Heisha monitor</h1></header>";

static const char webFooter[] PROGMEM  = "</body></html>";
static const char menuJS[] PROGMEM =
  "<script>"
  "	function openLeftMenu() {"
  "		var x = document.getElementById(\"leftMenu\");"
  "		if (x.style.display === \"none\") {"
  "			x.style.display = \"block\";"
  "		} else {"
  "			x.style.display = \"none\";"
  "		}"
  "	}"
  "</script>";

static const char refreshJS[] PROGMEM =
  "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>"
  "<script>"
  "	$(document).ready(function(){refreshTable();});"
  "	function refreshTable(){"
  "		$('#heishavalues').load('/tablerefresh', function(){setTimeout(refreshTable, 30000);});"
  "   $('#dallasvalues').load('/tablerefresh?1wire', function(){});"
  "   $('#s0values').load('/tablerefresh?s0', function(){});"
  "	}"
  "</script>";

static const char selectJS[] PROGMEM =
  "<script>"
  "function openTable(tableName) {"
  "  var i;"
  "  var x = document.getElementsByClassName(\"heishatable\");"
  "  for (i = 0; i < x.length; i++) {"
  "    x[i].style.display = \"none\";"
  "  }"
  "  document.getElementById(tableName).style.display = \"block\";"
  "}"
  "</script>";

static const char s0SettingsJS[] PROGMEM =
  "<script type=\"text/javascript\">"
  "    function ShowHideDallasTable(dallasEnabled) {"
  "        var dallassettings = document.getElementById(\"dallassettings\");"
  "        dallassettings.style.display = dallasEnabled.checked ? \"table\" : \"none\";"
  "    }"
  "    function ShowHideS0Table(s0enabled) {"
  "        var s0settings = document.getElementById(\"s0settings\");"
  "        s0settings.style.display = s0enabled.checked ? \"table\" : \"none\";"
  "    }"
  "    function changeMinWatt(port) {"
  "        var ppkwh = document.getElementById('s0_ppkwh_'+port).value;"
  "        var interval = document.getElementById('s0_interval_'+port).value;"
  "        document.getElementById('s0_minwatt_'+port).innerHTML = Math.round((3600 * 1000 / ppkwh) / interval);"
  "    }"
  "</script>";


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

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
  Serial1.println(total_memory);
  Serial1.println(free_memory);
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
  sprintf(uptime, "%d day%s %d hour%s %d minute%s %d second%s", d, (d == 1) ? "" : "s", h, (h == 1) ? "" : "s", m, (m == 1) ? "" : "s", sec, (sec == 1) ? "" : "s");
  return String(uptime);
}

void setupWifi(DoubleResetDetect &drd, settingsStruct *heishamonSettings) {

  //first get total memory before we do anything
  getFreeMemory();

  //set boottime
  getUptime();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true); //this is debugging on serial port, because serial swap is done after full startup this is ok

  if (drd.detect()) {
    Serial.println("Double reset detected, clearing config.");
    SPIFFS.begin();
    SPIFFS.format();
    wifiManager.resetSettings();
    Serial.println("Config cleared. Please open the Wifi portal to configure this device...");
  } else {
    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument jsonDoc(1024);
          DeserializationError error = deserializeJson(jsonDoc, buf.get());
          serializeJson(jsonDoc, Serial);
          if (!error) {
            Serial.println("\nparsed json");
            //read updated parameters, make sure no overflow
            if ( jsonDoc["wifi_hostname"] ) strlcpy(heishamonSettings->wifi_hostname, jsonDoc["wifi_hostname"], sizeof(heishamonSettings->wifi_hostname));
            if ( jsonDoc["ota_password"] ) strlcpy(heishamonSettings->ota_password, jsonDoc["ota_password"], sizeof(heishamonSettings->ota_password));
            if ( jsonDoc["mqtt_topic_base"] ) strlcpy(heishamonSettings->mqtt_topic_base, jsonDoc["mqtt_topic_base"], sizeof(heishamonSettings->mqtt_topic_base));
            if ( jsonDoc["mqtt_server"] ) strlcpy(heishamonSettings->mqtt_server, jsonDoc["mqtt_server"], sizeof(heishamonSettings->mqtt_server));
            if ( jsonDoc["mqtt_port"] ) strlcpy(heishamonSettings->mqtt_port, jsonDoc["mqtt_port"], sizeof(heishamonSettings->mqtt_port));
            if ( jsonDoc["mqtt_username"] ) strlcpy(heishamonSettings->mqtt_username, jsonDoc["mqtt_username"], sizeof(heishamonSettings->mqtt_username));
            if ( jsonDoc["mqtt_password"] ) strlcpy(heishamonSettings->mqtt_password, jsonDoc["mqtt_password"], sizeof(heishamonSettings->mqtt_password));
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
            if ( jsonDoc["waitTime"]) heishamonSettings->waitTime = jsonDoc["waitTime"];
            if ( jsonDoc["waitDallasTime"]) heishamonSettings->waitDallasTime = jsonDoc["waitDallasTime"];
            if ( jsonDoc["updateAllTime"]) heishamonSettings->updateAllTime = jsonDoc["updateAllTime"];
            if ( jsonDoc["updataAllDallasTime"]) heishamonSettings->updataAllDallasTime = jsonDoc["updataAllDallasTime"];
          } else {
            Serial.println("Failed to load json config, forcing config reset.");
            wifiManager.resetSettings();
          }
          configFile.close();
        }
      }
      else {
        Serial.println("No config.json exists! Forcing a config reset.");
        wifiManager.resetSettings();
      }
    } else {
      Serial.println("failed to mount FS");
    }
    //end read
  }

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_text1("<p>My hostname and OTA password</p>");
  WiFiManagerParameter custom_wifi_hostname("wifi_hostname", "wifi hostname", heishamonSettings->wifi_hostname, 39);
  WiFiManagerParameter custom_ota_password("ota_password", "ota password", heishamonSettings->ota_password, 39);
  WiFiManagerParameter custom_text2("<p>Configure MQTT settings</p>");
  WiFiManagerParameter custom_mqtt_topic_base("mqtt topic base", "mqtt topic base", heishamonSettings->mqtt_topic_base, 39);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", heishamonSettings->mqtt_server, 39);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", heishamonSettings->mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", heishamonSettings->mqtt_username, 39);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", heishamonSettings->mqtt_password, 39);


  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);


  //add all your parameters here
  wifiManager.addParameter(&custom_text1);
  wifiManager.addParameter(&custom_wifi_hostname);
  wifiManager.addParameter(&custom_ota_password);
  wifiManager.addParameter(&custom_text2);
  wifiManager.addParameter(&custom_mqtt_topic_base);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);


  wifiManager.setConfigPortalTimeout(120);
  wifiManager.setConnectTimeout(10);
  if (!wifiManager.autoConnect("HeishaMon-Setup")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Wifi connected...yeey :)");

  //read updated parameters, make sure no overflow
  strncpy(heishamonSettings->wifi_hostname, custom_wifi_hostname.getValue(), 39); heishamonSettings->wifi_hostname[39] = '\0';
  strncpy(heishamonSettings->ota_password, custom_ota_password.getValue(), 39); heishamonSettings->ota_password[39] = '\0';
  strncpy(heishamonSettings->mqtt_topic_base, custom_mqtt_topic_base.getValue(), 39); heishamonSettings->mqtt_topic_base[39] = '\0';
  strncpy(heishamonSettings->mqtt_server, custom_mqtt_server.getValue(), 39); heishamonSettings->mqtt_server[39] = '\0';
  strncpy(heishamonSettings->mqtt_port, custom_mqtt_port.getValue(), 5); heishamonSettings->mqtt_port[5] = '\0';
  strncpy(heishamonSettings->mqtt_username, custom_mqtt_username.getValue(), 39); heishamonSettings->mqtt_username[39] = '\0';
  strncpy(heishamonSettings->mqtt_password, custom_mqtt_password.getValue(), 39); heishamonSettings->mqtt_password[39] = '\0';

  //Set hostname on wifi rather than ESP_xxxxx
  WiFi.hostname(heishamonSettings->wifi_hostname);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["wifi_hostname"] = heishamonSettings->wifi_hostname;
    jsonDoc["ota_password"] = heishamonSettings->ota_password;
    jsonDoc["mqtt_topic_base"] = heishamonSettings->mqtt_topic_base;
    jsonDoc["mqtt_server"] = heishamonSettings->mqtt_server;
    jsonDoc["mqtt_port"] = heishamonSettings->mqtt_port;
    jsonDoc["mqtt_username"] = heishamonSettings->mqtt_username;
    jsonDoc["mqtt_password"] = heishamonSettings->mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJson(jsonDoc, Serial);
    serializeJson(jsonDoc, configFile);
    configFile.close();
    //end save
  }
  Serial.println("==========");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void handleRoot(ESP8266WebServer *httpServer, float readpercentage, settingsStruct *heishamonSettings) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webBodyStart);

  String httptext = "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">";
  httptext = httptext + "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>";
  httptext = httptext + "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>";
  httptext = httptext + "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>";
  httptext = httptext + "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>";
  httptext = httptext + "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>";
  httptext = httptext + "<hr><div class=\"w3-text-grey\">Version: " + heishamon_version + "<br><a href=\"https://github.com/Egyras/HeishaMon\">Heishamon software</a></div><hr></div>";


  httptext = httptext + "<div class=\"w3-bar w3-red\">";
  httptext = httptext + "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Heatpump')\">Heatpump</button>";
  if (heishamonSettings->use_1wire) httptext = httptext + "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Dallas')\">Dallas 1-wire</button>";
  if (heishamonSettings->use_s0) httptext = httptext + "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('S0')\">S0 kWh meters</button>";
  httptext = httptext + "</div>";

  httptext = httptext + "<div class=\"w3-container w3-left\">";
  httptext = httptext + "<br>Wifi signal: " + String(getWifiQuality()) + "%";
  httptext = httptext + "<br>Memory free: " + String(getFreeMemory()) + "%";
  httptext = httptext + "<br>Correct received data: " + String(readpercentage) + "%";
  httptext = httptext + "<br>Uptime: " + getUptime();
  httptext = httptext + "</div>";

  httptext = httptext + "<div id=\"Heatpump\" class=\"w3-container w3-center heishatable\">";
  httptext = httptext + "<h2>Current heatpump values</h2>";
  httptext = httptext + "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Name</th><th>Value</th><th>Description</th></tr></thead><tbody id=\"heishavalues\"><tr><td>...Loading...</td><td></td></r></tbody></table></div>";
  if (heishamonSettings->use_1wire) {
    httptext = httptext + "<div id=\"Dallas\" class=\"w3-container w3-center heishatable\" style=\"display:none\">";
    httptext = httptext + "<h2>Current Dallas 1-wire values</h2>";
    httptext = httptext + "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Sensor</th><th>Temperature</th></tr></thead><tbody id=\"dallasvalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";
  }
  if (heishamonSettings->use_s0) {
    httptext = httptext + "<div id=\"S0\" class=\"w3-container w3-center heishatable\" style=\"display:none\">";
    httptext = httptext + "<h2>Current S0 kWh meters values</h2>";
    httptext = httptext + "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>S0 port</th><th>Watt</th></tr></thead><tbody id=\"s0values\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";
  }
  httpServer->sendContent(httptext);


  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(refreshJS);
  httpServer->sendContent_P(selectJS);
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
      if (strcmp(topicDescription[topic][0], valuetext) == 0) {
        topicdesc = topicDescription[topic][1];
      }
      else {
        int value = actData[topic].toInt();
        topicdesc = topicDescription[topic][value];
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
  String tabletext = "{";
  //heatpump values in json
  tabletext = tabletext + "\"heatpump\":[";
  httpServer->sendContent(tabletext);
  for (unsigned int topic = 0 ; topic < NUMBER_OF_TOPICS ; topic++) {
    String topicdesc;
    const char *valuetext = "value";
    if (strcmp(topicDescription[topic][0], valuetext) == 0) {
      topicdesc = topicDescription[topic][1];
    }
    else {
      int value = actData[topic].toInt();
      topicdesc = topicDescription[topic][value];
    }
    tabletext = "{";
    tabletext = tabletext + "\"Topic\": \"TOP" + topic + "\",";
    tabletext = tabletext + "\"Name\": \"" + topics[topic] + "\",";
    tabletext = tabletext + "\"Value\": \"" + actData[topic] + "\",";
    tabletext = tabletext + "\"Description\": \"" + topicdesc + "\"";
    tabletext = tabletext + "}";
    if (topic < NUMBER_OF_TOPICS - 1) tabletext = tabletext + ",";
    httpServer->sendContent(tabletext);
  }
  tabletext = "]";
  httpServer->sendContent(tabletext);
  //1wire data in json
  tabletext =  ",\"1wire\":" + dallasJsonOutput();
  httpServer->sendContent(tabletext);
  //s0 data in json
  tabletext =  ",\"s0\":" + s0JsonOutput();
  httpServer->sendContent(tabletext);
  //end json string
  tabletext = "}";
  httpServer->sendContent(tabletext);
  httpServer->sendContent("");
  httpServer->client().stop();
}


void handleFactoryReset(ESP8266WebServer *httpServer) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(refreshMeta);
  httpServer->sendContent_P(webBodyStart);

  String httptext = "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<p>Removing configuration. To reconfigure please connect to WiFi hotspot after reset.</p>";
  httptext = httptext + "</div>";
  httpServer->sendContent(httptext);
  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
  delay(1000);
  SPIFFS.begin();
  SPIFFS.format();
  WiFi.disconnect(true);
  delay(1000);
  ESP.restart();
}

void handleReboot(ESP8266WebServer *httpServer) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(refreshMeta);
  httpServer->sendContent_P(webBodyStart);

  String httptext = "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<p>Rebooting</p>";
  httptext = httptext + "</div>";
  httpServer->sendContent(httptext);
  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
  delay(1000);
  ESP.restart();
}

void handleSettings(ESP8266WebServer *httpServer, settingsStruct *heishamonSettings) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webBodyStart);

  String httptext = "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">";
  httptext = httptext + "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>";
  httptext = httptext + "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>";
  httptext = httptext + "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>";
  httptext = httptext + "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>";
  httptext = httptext + "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>";
  httptext = httptext + "</div>";
  httpServer->sendContent(httptext);

  //check if POST was made with save settings, if yes then save and reboot
  if (httpServer->args()) {
    DynamicJsonDocument jsonDoc(1024);
    //set jsonDoc with current settings
    jsonDoc["wifi_hostname"] = heishamonSettings->wifi_hostname;
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
    jsonDoc["waitTime"] = heishamonSettings->waitTime;
    jsonDoc["waitDallasTime"] = heishamonSettings->waitDallasTime;
    jsonDoc["updateAllTime"] = heishamonSettings->updateAllTime;
    jsonDoc["updataAllDallasTime"] = heishamonSettings->updataAllDallasTime;

    //then overwrite with new settings
    if (httpServer->hasArg("wifi_hostname")) {
      jsonDoc["wifi_hostname"] = httpServer->arg("wifi_hostname");
    }
    if (httpServer->hasArg("new_ota_password") && (httpServer->arg("new_ota_password") != NULL) && (httpServer->arg("current_ota_password") != NULL) ) {
      if (httpServer->hasArg("current_ota_password") && (strcmp(heishamonSettings->ota_password, httpServer->arg("current_ota_password").c_str()) == 0 )) {
        jsonDoc["ota_password"] = httpServer->arg("new_ota_password");
      }
      else {
        httptext = "<div class=\"w3-container w3-center\">";
        httptext = httptext + "<h3>------- wrong current password -------</h3>";
        httptext = httptext + "<h3>-- do factory reset if password lost --</h3>";
        httptext = httptext + "</div>";
        httpServer->sendContent(httptext);
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

    if (SPIFFS.begin()) {
      File configFile = SPIFFS.open("/config.json", "w");
      if (configFile) {
        serializeJson(jsonDoc, configFile);
        configFile.close();
        delay(1000);

        httptext = "<div class=\"w3-container w3-center\">";
        httptext = httptext + "<h3>--- saved ---</h3>";
        httptext = httptext + "<h3>-- rebooting --</h3>";
        httptext = httptext + "</div>";
        httpServer->sendContent(httptext);
        httpServer->sendContent_P(refreshMeta);
        httpServer->sendContent_P(webFooter);
        httpServer->sendContent("");
        httpServer->client().stop();
        delay(1000);
        ESP.restart();
      }
    }
  }

  httptext = "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<h2>Settings</h2>";
  httptext = httptext + "<form action=\"/settings\" method=\"POST\">";
  httptext = httptext + "<table style=\"width:100%\">";
  httptext = httptext + "<tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Hostname:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"text\" name=\"wifi_hostname\" value=\"" + heishamonSettings->wifi_hostname + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Current update password:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"password\" name=\"current_ota_password\" value=\"\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "New update password:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"password\" name=\"new_ota_password\" value=\"\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Mqtt topic base:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"text\" name=\"mqtt_topic_base\" value=\"" + heishamonSettings->mqtt_topic_base + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Mqtt server:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"text\" name=\"mqtt_server\" value=\"" + heishamonSettings->mqtt_server + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Mqtt port:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"number\" name=\"mqtt_port\" value=\"" + heishamonSettings->mqtt_port + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Mqtt username:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"text\" name=\"mqtt_username\" value=\"" + heishamonSettings->mqtt_username + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Mqtt password:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"password\" name=\"mqtt_password\" value=\"" + heishamonSettings->mqtt_password + "\">";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "How often new values are collected from heatpump:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"number\" name=\"waitTime\" value=\"" + heishamonSettings->waitTime + "\"> seconds";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "How often all heatpump values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"number\" name=\"updateAllTime\" value=\"" + heishamonSettings->updateAllTime + "\"> seconds";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Listen only mode:</td><td style=\"text-align:left\">";
  if (heishamonSettings->listenonly) {
    httptext = httptext + "<input type=\"checkbox\" name=\"listenonly\" value=\"enabled\" checked >";
  } else {
    httptext = httptext + "<input type=\"checkbox\" name=\"listenonly\" value=\"enabled\">";
  }
  httptext = httptext + "</td></tr>";
  httptext = httptext + "</table>";
  
  // 1wire
  httptext = httptext + "<table style=\"width:100%\">";
  httptext = httptext + "<tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Use 1wire DS18b20:</td><td style=\"text-align:left\">";
  if (heishamonSettings->use_1wire) {
    httptext = httptext + "<input type=\"checkbox\" onclick=\"ShowHideDallasTable(this)\" name=\"use_1wire\" value=\"enabled\" checked >";
    httptext = httptext + "</td></tr>";
    httptext = httptext + "</table>";
    httptext = httptext + "<table id=\"dallassettings\" style=\"display: table; width:100%\">";    
  } else {
    httptext = httptext + "<input type=\"checkbox\" onclick=\"ShowHideDallasTable(this)\" name=\"use_1wire\" value=\"enabled\">";
    httptext = httptext + "</td></tr>";
    httptext = httptext + "</table>";
    httptext = httptext + "<table id=\"dallassettings\" style=\"display: table; width:100%\">";    
  }
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "How often new values are collected from 1wire:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"number\" name=\"waitDallasTime\" value=\"" + heishamonSettings->waitDallasTime + "\"> seconds";
  httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "How often all 1wire values are retransmitted to MQTT broker:</td><td style=\"text-align:left\">";
  httptext = httptext + "<input type=\"number\" name=\"updataAllDallasTime\" value=\"" + heishamonSettings->updataAllDallasTime + "\"> seconds";
  httptext = httptext + "</td></tr>";
  httptext = httptext + "</table>";
  
  // s0
  httptext = httptext + "<table style=\"width:100%\">";
  httptext = httptext + "<tr><td style=\"text-align:right; width: 50%\">";
  httptext = httptext + "Use s0 kWh metering:</td><td style=\"text-align:left\">";
  if (heishamonSettings->use_s0) {
    httptext = httptext + "<input type=\"checkbox\" onclick=\"ShowHideS0Table(this)\" name=\"use_s0\" value=\"enabled\" checked >";
    httptext = httptext + "</td></tr>";
    httptext = httptext + "</table>";
    httptext = httptext + "<table id=\"s0settings\" style=\"display: table; width:100%\">";
  } else {
    httptext = httptext + "<input type=\"checkbox\" onclick=\"ShowHideS0Table(this)\" name=\"use_s0\" value=\"enabled\">";
    httptext = httptext + "</td></tr>";
    httptext = httptext + "</table>";
    httptext = httptext + "<table id=\"s0settings\" style=\"display: none; width:100%\">";
  }
  //begin default S0 pins hack
  if (heishamonSettings->s0Settings[0].gpiopin == 255) heishamonSettings->s0Settings[0].gpiopin = DEFAULT_S0_PIN_1;
  if (heishamonSettings->s0Settings[1].gpiopin == 255) heishamonSettings->s0Settings[1].gpiopin = DEFAULT_S0_PIN_2;
  //end default S0 pins hack
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    httptext = httptext + "<tr><td style=\"text-align:right; width: 50%\">";
    httptext = httptext + "S0 port " + (i + 1) + " GPIO:</td><td style=\"text-align:left\">";
    httptext = httptext + "<input type=\"number\" name=\"s0_" + (i + 1) + "_gpio\" value=\"" + heishamonSettings->s0Settings[i].gpiopin + "\">";
    httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
    httptext = httptext + "S0 port " + (i + 1) + " imp/kwh:</td><td style=\"text-align:left\">";
    httptext = httptext + "<input type=\"number\" id=\"s0_ppkwh_" + (i + 1) + "\" onchange=\"changeMinWatt(" + (i + 1) + ")\" name=\"s0_" + (i + 1) + "_ppkwh\" value=\"" + (heishamonSettings->s0Settings[i].ppkwh) + "\">";
    httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
    httptext = httptext + "S0 port " + (i + 1) + " reporting interval during standby/low power usage:</td><td style=\"text-align:left\">";
    httptext = httptext + "<input type=\"number\" id=\"s0_interval_" + (i + 1) + "\" onchange=\"changeMinWatt(" + (i + 1) + ")\" name=\"s0_" + (i + 1) + "_interval\" value=\"" + (heishamonSettings->s0Settings[i].lowerPowerInterval) + "\"> seconds";
    httptext = httptext + "</td></tr><tr><td style=\"text-align:right; width: 50%\">";
    httptext = httptext + "S0 port " + (i + 1) + " standby/low power usage threshold:</td><td style=\"text-align:left\"><label id=\"s0_minwatt_" + (i + 1) + "\">" + (int) round((3600 * 1000 / heishamonSettings->s0Settings[i].ppkwh) / heishamonSettings->s0Settings[i].lowerPowerInterval) + "</label> Watt";
    httptext = httptext + "</td></tr>";
  }
  httptext = httptext + "</table>";


  httptext = httptext + "<br><br>";  httptext = httptext + "<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save and reboot\">";
  httptext = httptext + "</form>";
  httptext = httptext + "<br><a href=\"/factoryreset\" class=\"w3-red w3-button\" onclick=\"return confirm('Are you sure?')\" >Factory reset</a>";
  httptext = httptext + "</div>";
  httpServer->sendContent(httptext);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(s0SettingsJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}
