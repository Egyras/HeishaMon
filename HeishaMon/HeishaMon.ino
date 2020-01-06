#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>

#include <ArduinoJson.h>

#include "webfunctions.h"
#include "decode.h"
#include "commands.h"




// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 0.1

// address to the block in the RTC user memory
// change it if it collides with another usage
// of the address block
#define DRD_ADDRESS 0x00

#define WAITTIME 5000


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* default_hostname = "HeishaMon";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "heisha";

// Default settings if config does not exists
char* wifi_hostname = strdup(default_hostname);
char* ota_password  = strdup(update_password);
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[40];
char mqtt_password[40];

bool sending = false; // mutex for sending data
unsigned long nexttime = 0;


//useful for debugging, outputs info to a separate mqtt topic
bool outputMqttLog = true;
//toggle to dump received hex data in log
bool outputHexDump = false;
// toggle to dump  extralog to serial1
// *needs implementation in this code and in the webpage*
bool outputSerial1 = true;




// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 256
char data[MAXDATASIZE];
int data_length = 0;

// store actual data in a json doc
DynamicJsonDocument actData(2048);

// log message to sprintf to
char log_msg[256];

// mqtt topic to sprintf and then publish to
char mqtt_topic[256];


//doule reset detection
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// mqtt
WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);


void mqtt_reconnect()
{
  Serial1.println("Reconnecting to mqtt server ...");
  if (mqtt_client.connect(wifi_hostname, mqtt_username, mqtt_password, mqtt_willtopic, 1, true, "Offline"))
  {
    mqtt_client.subscribe(mqtt_set_quiet_mode_topic);
    mqtt_client.subscribe(mqtt_set_shift_temperature_topic);
    mqtt_client.subscribe(mqtt_set_mode_topic);
    mqtt_client.subscribe(mqtt_set_force_DHW_topic);
    mqtt_client.subscribe(mqtt_set_force_defrost_topic);
    mqtt_client.subscribe(mqtt_set_force_sterilization_topic);
    mqtt_client.subscribe(mqtt_set_holiday_topic);
    mqtt_client.subscribe(mqtt_set_powerfull_topic);
    mqtt_client.subscribe(mqtt_set_tank_temp_topic);
    mqtt_client.subscribe(mqtt_set_cool_temp_topic);

    mqtt_client.publish(mqtt_willtopic, "Online");
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

void logHex(char *hex, int hex_len) {
  if (outputHexDump) {
    char buffer [48];
    buffer[47] = 0;
    for (int i = 0; i < hex_len; i += 16) {
      for (int j = 0; ((j < 16) && ((i + j) < hex_len)); j++) {
        sprintf(&buffer[3 * j], "%02X ", hex[i + j]);
      }
      sprintf(log_msg, "data: %s", buffer ); log_message(log_msg);
    }
  }
}

byte calcChecksum(byte* command, int length) {
  byte chk = 0;
  for ( int i = 0; i < length; i++)  {
    chk += command[i];
  }
  chk = (chk ^ 0xFF) + 01;
  return chk;
}

bool readSerial()
{
  //heatpump data is always 203 bytes
  data_length = Serial.readBytes(data, 203);
  sprintf(log_msg, "received size : %d", data_length); log_message(log_msg);
  logHex(data, data_length);
  byte chk = 0;
  for ( int i = 0; i < sizeof(data); i++)  {
    chk += data[i];
  }
  if ( chk == 0 ) {
    log_message(strdup("Checksum received ok!"));
    return true;
  }
  else {
    log_message(strdup("Checksum received false!"));
    return false;
  }
}


bool send_command(byte* command, int length)
{
  if ( sending ) {
    log_message(strdup("Already sending data. Aborting this send request"));
    return false;
  }
  sending = true;

  byte chk = calcChecksum(command, length);
  int bytesSent = Serial.write(command, length);
  bytesSent += Serial.write(chk);

  sprintf(log_msg, "sent bytes: %d with checksum: %d ", bytesSent, int(chk)); log_message(log_msg);
  logHex((char*)command, length);
  sending = false;
  return true;
}

// Callback function that is called when a message has been pushed to one of your topics.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  msg[length] = '\0';
  send_heatpump_command(topic,msg,send_command,log_message);
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

void setupHttp() {
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.on("/", [] {
    handleRoot(&httpServer, &actData);
  });
  httpServer.on("/factoryreset", [] {
    handleFactoryReset(&httpServer);
  });
  httpServer.on("/reboot", [] {
    handleReboot(&httpServer);
  });
  httpServer.on("/togglelog", [] {
    log_message(strdup("Toggled mqtt log flag"));
    outputMqttLog ^= true;
    handleRoot(&httpServer, &actData);
  });
  httpServer.on("/togglehexdump", [] {
    log_message(strdup("Toggled hexdump log flag"));
    outputHexDump ^= true;
    handleRoot(&httpServer, &actData);
  });
  httpServer.begin();
}

void setupSerial() {
  //debug line on serial1 (D4, GPIO2)
  Serial1.begin(115200);

  //serial to cn-cnt
  Serial.begin(9600, SERIAL_8E1);
  Serial.flush();
  //swap to gpio13 (D7) and gpio15 (D8)
  Serial.swap();

  //enable gpio15 after boot using gpio5 (D1)
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
}

void setupMqtt() {
  mqtt_client.setServer(mqtt_server, atoi(mqtt_port));
  mqtt_client.setCallback(mqtt_callback);
}

void setup() {
  setupSerial();
  setupWifi(drd, wifi_hostname, ota_password, mqtt_server, mqtt_port, mqtt_username, mqtt_password);
  setupOTA();
  setupMqtt();
  setupHttp();
  //switchSerial();
}

void send_panasonic_query() {
  log_message(strdup("Requesting new panasonic data..."));
  send_command(panasonicQuery, PANASONICQUERYSIZE);
}

void read_panasonic_data() {
  if (Serial.available() > 0) {
    // read the serial
    if ( readSerial() ) decode_heatpump_data(data,actData,mqtt_client,log_message);
  }
}

void loop() {
  // Handle OTA first.
  ArduinoOTA.handle();
  // then handle HTTP
  httpServer.handleClient();

  if (!mqtt_client.connected())
  {
    mqtt_reconnect();
  }
  mqtt_client.loop();

  read_panasonic_data();

  // run the data query only each WAITTIME
  if (millis() > nexttime) {
    nexttime = millis() + WAITTIME;
    send_panasonic_query();
  }
}
