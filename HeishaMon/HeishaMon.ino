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

#include "commands.h"
#include "webfunctions.h"


// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 0.1

// address to the block in the RTC user memory
// change it if it collides with another usage
// of the address block
#define DRD_ADDRESS 0x00

#define WAITTIME 5000
#define UPDATEALLTIME 300000 // how often all data is cleared and so resend to mqtt

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "heisha";

// Default settings if config does not exists
char* wifi_hostname = "HeishaMon";
char* ota_password  = "panasonic";
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[40];
char mqtt_password[40];

bool sending = false;
unsigned long nexttime = 0;
unsigned long nextalldatatime = 0;

byte inCheck = 0;


//useful for debugging, outputs info to a separate mqtt topic
bool outputMqttLog = true;
//toggle to dump received hex data in log
bool outputHexDump = false;

//retain mqtt values for subscriber to receive on first connect
const bool MQTT_RETAIN_VALUES = true;


// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 256
char data[MAXDATASIZE];
int data_length = 0;

// store actual data in a json doc
DynamicJsonDocument actData(1024);

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
    log_message("Checksum received ok!");
    return true;
  }
  else {
    log_message("Checksum received false!");
    return false;
  }
}


bool send_command(byte* command, int length)
{
  if ( sending ) {
    log_message("Already sending data. Aborting this send request");
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
  // set 0 for Off mode, set 1 for Quiet mode 1, set 2 for Quiet mode 2, set 3 for Quiet mode 3
  if (strcmp(topic, mqtt_set_quiet_mode_topic) == 0)
  {
    String set_quiet_mode_string(msg);
    int quiet_mode = (set_quiet_mode_string.toInt() + 1) * 8;

    sprintf(log_msg, "set Quiet mode to %d", quiet_mode / 8 - 1); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, quiet_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }
  // set from -5 to 5 to get same temperature shift point
  if (strcmp(topic, mqtt_set_shift_temperature_topic) == 0)
  {
    String set_shift_temperature_string(msg);
    int shift_mode = set_shift_temperature_string.toInt() + 128;


    sprintf(log_msg, "set shift temperature to %d", shift_mode - 128 ); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, shift_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set mode to force DHW by sending 1
  if (strcmp(topic, mqtt_set_force_DHW_topic) == 0)
  {
    String set_force_DHW_string(msg);
    int force_DHW_mode = 66; //hex 0x42
    if ( set_force_DHW_string.toInt() == 1 ) {
      force_DHW_mode = 130; //hex 0x82
    }
    sprintf(log_msg, "set force mode to %d", force_DHW_mode); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, force_DHW_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set mode to force defrost  by sending 1
  if (strcmp(topic, mqtt_set_force_defrost_topic) == 0)
  {
    String set_force_defrost_string(msg);
    int force_defrost_mode = 0;
    if ( set_force_defrost_string.toInt() == 1 ) {
      force_defrost_mode = 2; //hex 0x02
    }
    sprintf(log_msg, "set force defrost mode to %d", force_defrost_mode); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, force_defrost_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set mode to force sterilization by sending 1
  if (strcmp(topic, mqtt_set_force_sterilization_topic) == 0)
  {
    String set_force_sterilization_string(msg);
    int force_sterilization_mode = 0;
    if ( set_force_sterilization_string.toInt() == 1 ) {
      force_sterilization_mode = 4; //hex 0x04
    }
    sprintf(log_msg, "set force sterilization mode to %d", force_sterilization_mode); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, force_sterilization_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }


  // set Holiday mode by sending 1, off will be 0
  if (strcmp(topic, mqtt_set_holiday_topic) == 0)
  {
    String set_holiday_string(msg);

    int set_holiday = 84; //hex 0x54
    if ( set_holiday_string.toInt() == 1 ) {
      set_holiday = 100; //hex 0x64
    }

    sprintf(log_msg, "set holiday mode to %d", set_holiday); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, set_holiday, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set Powerfull mode by sending 0 = off, 1 for 30min, 2 for 60min, 3 for 90 min
  if (strcmp(topic, mqtt_set_powerfull_topic) == 0)
  {
    String set_powerfull_string(msg);
    int set_powerfull = (set_powerfull_string.toInt() ) + 73;

    sprintf(log_msg, "set powerfull mode to %d", (set_powerfull - 73) ); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, set_powerfull, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set Heat pump mode  3 = tank only, 0 = heat only, 1 = cool only, 2 = Auto, 4 = Heat+DHW, 5 = Cool+DHW, 6 = Auto + DHW
  if (strcmp(topic, mqtt_set_mode_topic) == 0)
  {
    String set_mode_string(msg);
    int set_mode;
    switch (set_mode_string.toInt()) {
      case 0: set_mode = 33; break;
      case 1: set_mode = 82; break;
      case 2: set_mode = 83; break;
      case 3: set_mode = 88; break;
      case 4: set_mode = 98; break;
      case 5: set_mode = 99; break;
      case 6: set_mode = 104; break;
      default: set_mode = 0; break;
    }

    sprintf(log_msg, "set heat pump mode to %d", set_mode); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x02, 0x00, set_mode, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set Tank temperature by sending desired temperature between 40C-75C
  if (strcmp(topic, mqtt_set_tank_temp_topic) == 0)
  {
    String set_tank_temp_string(msg);
    int set_tank_temp = set_tank_temp_string.toInt() + 128;

    sprintf(log_msg, "set Tank temperature to %d", set_tank_temp - 128); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, set_tank_temp, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  // set Cool temperature by sending desired temperature between 5C-20C (if selected Direct mode)
  if (strcmp(topic, mqtt_set_cool_temp_topic) == 0)
  {
    String set_cool_temp_string(msg);
    int set_cool_temp = set_cool_temp_string.toInt() + 128;


    sprintf(log_msg, "set Cool temperature to %d", set_cool_temp - 128); log_message(log_msg);
    byte command[] = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, set_cool_temp, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    send_command(command, sizeof(command));
  }

  if (strcmp(topic, mqtt_topic_base) == 0)
  {
    log_message("Updating..");

    send_panasonic_query();
  }
}

void decode_heatpump_data() {
  if (millis() > nextalldatatime) {
    actData.clear(); // clearing all actual data so everything will be updated and sent to mqtt
    nextalldatatime = millis() + UPDATEALLTIME;
  }

  int Power_State = (int)(data[4]);
  char* Power_State_string;
  switch (Power_State & 0b11) { //probably only last two bits for Power dhw state
    case 0b01:
      Power_State_string = "0";
      break;
    case 0b10:
      Power_State_string = "1";
      break;
    default:
      Power_State_string = "-1";
      break;
  }

  // TOP3 //
  if ( actData["Power_State"] != Power_State_string ) {
    actData["Power_State"] = Power_State_string;
    sprintf(log_msg, "received Power state : %d (%s)", Power_State, Power_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Power_State"); mqtt_client.publish(mqtt_topic, Power_State_string, MQTT_RETAIN_VALUES);
  }

  int Mode_State = (int)(data[6]);
  char* Mode_State_string;
  switch (Mode_State) {
    case 82:
      Mode_State_string = "0";
      break;
    case 83:
      Mode_State_string = "1";
      break;
    case 89:
      Mode_State_string = "2";
      break;
    case 97:
      Mode_State_string = "3";
      break;
    case 98:
      Mode_State_string = "4";
      break;
    case 99:
      Mode_State_string = "5";
      break;
    case 105:
      Mode_State_string = "6";
      break;
    default:
      Mode_State_string = "-1";
      break;
  }

  // TOP4 //
  if ( actData["OpMode_State"] != Mode_State_string ) {
    actData["OpMode_State"] = Mode_State_string;
    sprintf(log_msg, "received OpMode state : %d (%s)", Mode_State, Mode_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "OpMode_State"); mqtt_client.publish(mqtt_topic, Mode_State_string, MQTT_RETAIN_VALUES);
  }


  int quietpower_Mode_State = (int)(data[7]);
  char* Powerfull_Mode_State_string = "-1";
  char* Quiet_Mode_State_string = "-1";
  switch (quietpower_Mode_State & 0b11111000) { // only interested in left most 5 bits for quiet state
    case 0b10001000:
      Quiet_Mode_State_string = "4";
      break;
    case 0b01001000:
      Quiet_Mode_State_string = "0";
      break;
    case 0b01010000:
      Quiet_Mode_State_string = "1";
      break;
    case 0b01011000:
      Quiet_Mode_State_string = "2";
      break;
    case 0b01100000:
      Quiet_Mode_State_string = "3";
      break;
    default:
      break;
  }
  switch (quietpower_Mode_State & 0b111) { // only interested in last 3 bits for powerfull state
    case 0b001:
      Powerfull_Mode_State_string = "0";
      break;
    case 0b010:
      Powerfull_Mode_State_string = "1";
      break;
    case 0b011:
      Powerfull_Mode_State_string = "2";
      break;
    case 0b100:
      Powerfull_Mode_State_string = "3";
      break;
    default:
      break;
  }
  // TOP18 //
  if ( actData["Quietmode_Level"] != Quiet_Mode_State_string ) {
    actData["Quietmode_Level"] = Quiet_Mode_State_string;
    sprintf(log_msg, "received quietmode level : %d (%s)", quietpower_Mode_State, Quiet_Mode_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Quietmode_Level"); mqtt_client.publish(mqtt_topic, Quiet_Mode_State_string, MQTT_RETAIN_VALUES);
  }
  // TOP17 //
  if ( actData["Powerfullmode_State"] != Powerfull_Mode_State_string ) {
    actData["Powerfullmode_State"] = Powerfull_Mode_State_string;
    sprintf(log_msg, "received powerfullmode state : %d (%s)", quietpower_Mode_State, Powerfull_Mode_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Powerfullmode_State"); mqtt_client.publish(mqtt_topic, Powerfull_Mode_State_string, MQTT_RETAIN_VALUES);
  }

  int valve_defrost_State = (int)(data[111]);
  char* Valve_State_string;
  switch (valve_defrost_State & 0b11) { //bitwise AND with 0b11 because we are only interested in last 2 bits of the byte.
    case 0b01:
      Valve_State_string = "0";
      break;
    case 0b10:
      Valve_State_string = "1";
      break;
    default:
      Valve_State_string = "-1";
      break;
  }

  // TOP20//
  if ( actData["Valve_State"] != Valve_State_string ) {
    actData["Valve_State"] = Valve_State_string;
    sprintf(log_msg, "received 3-way valve state : %d (%s)", valve_defrost_State, Valve_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Valve_State"); mqtt_client.publish(mqtt_topic, Valve_State_string, MQTT_RETAIN_VALUES);
  }

  char* Defrosting_State_string;
  switch (valve_defrost_State & 0b1100) { //bitwise AND with 0b1100 because we are only interested in these two bits
    case 0b0100:
      Defrosting_State_string = "0";
      break;
    case 0b1000:
      Defrosting_State_string = "1";
      break;
    default:
      Defrosting_State_string = "-1";
      break;
  }

  // TOP26 //
  if ( actData["Defrosting_State"] != Defrosting_State_string ) {
    actData["Defrosting_State"] = Defrosting_State_string;
    sprintf(log_msg, "received defrosting state : %d (%s)", valve_defrost_State, Defrosting_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Defrosting_State"); mqtt_client.publish(mqtt_topic, Defrosting_State_string, MQTT_RETAIN_VALUES);
  }

  // TOP7 //
  float Flow_Target_Temp = (float)data[153] - 128;
  if ( actData["Flow_Target_Temp"] != Flow_Target_Temp ) {
    actData["Flow_Target_Temp"] = Flow_Target_Temp;
    sprintf(log_msg, "received temperature (Flow_Target_Temp): %.2f", Flow_Target_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Flow_Target_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Target_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP6 //
  float Flow_Outlet_Temp = (float)data[139] - 128;
  if ( actData["Flow_Outlet_Temp"] != Flow_Outlet_Temp ) {
    actData["Flow_Outlet_Temp"] = Flow_Outlet_Temp;
    sprintf(log_msg, "received temperature (Flow_Outlet_Temp): %.2f", Flow_Outlet_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Flow_Outlet_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Outlet_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP5 //
  float Flow_Inlet_Temp = (float)data[143] - 128;
  if ( actData["Flow_Inlet_Temp"] != Flow_Inlet_Temp ) {
    actData["Flow_Inlet_Temp"] = Flow_Inlet_Temp;
    sprintf(log_msg, "received temperature (Flow_Inlet_Temp): %.2f", Flow_Inlet_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Flow_Inlet_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Inlet_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP9 //
  float Tank_Target_Temp = (float)data[42] - 128;
  if ( actData["Tank_Target_Temp"] != Tank_Target_Temp ) {
    actData["Tank_Target_Temp"] = Tank_Target_Temp;
    sprintf(log_msg, "received temperature (Tank_Target_Temp): %.2f", Tank_Target_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Target_Temp"); mqtt_client.publish(mqtt_topic, String(Tank_Target_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP10 //
  float Tank_Temp = (float)data[141] - 128;
  if ( actData["Tank_Temp"] != Tank_Temp ) {
    actData["Tank_Temp"] = Tank_Temp;
    sprintf(log_msg, "received temperature (Tank_Temp): %.2f", Tank_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Temp"); mqtt_client.publish(mqtt_topic, String(Tank_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP14 //
  float Outside_Temp = (float)data[142] - 128;
  if ( actData["Outside_Temp"] != Outside_Temp ) {
    actData["Outside_Temp"] = Outside_Temp;
    sprintf(log_msg, "received temperature (Outside_Temp): %.2f", Outside_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Outside_Temp"); mqtt_client.publish(mqtt_topic, String(Outside_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP33 //
  float Room_Temp = (float)data[156] - 128;
  if ( actData["Room_Temp"] != Room_Temp ) {
    actData["Room_Temp"] = Room_Temp;
    sprintf(log_msg, "received temperature (Room_Temp): %.2f", Room_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Room_Temp"); mqtt_client.publish(mqtt_topic, String(Room_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP21 //
  float Outside_Pipe_Temp = (float)data[158] - 128;
  if ( actData["Outside_Pipe_Temp"] != Outside_Pipe_Temp ) {
    actData["Outside_Pipe_Temp"] = Outside_Pipe_Temp;
    sprintf(log_msg, "received temperature (Outside_Pipe_Temp): %.2f", Outside_Pipe_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Outside_Pipe_Temp"); mqtt_client.publish(mqtt_topic, String(Outside_Pipe_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP1 //
  int PumpFlow1 = (int)data[170];
  float PumpFlow2 = ((((float)data[169] - 1) / 5) * 2) / 100;
  float PumpFlow = PumpFlow1 + PumpFlow2;
  if ( actData["Pump_Flow"] != PumpFlow ) {
    actData["Pump_Flow"] = PumpFlow;
    sprintf(log_msg, "received pump flow (Pump_Flow): %.2f", PumpFlow); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Pump_Flow"); mqtt_client.publish(mqtt_topic, String(PumpFlow).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP8 //
  float CompFreq = (float)data[166] - 1;
  if ( actData["Compressor_Freq"] != CompFreq ) {
    actData["Compressor_Freq"] = CompFreq;
    sprintf(log_msg, "received compressor frequency (Compressor_Freq): %.2f", CompFreq); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Compressor_Freq"); mqtt_client.publish(mqtt_topic, String(CompFreq).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP13 //
  float Heat_Shift_Temp = (float)data[38] - 128;
  if ( actData["HeatShift_Temp"] != Heat_Shift_Temp ) {
    actData["HeatShift_Temp"] = Heat_Shift_Temp;
    sprintf(log_msg, "received temperature (HeatShift_Temp): %.2f", Heat_Shift_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HeatShift_Temp"); mqtt_client.publish(mqtt_topic, String(Heat_Shift_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP28 //
  float Cool_Shift_Temp = (float)data[39] - 128;
  if ( actData["CoolShift_Temp"] != Cool_Shift_Temp ) {
    actData["CoolShift_Temp"] = Cool_Shift_Temp;
    sprintf(log_msg, "received temperature (CoolShift_Temp): %.2f", Cool_Shift_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Cool_Shift_Temp"); mqtt_client.publish(mqtt_topic, String(Cool_Shift_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP29 //
  float HCurveOutHighTemp = (float)data[75] - 128;
  if ( actData["HCurve_OutHighTemp"] != HCurveOutHighTemp ) {
    actData["HCurve_OutHighTemp"] = HCurveOutHighTemp;
    sprintf(log_msg, "received temperature (HCurve_OutHighTemp): %.2f", HCurveOutHighTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutHighTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP30 //
  float HCurveOutLowTemp = (float)data[76] - 128;
  if ( actData["HCurve_OutLowTemp"] != HCurveOutLowTemp ) {
    actData["HCurve_OutLowTemp"] = HCurveOutLowTemp;
    sprintf(log_msg, "received temperature (HCurve_OutLowTemp): %.2f", HCurveOutLowTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutLowTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP32 //
  float HCurveOutsLowTemp = (float)data[77] - 128;
  if ( actData["HCurve_OutsLowTemp"] != HCurveOutsLowTemp ) {
    actData["HCurve_OutsLowTemp"] = HCurveOutsLowTemp;
    sprintf(log_msg, "received temperature (HCurve_OutsLowTemp): %.2f", HCurveOutsLowTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutsLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsLowTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP31 //
  float HCurveOutsHighTemp = (float)data[78] - 128;
  if ( actData["HCurve_OutsHighTemp"] != HCurveOutsHighTemp ) {
    actData["HCurve_OutsHighTemp"] = HCurveOutsHighTemp;
    sprintf(log_msg, "received temperature (HCurve_OutsHighTemp): %.2f", HCurveOutsHighTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutsHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsHighTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  int ForceDHW_State = (int)(data[4]);
  char* ForceDHW_State_string;
  switch (ForceDHW_State & 0b11000000) { //probably only first two bits for force dhw state
    case 0b01000000:
      ForceDHW_State_string = "0";
      break;
    case 0b10000000:
      ForceDHW_State_string = "1";
      break;
    default:
      ForceDHW_State_string = "-1";
      break;
  }

  // TOP2 //
  if ( actData["ForceDHW_State"] != ForceDHW_State_string ) {
    actData["ForceDHW_State"] = ForceDHW_State_string;
    sprintf(log_msg, "received force DHW state : %d (%s)", ForceDHW_State, ForceDHW_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ForceDHW"); mqtt_client.publish(mqtt_topic, ForceDHW_State_string, MQTT_RETAIN_VALUES);
  }

  int Holiday_Mode_State = (int)(data[5]);
  char* Holiday_Mode_State_string;
  switch (Holiday_Mode_State & 0b00110000) { //probably only these two bits determine holiday state
    case 0b00010000:
      Holiday_Mode_State_string = "0";
      break;
    case 0b00100000:
      Holiday_Mode_State_string = "1";
      break;
    default:
      Holiday_Mode_State_string = "-1";
      break;
  }

  // TOP19 //
  if ( actData["Holidaymode_State"] != Holiday_Mode_State_string ) {
    actData["Holidaymode_State"] = Holiday_Mode_State_string;
    sprintf(log_msg, "received Holidaymode state : %d (%s)", Holiday_Mode_State, Holiday_Mode_State_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Holidaymode_State"); mqtt_client.publish(mqtt_topic, Holiday_Mode_State_string, MQTT_RETAIN_VALUES);
  }

  // TOP23 //
  float Heat_Delta = (float)data[84] - 128;
  if ( actData["Heat_Delta"] != Heat_Delta ) {
    actData["Heat_Delta"] = Heat_Delta;
    sprintf(log_msg, "received temperature (Heat_Delta): %.2f", Heat_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Delta"); mqtt_client.publish(mqtt_topic, String(Heat_Delta).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP24 //
  float Cool_Delta = (float)data[94] - 128;
  if ( actData["Cool_Delta"] != Cool_Delta ) {
    actData["Cool_Delta"] = Cool_Delta;
    sprintf(log_msg, "received temperature (Cool_Delta): %.2f", Cool_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Cool_Delta"); mqtt_client.publish(mqtt_topic, String(Cool_Delta).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP22 //
  float Tank_Heat_Delta = (float)data[99] - 128;
  if ( actData["Tank_Heat_Delta"] != Tank_Heat_Delta ) {
    actData["Tank_Heat_Delta"] = Tank_Heat_Delta;
    sprintf(log_msg, "received temperature (Tank_Heat_Delta): %.2f", Tank_Heat_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Heat_Delta"); mqtt_client.publish(mqtt_topic, String(Tank_Heat_Delta).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP16 //
  float Energy_Consumtion = ((float)data[193] - 1.0) * 200;
  if ( actData["Heat_Energy_Consumtion"] != Energy_Consumtion ) {
    actData["Heat_Energy_Consumtion"] = Energy_Consumtion;
    sprintf(log_msg, "received Watt (Heat_Energy_Consumtion): %.2f", Energy_Consumtion); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Energy_Consumtion"); mqtt_client.publish(mqtt_topic, String(Energy_Consumtion).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP15 //
  float Energy_Production = ((float)data[194] - 1.0) * 200;
  if ( actData["Heat_Energy_Production"] != Energy_Production ) {
    actData["Heat_Energy_Production"] = Energy_Production;
    sprintf(log_msg, "received Watt (Heat_Energy_Production): %.2f", Energy_Production); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Energy_Production"); mqtt_client.publish(mqtt_topic, String(Energy_Production).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP11 //
  int Operations_Hours  = word(data[183], data[182]) - 1;
  if ( actData["Operations_Hours"] != Operations_Hours ) {
    actData["Operations_Hours"] = Operations_Hours;
    sprintf(log_msg, "received (Operations_Hours): %.2f", Operations_Hours); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Operations_Hours"); mqtt_client.publish(mqtt_topic, String(Operations_Hours).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP12 //
  int Operations_Counter  = word(data[180], data[179]) - 1;
  if ( actData["Operations_Counter"] != Operations_Counter ) {
    actData["Operations_Counter"] = Operations_Counter;
    sprintf(log_msg, "received (Operations_Counter): %.2f", Operations_Counter); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Operations_Counter"); mqtt_client.publish(mqtt_topic, String(Operations_Counter).c_str(), MQTT_RETAIN_VALUES);
  }

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
    log_message("Toggled mqtt log flag");
    outputMqttLog ^= true;
    handleRoot(&httpServer, &actData);
  });
  httpServer.on("/togglehexdump", [] {
    log_message("Toggled hexdump log flag");
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
  log_message("Requesting new panasonic data...");
  send_command(panasonicQuery, sizeof(panasonicQuery));
}

void read_panasonic_data() {
  if (Serial.available() > 0) {
    // read the serial
    if ( readSerial() ) decode_heatpump_data();
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
