#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include "webfunctions.h"

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
"<header class=\"w3-container w3-card w3-theme\"><h1>Heisha monitor configuration</h1></header>";

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
  Serial1.println("Should save config");
  shouldSaveConfig = true;
}

void setupWifi(DoubleResetDetect &drd, char* wifi_hostname, char* ota_password, char* mqtt_server, char* mqtt_port, char* mqtt_username, char* mqtt_password) {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);

  if (drd.detect()) {
    Serial1.println("Double reset detected, clearing config.");
    SPIFFS.begin();
    SPIFFS.format();
    wifiManager.resetSettings();
    Serial1.println("Config cleared. Please open the Wifi portal to configure this device...");
  } else {
    //read configuration from FS json
    Serial1.println("mounting FS...");

    if (SPIFFS.begin()) {
      Serial1.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial1.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial1.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument jsonDoc(1024);
          DeserializationError error = deserializeJson(jsonDoc, buf.get());
          serializeJson(jsonDoc, Serial1);
          if (!error) {
            Serial1.println("\nparsed json");
            strcpy(wifi_hostname, jsonDoc["wifi_hostname"]);
            strcpy(ota_password, jsonDoc["ota_password"]);
            strcpy(mqtt_server, jsonDoc["mqtt_server"]);
            strcpy(mqtt_port, jsonDoc["mqtt_port"]);
            strcpy(mqtt_username, jsonDoc["mqtt_username"]);
            strcpy(mqtt_password, jsonDoc["mqtt_password"]);

          } else {
            Serial1.println("Failed to load json config, forcing config reset.");
            wifiManager.resetSettings();
          }
          configFile.close();
        }
      }
      else {
        Serial1.println("No config.json exists! Forcing a config reset.");
      }
    } else {
      Serial1.println("failed to mount FS");
    }
    //end read
  }

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_text1("<p>My hostname and OTA password</p>");
  WiFiManagerParameter custom_wifi_hostname("wifi_hostname", "wifi hostname", wifi_hostname, 40);
  WiFiManagerParameter custom_ota_password("ota_password", "ota password", ota_password, 40);
  WiFiManagerParameter custom_text2("<p>Configure MQTT settings</p>");
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);


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
  wifiManager.setConnectTimeout(60);
  if (!wifiManager.autoConnect("HeishaMon-Setup")) {
    Serial1.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial1.println("Wifi connected...yeey :)");

  //read updated parameters
  strcpy(wifi_hostname, custom_wifi_hostname.getValue());
  strcpy(ota_password, custom_ota_password.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());


  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial1.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["wifi_hostname"] = wifi_hostname;
    jsonDoc["ota_password"] = ota_password;
    jsonDoc["mqtt_server"] = mqtt_server;
    jsonDoc["mqtt_port"] = mqtt_port;
    jsonDoc["mqtt_username"] = mqtt_username;
    jsonDoc["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial1.println("failed to open config file for writing");
    }

    serializeJson(jsonDoc, Serial1);
    serializeJson(jsonDoc, configFile);
    configFile.close();
    //end save
  }

  Serial1.println("local ip");
  Serial1.println(WiFi.localIP());
}
  
void handleRoot(ESP8266WebServer *httpServer, DynamicJsonDocument *actData) {
  httpServer->setContentLength(CONTENT_LENGTH_UNKNOWN); 
  httpServer->send(200, "text/html", "");
  httpServer->sendContent_P(webHeader);
  httpServer->sendContent_P(webBodyStart);

  String httptext = "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">";
  httptext = httptext + "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot heisha monitor</a>";
  httptext = httptext + "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>";
  httptext = httptext + "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>";
  httptext = httptext + "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>";
  httptext = httptext + "<br />";
  httptext = httptext + "<a href=\"/factoryreset\" class=\"w3-red w3-bar-item w3-button\">Factory reset</a>";
  httptext = httptext + "</div>";

  httptext = httptext + "<div class=\"w3-container w3-center\">";
  httptext = httptext + "<h2>Current heatpump values</h2>";
 
  httptext = httptext + "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Value</th></tr></thead><tbody id=\"heishavalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";
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
    String tabletext = "<tr>";
    tabletext = tabletext + "<td>";
    tabletext = tabletext + "panasonic_heat_pump/sdc/" + kv.key().c_str();
    tabletext = tabletext + "</td>";
    tabletext = tabletext + "<td>";
    tabletext = tabletext + kv.value().as<String>();
    tabletext = tabletext + "</td>";
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
