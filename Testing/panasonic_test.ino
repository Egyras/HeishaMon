#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

//needed for managent port
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <DoubleResetDetect.h>    //https://github.com/jenscski/DoubleResetDetect

// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 0.1

// address to the block in the RTC user memory
// change it if it collides with another usage
// of the address block
#define DRD_ADDRESS 0x00



// Replace with your network credentials
const char* wifi_ssid     = "ssid";
const char* wifi_password = "password";
const char* wifi_hostname = "panasonic_heat_pump";
const char* ota_password  = "panasonic";


char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[40];
char mqtt_password[40];

const char* mqtt_topic_base = "panasonic_heat_pump/sdc";
const char* mqtt_logtopic = "panasonic_heat_pump/sdc/log";
const char* mqtt_set_quiet_mode_topic = "panasonic_heat_pump/SetQuietMode";
const char* mqtt_set_shift_temperature_topic = "panasonic_heat_pump/SetShiftTemperature";
const char* mqtt_set_mode_topic = "panasonic_heat_pump/SetMode";
const char* mqtt_set_force_DHW_topic = "panasonic_heat_pump/SetForceDHW";
const char* mqtt_set_holiday_topic = "panasonic_heat_pump/SetHoliday";
const char* mqtt_set_powerfull_topic = "panasonic_heat_pump/SetPowerfull";
const char* mqtt_set_tank_temp_topic = "panasonic_heat_pump/SetTankTemp";
const char* mqtt_set_cool_temp_topic = "panasonic_heat_pump/SetCoolTemp";
byte inCheck = 0;


//useful for debugging, outputs info to a separate mqtt topic
const bool outputMqttLog = true;


// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 256
char data[MAXDATASIZE];
int data_length = 0;

// log message to sprintf to
char log_msg[256];

// mqtt topic to sprintf and then publish to
char mqtt_topic[256];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial1.println("Should save config");
  shouldSaveConfig = true;
}

//doule reset detection
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// mqtt
WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);


void setupWifi() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  Serial1.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);

  if (drd.detect()) {
    Serial1.println("Double reset detected, clearing config.");
    SPIFFS.format();
    wifiManager.resetSettings();
  }


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

          strcpy(mqtt_server, jsonDoc["mqtt_server"]);
          strcpy(mqtt_port, jsonDoc["mqtt_port"]);
          strcpy(mqtt_username, jsonDoc["mqtt_username"]);
          strcpy(mqtt_password, jsonDoc["mqtt_password"]);

        } else {
          Serial1.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial1.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);


  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);


  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);


  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("PanaMon-Setup")) {
    Serial1.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial1.println("Wifi connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());


  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial1.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
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

void send_command(byte* command, int length)
{


  int bytesSent = Serial.write(command, length);
  sprintf(log_msg, "sent bytes    : %d", bytesSent); log_message(log_msg);

  // wait until the serial buffer is filled with the replies
  delay(1000);

  // read the serial
  readSerial();

  sprintf(log_msg, "received size : %d", data_length); log_message(log_msg);
}

// Callback function that is called when a message has been pushed to one of your topics.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  msg[length] = '\0';
  // set 0 for Off mode, set 1 for Quiet mode 1, set 2 for Quiet mode 2, set 3 for Quiet mode 3
  if (strcmp(topic, mqtt_set_quiet_mode_topic) == 0)
  {
    String set_quiet_mode_string(msg);
    int quiet_mode = (set_quiet_mode_string.toInt() + 1) * 8;
    int checksum = 512 - (241 + 108 + 1 + 16 + quiet_mode);

    sprintf(log_msg, "set Quiet mode to %d", quiet_mode / 8 - 1); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, quiet_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
  }
  // set from -5 to 5 to get same temperature shift point
  if (strcmp(topic, mqtt_set_shift_temperature_topic) == 0)
  {
    String set_shift_temperature_string(msg);
    int shift_mode = set_shift_temperature_string.toInt() + 128;
    int checksum = 512 - (241 + 108 + 1 + 16 + shift_mode);

    sprintf(log_msg, "set shift temperature to %d", shift_mode - 128 ); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, shift_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
<<<<<<< HEAD
     }

   // set mode to force DHW by sending 1   
    if (strcmp(topic, mqtt_set_force_DHW_topic) == 0)
     {
    String set_force_DHW_string(msg);
    int force_DHW_mode = set_force_DHW_string.toInt() + 129; 
    int checksum = 1536 - (241 + 108 + 1 + 16 + force_DHW_mode + 84 + 33 + 73 + 5 + 128 + 148 + 177 + 113 + 113);

    sprintf(log_msg, "set force mode to %d", force_DHW_mode - 129); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, force_DHW_mode, 0x54, 0x21, 0x49, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x94, 0x00, 0x00, 0xb1, 0x71, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }

      // set Holiday mode by sending 100, off will be 84
    if (strcmp(topic, mqtt_set_holiday_topic) == 0)
     {
    String set_holiday_string(msg);
    int set_holiday = set_holiday_string.toInt(); 
    int checksum = 1536 - (241 + 108 + 1 + 16 + 66 + set_holiday + 34 + 73 + 5 + 128 + 148 + 177 + 113 + 113);

    sprintf(log_msg, "set holiday mode to %d", set_holiday); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x42, set_holiday, 0x22, 0x49, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x94, 0x00, 0x00, 0xb1, 0x71, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }

    // set Powerfull mode by sending 0 = off, 30 for 30min, 60 for 60min, 90 for 90 min
    if (strcmp(topic, mqtt_set_powerfull_topic) == 0)
     {
    String set_powerfull_string(msg);
    int set_powerfull = (set_powerfull_string.toInt() / 30) + 73; 
    int checksum = 1536 - (241 + 108 + 1 + 16 + 66 + 84 + 34 + set_powerfull + 5 + 128 + 148 + 177 + 113 + 113);

    sprintf(log_msg, "set powerfull mode to %d", (set_powerfull - 73) * 30); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x42, 0x54, 0x22, set_powerfull, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x94, 0x00, 0x00, 0xb1, 0x71, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }

    // set Heat pump mode  18 = heat only, 19 = cool only, 24 = Auto, 34 = Heat+DHW, 35 = Cool+DHW, 40 = Auto + DHW
    if (strcmp(topic, mqtt_set_mode_topic) == 0)
     {
    String set_mode_string(msg);
    int set_mode = set_mode_string.toInt(); 
    int checksum = 1536 - (241 + 108 + 1 + 16 + 66 + 84 + set_mode + 73 + 5 + 128 + 148 + 177 + 113 + 113);

    sprintf(log_msg, "set heat pump mode to %d", set_mode); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x42, 540x54, set_mode, 0x49, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x94, 0x00, 0x00, 0xb1, 0x71, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }

    // set Tank temperature by sending desired temperature between 40C-75C    
    if (strcmp(topic, mqtt_set_tank_temp_topic) == 0)
     {
    String set_tank_temp_string(msg);
    int set_tank_temp = set_tank_temp_string.toInt() + 128; 
    int checksum = 512 - (241 + 108 + 1 + 16 + set_tank_temp);

    sprintf(log_msg, "set Tank temperature to %d", set_tank_temp - 128); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, set_tank_temp, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }

    // set Cool temperature by sending desired temperature between 5C-20C (if selected Direct mode)    
    if (strcmp(topic, mqtt_set_cool_temp_topic) == 0)
     {
    String set_cool_temp_string(msg);
    int set_cool_temp = set_cool_temp_string.toInt() + 128; 
    int checksum = 512 - (241 + 108 + 1 + 16 + set_cool_temp);

    sprintf(log_msg, "set Cool temperature to %d", set_cool_temp - 128); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, set_cool_temp, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, checksum};
    send_command(command, sizeof(command));
     }  
     
=======
  }

>>>>>>> 95d28ee429ed28c65179a9d125804a131df6352d
  if (strcmp(topic, mqtt_topic_base) == 0)
  {
    log_message("Updating..");

    update_everything();
  }
}

void update_everything()
{

  get_heatpump_data();

}

void get_heatpump_data() {
  byte command[] = {0x71, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12};
  send_command(command, sizeof(command));

  int quiet_mode_state = (int)(data[7]);

  char* quiet_mode_state_string;
  if (quiet_mode_state == 73) {
    quiet_mode_state_string = "0";
  } else if (quiet_mode_state == 81) {
    quiet_mode_state_string = "1";
  } else if (quiet_mode_state == 89) {
    quiet_mode_state_string = "2";
  } else if (quiet_mode_state == 97) {
    quiet_mode_state_string = "3";
  } else {
    quiet_mode_state_string = "Unknown";
  }
  sprintf(log_msg, "received quiet mode state : %d (%s)", quiet_mode_state, quiet_mode_state_string); log_message(log_msg);

<<<<<<< HEAD
    int mode_state = (int)(data[6]);
    
    char* mode_state_string;
    if (mode_state == 98) {
      mode_state_string = "Heat+DHW";
    } else if (mode_state == 82) {
      mode_state_string = "Heat";
    } else if (mode_state == 97) {
      mode_state_string = "DHW";
    } else if (mode_state == 105) {
      mode_state_string = "Auto+DHW";
    } else if (mode_state == 99) {
      mode_state_string = "Cool+DHW";
    } else if (mode_state == 83) {
      mode_state_string = "Cool";
    } else if (mode_state == 89) {
      mode_state_string = "Auto";
    } else {
      mode_state_string = "Unknown"; 
    }
    sprintf(log_msg, "received heat pump mode state : %d (%s)", mode_state, mode_state_string); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "mode_state"); mqtt_client.publish(mqtt_topic, mode_state_string);
    

    int quiet_mode_state = (int)(data[7]);
   

    char* powerfull_mode_state_string;
    char* quiet_mode_state_string;
    if (quiet_mode_state == 73) {
      quiet_mode_state_string = "0";
      powerfull_mode_state_string = "0";
    } else if (quiet_mode_state == 81) {
      quiet_mode_state_string = "1";
    } else if (quiet_mode_state == 89) {
      quiet_mode_state_string = "2";
    } else if (quiet_mode_state == 97) {
      quiet_mode_state_string = "3";
    } else if (quiet_mode_state == 74) {
      powerfull_mode_state_string = "30";
    } else if (quiet_mode_state == 75) {
      powerfull_mode_state_string = "60";
    } else if (quiet_mode_state == 76) {
      powerfull_mode_state_string = "90";
    } else {
      quiet_mode_state_string = "Unknown"; 
    }
    sprintf(log_msg, "received quiet mode state : %d (%s)", quiet_mode_state, quiet_mode_state_string); log_message(log_msg);
        
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "quiet_mode_state"); mqtt_client.publish(mqtt_topic, quiet_mode_state_string);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "powerfull_mode_state"); mqtt_client.publish(mqtt_topic, powerfull_mode_state_string);
    
    
    float HeatShiftTemp = (float)data[38] - 128;
      sprintf(log_msg, "received temperature (HeatShiftTemp): %.2f", HeatShiftTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HeatShiftTemp"); mqtt_client.publish(mqtt_topic, String(HeatShiftTemp).c_str());

    float CoolShiftTemp = (float)data[39] - 128;
      sprintf(log_msg, "received temperature (CoolShiftTemp): %.2f", CoolShiftTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "CoolShiftTemp"); mqtt_client.publish(mqtt_topic, String(CoolShiftTemp).c_str());

    float TankSetTemp = (float)data[42] - 128;
      sprintf(log_msg, "received temperature (TankSetTemp): %.2f", TankSetTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "TankSetTemp"); mqtt_client.publish(mqtt_topic, String(TankSetTemp).c_str());

    float HCurveOutHighTemp = (float)data[75] - 128;
      sprintf(log_msg, "received temperature (HCurveOutHighTemp): %.2f", HCurveOutHighTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutHighTemp).c_str());

    float HCurveOutLowTemp = (float)data[76] - 128;
      sprintf(log_msg, "received temperature (HCurveOutLowTemp): %.2f", HCurveOutLowTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutLowTemp).c_str());

    float HCurveOutsLowTemp = (float)data[77] - 128;
      sprintf(log_msg, "received temperature (HCurveOutsLowTemp): %.2f", HCurveOutsLowTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutsLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsLowTemp).c_str());

    float HCurveOutsHighTemp = (float)data[78] - 128;
      sprintf(log_msg, "received temperature (HCurveOutsHighTemp): %.2f", HCurveOutsHighTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutsHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsHighTemp).c_str());               

    int walve_state = (int)(data[111]);
    
    char* walve_state_string;
    if (walve_state == 85) {
      walve_state_string = "Room";
    } else if (walve_state == 86) {
      walve_state_string = "Tank";
    } else {
      walve_state_string = "Unknown"; 
    }
    sprintf(log_msg, "received filter state : %d (%s)", walve_state, walve_state_string); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "walve_state"); mqtt_client.publish(mqtt_topic, walve_state_string);

    float ActWatOutTemp = (float)data[139] - 128;
      sprintf(log_msg, "received temperature (ActWatOutTemp): %.2f", ActWatOutTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActWatOutTemp"); mqtt_client.publish(mqtt_topic, String(ActWatOutTemp).c_str());

    float ActTankTemp = (float)data[141] - 128;
      sprintf(log_msg, "received temperature (ActTankTemp): %.2f", ActTankTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActTankTemp"); mqtt_client.publish(mqtt_topic, String(ActTankTemp).c_str());

    float ActOutTemp = (float)data[142] - 128;
      sprintf(log_msg, "received temperature (ActOutTemp): %.2f", ActOutTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActOutTemp"); mqtt_client.publish(mqtt_topic, String(ActOutTemp).c_str());

    float WatOutTarTemp = (float)data[153] - 128;
      sprintf(log_msg, "received temperature (WatOutTarTemp): %.2f", WatOutTarTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "WatOutTarTemp"); mqtt_client.publish(mqtt_topic, String(WatOutTarTemp).c_str());  
    
    float RoomTherTemp = (float)data[156] - 128;
      sprintf(log_msg, "received temperature (RoomTherTemp): %.2f", RoomTherTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "RoomTherTemp"); mqtt_client.publish(mqtt_topic, String(RoomTherTemp).c_str());

    int PumpFlow1 = (int)data[170];
    float PumpFlow2 = ((((float)data[169] - 1) / 5) * 2) / 100;
    float PumpFlow = PumpFlow1 + PumpFlow2;

      sprintf(log_msg, "received pump flow (PumpFlow): %.2f", PumpFlow); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "PumpFlow"); mqtt_client.publish(mqtt_topic, String(PumpFlow).c_str());       

    float CompFreq = (float)data[166] - 1;
      sprintf(log_msg, "received compressor frequency (CompFreq): %.2f", CompFreq); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "CompFreq"); mqtt_client.publish(mqtt_topic, String(CompFreq).c_str());


    float InletTemp = (float)data[143] - 128;
      sprintf(log_msg, "received temperature (InletTemp): %.2f", InletTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "InletTemp"); mqtt_client.publish(mqtt_topic, String(InletTemp).c_str());


  int ForceDHW_status = (int)(data[4]);
    
    char* ForceDHW_status_string;
    if (ForceDHW_status == 86) {
      ForceDHW_status_string = "0";
    } else if (ForceDHW_status == 150) {
      ForceDHW_status_string = "1";
    } else {
      ForceDHW_status_string = "Unknown"; 
    }
    sprintf(log_msg, "received force DHW status : %d (%s)", ForceDHW_status, ForceDHW_status_string); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ForceDHW"); mqtt_client.publish(mqtt_topic, ForceDHW_status_string);


    int Holiday_mode_status = (int)(data[5]);
    
    char* Holiday_mode_status_string;
    if (Holiday_mode_status == 85) {
      Holiday_mode_status_string = "84";
    } else if (Holiday_mode_status == 101) {
      Holiday_mode_status_string = "100";
    } else {
      Holiday_mode_status_string = "Unknown"; 
    }
    sprintf(log_msg, "received Holiday status : %d (%s)", Holiday_mode_status, Holiday_mode_status_string); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Holiday"); mqtt_client.publish(mqtt_topic, Holiday_mode_status_string);


    float FloorHeatDelta = (float)data[84] - 128;
      sprintf(log_msg, "received temperature (FloorHeatDelta): %.2f", RoomTherTemp); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "FloorHeatDelta"); mqtt_client.publish(mqtt_topic, String(FloorHeatDelta).c_str());

    float FloorCoolDelta = (float)data[94] - 128;
      sprintf(log_msg, "received temperature (FloorCoolDelta): %.2f", FloorCoolDelta); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "FloorCoolDelta"); mqtt_client.publish(mqtt_topic, String(FloorCoolDelta).c_str());

    float TankHeatDelta = (float)data[99] - 128;
      sprintf(log_msg, "received temperature (TankHeatDelta): %.2f", TankHeatDelta); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "TankHeatDelta"); mqtt_client.publish(mqtt_topic, String(TankHeatDelta).c_str());    

    
}
=======
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "quiet_mode_state"); mqtt_client.publish(mqtt_topic, quiet_mode_state_string);
>>>>>>> 95d28ee429ed28c65179a9d125804a131df6352d


  float HeatShiftTemp = (float)data[38] - 128;
  sprintf(log_msg, "received temperature (HeatShiftTemp): %.2f", HeatShiftTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HeatShiftTemp"); mqtt_client.publish(mqtt_topic, String(HeatShiftTemp).c_str());

  float CoolShiftTemp = (float)data[39] - 128;
  sprintf(log_msg, "received temperature (CoolShiftTemp): %.2f", CoolShiftTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "CoolShiftTemp"); mqtt_client.publish(mqtt_topic, String(CoolShiftTemp).c_str());

  float TankSetTemp = (float)data[42] - 128;
  sprintf(log_msg, "received temperature (TankSetTemp): %.2f", TankSetTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "TankSetTemp"); mqtt_client.publish(mqtt_topic, String(TankSetTemp).c_str());

  float HCurveOutHighTemp = (float)data[75] - 128;
  sprintf(log_msg, "received temperature (HCurveOutHighTemp): %.2f", HCurveOutHighTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutHighTemp).c_str());

  float HCurveOutLowTemp = (float)data[76] - 128;
  sprintf(log_msg, "received temperature (HCurveOutLowTemp): %.2f", HCurveOutLowTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutLowTemp).c_str());

  float HCurveOutsLowTemp = (float)data[77] - 128;
  sprintf(log_msg, "received temperature (HCurveOutsLowTemp): %.2f", HCurveOutsLowTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutsLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsLowTemp).c_str());

  float HCurveOutsHighTemp = (float)data[78] - 128;
  sprintf(log_msg, "received temperature (HCurveOutsHighTemp): %.2f", HCurveOutsHighTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurveOutsHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsHighTemp).c_str());

  int walve_state = (int)(data[111]);

  char* walve_state_string;
  if (walve_state == 85) {
    walve_state_string = "Room";
  } else if (walve_state == 86) {
    walve_state_string = "Tank";
  } else {
    walve_state_string = "Unknown";
  }
  sprintf(log_msg, "received filter state : %d (%s)", walve_state, walve_state_string); log_message(log_msg);

  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "walve_state"); mqtt_client.publish(mqtt_topic, walve_state_string);

  float ActWatOutTemp = (float)data[139] - 128;
  sprintf(log_msg, "received temperature (ActWatOutTemp): %.2f", ActWatOutTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActWatOutTemp"); mqtt_client.publish(mqtt_topic, String(ActWatOutTemp).c_str());

  float ActTankTemp = (float)data[141] - 128;
  sprintf(log_msg, "received temperature (ActTankTemp): %.2f", ActTankTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActTankTemp"); mqtt_client.publish(mqtt_topic, String(ActTankTemp).c_str());

  float ActOutTemp = (float)data[142] - 128;
  sprintf(log_msg, "received temperature (ActOutTemp): %.2f", ActOutTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ActOutTemp"); mqtt_client.publish(mqtt_topic, String(ActOutTemp).c_str());

  float WatOutTarTemp = (float)data[153] - 128;
  sprintf(log_msg, "received temperature (WatOutTarTemp): %.2f", WatOutTarTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "WatOutTarTemp"); mqtt_client.publish(mqtt_topic, String(WatOutTarTemp).c_str());

  float RoomTherTemp = (float)data[156] - 128;
  sprintf(log_msg, "received temperature (RoomTherTemp): %.2f", RoomTherTemp); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "RoomTherTemp"); mqtt_client.publish(mqtt_topic, String(RoomTherTemp).c_str());

  int PumpFlow1 = (int)data[170];
  float PumpFlow2 = ((((float)data[169] - 1) / 5) * 2) / 100;
  float PumpFlow = PumpFlow1 + PumpFlow2;

  sprintf(log_msg, "received pump flow (PumpFlow): %.2f", PumpFlow); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "PumpFlow"); mqtt_client.publish(mqtt_topic, String(PumpFlow).c_str());

  float CompFreq = (float)data[166] - 1;
  sprintf(log_msg, "received compressor frequency (CompFreq): %.2f", CompFreq); log_message(log_msg);
  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "CompFreq"); mqtt_client.publish(mqtt_topic, String(CompFreq).c_str());


}

void setupOTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(wifi_hostname);

  // Set authentication
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
  });
  ArduinoOTA.onEnd([]() {
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

  });
  ArduinoOTA.onError([](ota_error_t error) {

  });
  ArduinoOTA.begin();
}

void setup() {
  //serial to cn-cnt
  Serial.begin(9600, SERIAL_8E1);
  Serial.flush();

  setupWifi();
  setupOTA();

  mqtt_client.setServer(mqtt_server, atoi(mqtt_port));
  mqtt_client.setCallback(mqtt_callback);
}

void loop() {
  // Handle OTA first.
  ArduinoOTA.handle();

  if (!mqtt_client.connected())
  {
    mqtt_reconnect();
  }
  mqtt_client.loop();

  update_everything();
  delay(5000);
}

void mqtt_reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected())
  {
    if (mqtt_client.connect(wifi_hostname, mqtt_username, mqtt_password))
    {
      mqtt_client.subscribe(mqtt_set_quiet_mode_topic);
      mqtt_client.subscribe(mqtt_set_shift_temperature_topic);
      mqtt_client.subscribe(mqtt_set_mode_topic);
<<<<<<< HEAD
      mqtt_client.subscribe(mqtt_set_force_DHW_topic);
      mqtt_client.subscribe(mqtt_set_holiday_topic);
      mqtt_client.subscribe(mqtt_set_powerfull_topic);
      mqtt_client.subscribe(mqtt_set_tank_temp_topic);
      mqtt_client.subscribe(mqtt_set_cool_temp_topic);
                     
=======

>>>>>>> 95d28ee429ed28c65179a9d125804a131df6352d
    }
    else
    {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void log_message(char* string)
{
  Serial1.println(string);
  if (outputMqttLog)
  {
    mqtt_client.publish(mqtt_logtopic, string);
  }
}

void readSerial()
{
  if (Serial.available() > 0) {
    data_length = Serial.readBytes(data, 203);
    Serial1.println(String(data));
    while (Serial.available()) {
      delay(2);
      Serial.read();
    }
  }
<<<<<<< HEAD
=======
  else data_length = 0;
>>>>>>> 95d28ee429ed28c65179a9d125804a131df6352d
}
