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
  }

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

  sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "quiet_mode_state"); mqtt_client.publish(mqtt_topic, quiet_mode_state_string);


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
  else data_length = 0;
}