#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include "webfunctions.h"
#include "decode.h"
#include "version.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


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
  "	}"
  "</script>";

void(* resetFunc) (void) = 0;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupWifi(DoubleResetDetect &drd, char* wifi_hostname, char* ota_password, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password) {
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
            strncpy(wifi_hostname, jsonDoc["wifi_hostname"], 39); wifi_hostname[39] = '\0';
            strncpy(ota_password, jsonDoc["ota_password"], 39); ota_password[39] = '\0';
            strncpy(mqtt_server, jsonDoc["mqtt_server"], 39); mqtt_server[39] = '\0';
            strncpy(mqtt_port, jsonDoc["mqtt_port"], 5); mqtt_port[5] = '\0';
            strncpy(mqtt_username, jsonDoc["mqtt_username"], 39); mqtt_username[39] = '\0';
            strncpy(mqtt_password, jsonDoc["mqtt_password"], 39); mqtt_password[39] = '\0';

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
  WiFiManagerParameter custom_wifi_hostname("wifi_hostname", "wifi hostname", wifi_hostname, 39);
  WiFiManagerParameter custom_ota_password("ota_password", "ota password", ota_password, 39);
  WiFiManagerParameter custom_text2("<p>Configure MQTT settings</p>");
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 39);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 39);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 39);


  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);


  //add all your parameters here
  wifiManager.addParameter(&custom_text1);
  wifiManager.addParameter(&custom_wifi_hostname);
  wifiManager.addParameter(&custom_ota_password);
  wifiManager.addParameter(&custom_text2);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);


  wifiManager.setConfigPortalTimeout(180);
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
  strncpy(wifi_hostname, custom_wifi_hostname.getValue(), 39); wifi_hostname[39] = '\0';
  strncpy(ota_password, custom_ota_password.getValue(), 39); ota_password[39] = '\0';
  strncpy(mqtt_server, custom_mqtt_server.getValue(), 39); mqtt_server[39] = '\0';
  strncpy(mqtt_port, custom_mqtt_port.getValue(), 5); mqtt_port[5] = '\0';
  strncpy(mqtt_username, custom_mqtt_username.getValue(), 39); mqtt_username[39] = '\0';
  strncpy(mqtt_password, custom_mqtt_password.getValue(), 39); mqtt_password[39] = '\0';

  //Set hostname on wifi rather than ESP_xxxxx
  WiFi.hostname(wifi_hostname);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["wifi_hostname"] = wifi_hostname;
    jsonDoc["ota_password"] = ota_password;
    jsonDoc["mqtt_server"] = mqtt_server;
    jsonDoc["mqtt_port"] = mqtt_port;
    jsonDoc["mqtt_username"] = mqtt_username;
    jsonDoc["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJson(jsonDoc, Serial);
    serializeJson(jsonDoc, configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void handleRoot(ESP8266WebServer *httpServer, DynamicJsonDocument *actData) {
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
  httptext = httptext + "<hr><div class=\"w3-text-grey\">Version: " + heishamon_version + "</div><hr></div>";

  httptext = httptext + "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<h2>Current heatpump values</h2>";

  httptext = httptext + "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Name</th><th>Value</th><th>Description</th></tr></thead><tbody id=\"heishavalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";
  httpServer->sendContent(httptext);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(refreshJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}

void handleTableRefresh(ESP8266WebServer *httpServer, DynamicJsonDocument *actData) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer->send(200, "text/html", "");
  JsonObject root = actData->as<JsonObject>();
  for (JsonPair kv : root) {
    int topic = atoi(kv.key().c_str());
    String topicname = topics[topic];
    String topicdesc;
    if (topicDescription[topic][0] == "value") {
      topicdesc = topicDescription[topic][1];
    }
    else {
      int value = kv.value();
      topicdesc = topicDescription[topic][value];
    }
    String tabletext = "<tr>";
    tabletext = tabletext + "<td>TOP" + kv.key().c_str() + "</td>";
    tabletext = tabletext + "<td>" + topicname + "</td>";
    tabletext = tabletext + "<td>" + kv.value().as<String>() + "</td>";
    tabletext = tabletext + "<td>" + topicdesc + "</td>";
    tabletext = tabletext + "</tr>";
    httpServer->sendContent(tabletext);
  }
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
  resetFunc();
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
  resetFunc();
}

void handleSettings(ESP8266WebServer *httpServer, char* wifi_hostname, char* ota_password, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password) {
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
    jsonDoc["wifi_hostname"] = wifi_hostname;
    jsonDoc["ota_password"] = ota_password;
    jsonDoc["mqtt_server"] = mqtt_server;
    jsonDoc["mqtt_port"] = mqtt_port;
    jsonDoc["mqtt_username"] = mqtt_username;
    jsonDoc["mqtt_password"] = mqtt_password;
    if (httpServer->hasArg("wifi_hostname")) {
      jsonDoc["wifi_hostname"] = httpServer->arg("wifi_hostname");
    }
    if (httpServer->hasArg("new_ota_password")) {
      if (httpServer->hasArg("current_ota_password") && (strcmp(ota_password, httpServer->arg("current_ota_password").c_str()) == 0 )) {
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
        resetFunc();
      }
    }
  }

  httptext = "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<h2>Settings</h2>";
  httptext = httptext + "<form action=\"/settings\" method=\"POST\">";
  httptext = httptext + "Hostname:<br>";
  httptext = httptext + "<input type=\"text\" name=\"wifi_hostname\" value=\"" + wifi_hostname + "\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "Current update password:<br>";
  httptext = httptext + "<input type=\"password\" name=\"current_ota_password\" value=\"\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "New update password:<br>";
  httptext = httptext + "<input type=\"password\" name=\"new_ota_password\" value=\"\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "Mqtt server:<br>";
  httptext = httptext + "<input type=\"text\" name=\"mqtt_server\" value=\"" + mqtt_server + "\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "Mqtt port:<br>";
  httptext = httptext + "<input type=\"number\" name=\"mqtt_port\" value=\"" + mqtt_port + "\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "Mqtt username:<br>";
  httptext = httptext + "<input type=\"text\" name=\"mqtt_username\" value=\"" + mqtt_username + "\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "Mqtt password:<br>";
  httptext = httptext + "<input type=\"password\" name=\"mqtt_password\" value=\"" + mqtt_password + "\">";
  httptext = httptext + "<br><br>";
  httptext = httptext + "<input class=\"w3-green w3-button\" type=\"submit\" value=\"Save and reboot\">";
  httptext = httptext + "</form>";
  httptext = httptext + "<br><a href=\"/factoryreset\" class=\"w3-red w3-button\" onclick=\"return confirm('Are you sure?')\" >Factory reset</a>";
  httptext = httptext + "</div>";
  httpServer->sendContent(httptext);

  httpServer->sendContent_P(menuJS);
  httpServer->sendContent_P(webFooter);
  httpServer->sendContent("");
  httpServer->client().stop();
}
