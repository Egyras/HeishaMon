#define LWIP_INTERNAL

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <DoubleResetDetect.h>

#include <ArduinoJson.h>

#include "src/common/timerqueue.h"
#include "webfunctions.h"
#include "decode.h"
#include "commands.h"

DNSServer dnsServer;

//to read bus voltage in stats
ADC_MODE(ADC_VCC);

// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 0.1

// address to the block in the RTC user memory
// change it if it collides with another usageb
// of the address block
#define DRD_ADDRESS 0x00

const byte DNS_PORT = 53;

#define SERIALTIMEOUT 2000 // wait until all 203 bytes are read, must not be too long to avoid blocking the code

WebSocketsServer webSocket = WebSocketsServer(81);

settingsStruct heishamonSettings;

bool sending = false; // mutex for sending data
bool mqttcallbackinprogress = false; // mutex for processing mqtt callback

#define MQTTRECONNECTTIMER 30000 //it takes 30 secs for each mqtt server reconnect attempt
unsigned long lastMqttReconnectAttempt = 0;

#define WIFIRETRYTIMER 15000 // switch between hotspot and configured SSID each 10 secs if SSID is lost
unsigned long lastWifiRetryTimer = 0;

unsigned long lastRunTime = 0;

unsigned long sendCommandReadTime = 0; //set to millis value during send, allow to wait millis for answer
unsigned long goodreads = 0;
unsigned long totalreads = 0;
unsigned long badcrcread = 0;
unsigned long badheaderread = 0;
unsigned long tooshortread = 0;
unsigned long toolongread = 0;
unsigned long timeoutread = 0;
float readpercentage = 0;
static int uploadpercentage = 0;

// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 255
char data[MAXDATASIZE] = { '\0' };
byte  data_length = 0;

// store actual data in an String array
String actData[NUMBER_OF_TOPICS];
String actOptData[NUMBER_OF_OPT_TOPICS];
String RESTmsg = "";

// log message to sprintf to
char log_msg[256];

// mqtt topic to sprintf and then publish to
char mqtt_topic[256];

static int mqttReconnects = 0;

// can't have too much in buffer due to memory shortage
#define MAXCOMMANDSINBUFFER 10

// buffer for commands to send
struct cmdbuffer_t {
  uint8_t length;
  byte data[128];
} cmdbuffer[MAXCOMMANDSINBUFFER];

static uint8_t cmdstart = 0;
static uint8_t cmdend = 0;
static uint8_t cmdnrel = 0;

//doule reset detection
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// mqtt
WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);

bool firstConnectSinceBoot = true; //if this is true there is no first connection made yet

struct timerqueue_t **timerqueue = NULL;
int timerqueue_size = 0;

/*
    check_wifi will process wifi reconnecting managing
*/
void check_wifi()
{
  if ((WiFi.status() != WL_CONNECTED) || (!WiFi.localIP()))  {
    /*
        if we are not connected to an AP
        we must be in softAP so respond to DNS
    */
    dnsServer.processNextRequest();

    /* we need to stop reconnecting to a configured wifi network if there is a hotspot user connected
        also, do not disconnect if wifi network scan is active
    */
    if ((heishamonSettings.wifi_ssid[0] != '\0') && (WiFi.status() != WL_DISCONNECTED) && (WiFi.scanComplete() != -1) && (WiFi.softAPgetStationNum() > 0))  {
      log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
      WiFi.disconnect(true);
    }

    /*  only start this routine if timeout on
        reconnecting to AP and SSID is set
    */
    if ((heishamonSettings.wifi_ssid[0] != '\0') && ((unsigned long)(millis() - lastWifiRetryTimer) > WIFIRETRYTIMER ) )  {
      lastWifiRetryTimer = millis();
      if (WiFi.softAPSSID() == "") {
        log_message((char *)"WiFi lost, starting setup hotspot...");
        WiFi.softAP((char*)"HeishaMon-Setup");
      }
      if ((WiFi.status() == WL_DISCONNECTED)  && (WiFi.softAPgetStationNum() == 0 )) {
        log_message((char *)"Retrying configured WiFi, ...");
        if (heishamonSettings.wifi_password[0] == '\0') {
          WiFi.begin(heishamonSettings.wifi_ssid);
        } else {
          WiFi.begin(heishamonSettings.wifi_ssid, heishamonSettings.wifi_password);
        }
      } else {
        log_message((char *)"Reconnecting to WiFi failed. Waiting a few seconds before trying again.");
        WiFi.disconnect(true);
      }
    }
  } else { //WiFi connected
    if (WiFi.softAPSSID() != "") {
      log_message((char *)"WiFi (re)connected, shutting down hotspot...");
      WiFi.softAPdisconnect(true);
      MDNS.notifyAPChange();

    }

    if (firstConnectSinceBoot) { // this should start only when softap is down or else it will not work properly so run after the routine to disable softap
      firstConnectSinceBoot = false;
      lastMqttReconnectAttempt = 0; //initiate mqtt connection asap
      setupOTA();
      MDNS.begin(heishamonSettings.wifi_hostname);
      MDNS.addService("http", "tcp", 80);
      experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); //necessary for some users with bad wifi routers

      if (heishamonSettings.wifi_ssid[0] == '\0') {
        log_message((char *)"WiFi connected without SSID and password in settings. Must come from persistent memory. Storing in settings.");
        WiFi.SSID().toCharArray(heishamonSettings.wifi_ssid, 40);
        WiFi.psk().toCharArray(heishamonSettings.wifi_password, 40);
        DynamicJsonDocument jsonDoc(1024);
        settingsToJson(jsonDoc, &heishamonSettings); //stores current settings in a json document
        saveJsonToConfig(jsonDoc); //save to config file
      }
    }

    /*
       always update if wifi is working so next time on ssid failure
       it only starts the routine above after this timeout
    */
    lastWifiRetryTimer = millis();

    // Allow MDNS processing
    MDNS.update();
  }
}

void mqtt_reconnect()
{
  unsigned long now = millis();
  if ((unsigned long)(now - lastMqttReconnectAttempt) > MQTTRECONNECTTIMER) { //only try reconnect each MQTTRECONNECTTIMER seconds or on boot when lastMqttReconnectAttempt is still 0
    lastMqttReconnectAttempt = now;
    log_message((char*)"Reconnecting to mqtt server ...");
    char topic[256];
    sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
    if (mqtt_client.connect(heishamonSettings.wifi_hostname, heishamonSettings.mqtt_username, heishamonSettings.mqtt_password, topic, 1, true, "Offline"))
    {
      mqttReconnects++;

      sprintf(topic, "%s/%s/#", heishamonSettings.mqtt_topic_base, mqtt_topic_commands);
      mqtt_client.subscribe(topic);
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_send_raw_value_topic);
      mqtt_client.subscribe(topic);
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
      mqtt_client.publish(topic, "Online");
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_iptopic);
      mqtt_client.publish(topic, WiFi.localIP().toString().c_str(), true);

      if (heishamonSettings.use_s0) { // connect to s0 topic to retrieve older watttotal from mqtt
        sprintf_P(mqtt_topic, PSTR("%s/%s/WatthourTotal/1"), heishamonSettings.mqtt_topic_base, mqtt_topic_s0);
        mqtt_client.subscribe(mqtt_topic);
        sprintf_P(mqtt_topic, PSTR("%s/%s/WatthourTotal/2"), heishamonSettings.mqtt_topic_base, mqtt_topic_s0);
        mqtt_client.subscribe(mqtt_topic);
      }
      if (heishamonSettings.use_1wire) resetlastalldatatime_dallas; //resend all 1wire values to mqtt
      resetlastalldatatime; //resend all heatpump values to mqtt
    }
  }
}

void log_message(char* string)
{
  if (heishamonSettings.logSerial1) {
    Serial1.print(millis());
    Serial1.print(": ");
    Serial1.println(string);
  }
  if (heishamonSettings.logMqtt && mqtt_client.connected())
  {
    char log_topic[256];
    sprintf(log_topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_logtopic);

    if (!mqtt_client.publish(log_topic, string)) {
      Serial1.print(millis());
      Serial1.print(F(": "));
      Serial1.println(F("MQTT publish log message failed!"));
      mqtt_client.disconnect();
    }
  }
  if (webSocket.connectedClients() > 0) {
    webSocket.broadcastTXT(string, strlen(string));
  }
}

void logHex(char *hex, byte hex_len) {
#define LOGHEXBYTESPERLINE 32  // please be aware of max mqtt message size
  for (int i = 0; i < hex_len; i += LOGHEXBYTESPERLINE) {
    char buffer [(LOGHEXBYTESPERLINE * 3) + 1];
    buffer[LOGHEXBYTESPERLINE * 3] = '\0';
    for (int j = 0; ((j < LOGHEXBYTESPERLINE) && ((i + j) < hex_len)); j++) {
      sprintf(&buffer[3 * j], "%02X ", hex[i + j]);
    }
    sprintf(log_msg, "data: %s", buffer ); log_message(log_msg);
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

bool isValidReceiveChecksum() {
  byte chk = 0;
  for ( int i = 0; i < data_length; i++)  {
    chk += data[i];
  }
  return (chk == 0); //all received bytes + checksum should result in 0
}

bool readSerial()
{
  if (data_length == 0 ) totalreads++; //this is the start of a new read

  while ((Serial.available()) && (data_length < MAXDATASIZE)) {
    data[data_length] = Serial.read(); //read available data and place it after the last received data
    data_length++;
    if (data[0] != 113) { //wrong header received!
      log_message((char*)"Received bad header. Ignoring this data!");
      if (heishamonSettings.logHexdump) logHex(data, data_length);
      badheaderread++;
      data_length = 0;
      return false; //return so this while loop does not loop forever if there happens to be a continous invalid data stream
    }
  }

  if (data_length > 1) { //should have received length part of header now

    if ((data_length > (data[1] + 3)) || (data_length >= MAXDATASIZE) ) {
      log_message((char*)"Received more data than header suggests! Ignoring this as this is bad data.");
      if (heishamonSettings.logHexdump) logHex(data, data_length);
      data_length = 0;
      toolongread++;
      return false;
    }

    if (data_length == (data[1] + 3)) { //we received all data (data[1] is header length field)
      sprintf(log_msg, "Received %d bytes data", data_length); log_message(log_msg);
      sending = false; //we received an answer after our last command so from now on we can start a new send request again
      if (heishamonSettings.logHexdump) logHex(data, data_length);
      if (! isValidReceiveChecksum() ) {
        log_message((char*)"Checksum received false!");
        data_length = 0; //for next attempt
        badcrcread++;
        return false;
      }
      log_message((char*)"Checksum and header received ok!");
      goodreads++;

      if (data_length == 203) { //for now only return true for this datagram because we can not decode the shorter datagram yet
        data_length = 0;
        decode_heatpump_data(data, actData, mqtt_client, log_message, heishamonSettings.mqtt_topic_base, heishamonSettings.updateAllTime);
        return true;
      }
      else if (data_length == 20 ) { //optional pcb acknowledge answer
        log_message((char*)"Received optional PCB ack answer. Decoding this in OPT topics.");
        data_length = 0;
        decode_optional_heatpump_data(data, actOptData, mqtt_client, log_message, heishamonSettings.mqtt_topic_base, heishamonSettings.updateAllTime);
        return true;
      }
      else {
        log_message((char*)"Received a shorter datagram. Can't decode this yet.");
        data_length = 0;
        return false;
      }
    }
  }
  return false;
}

void popCommandBuffer() {
  // to make sure we can pop a command from the buffer
  if ((!sending) && cmdnrel > 0) {
    send_command(cmdbuffer[cmdstart].data, cmdbuffer[cmdstart].length);
    cmdstart = (cmdstart + 1) % (MAXCOMMANDSINBUFFER);
    cmdnrel--;
  }
}

void pushCommandBuffer(byte* command, int length) {
  if (cmdnrel + 1 > MAXCOMMANDSINBUFFER) {
    log_message((char *)"Too much commands already in buffer. Ignoring this commands.\n");
    return;
  }
  cmdbuffer[cmdend].length = length;
  memcpy(&cmdbuffer[cmdend].data, command, length);
  cmdend = (cmdend + 1) % (MAXCOMMANDSINBUFFER);
  cmdnrel++;
}

bool send_command(byte* command, int length) {
  if ( heishamonSettings.listenonly ) {
    log_message((char*)"Not sending this command. Heishamon in listen only mode!");
    return false;
  }
  if ( sending ) {
    log_message((char*)"Already sending data. Buffering this send request");
    pushCommandBuffer(command, length);
    return false;
  }
  sending = true; //simple semaphore to only allow one send command at a time, semaphore ends when answered data is received

  byte chk = calcChecksum(command, length);
  int bytesSent = Serial.write(command, length); //first send command
  bytesSent += Serial.write(chk); //then calculcated checksum byte afterwards
  sprintf_P(log_msg, PSTR("sent bytes: %d including checksum value: %d "), bytesSent, int(chk));
  log_message(log_msg);

  if (heishamonSettings.logHexdump) logHex((char*)command, length);
  sendCommandReadTime = millis(); //set sendCommandReadTime when to timeout the answer of this command
  return true;
}

// Callback function that is called when a message has been pushed to one of your topics.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (mqttcallbackinprogress) {
    log_message((char*)"Already processing another mqtt callback. Ignoring this one");
  }
  else {
    mqttcallbackinprogress = true; //simple semaphore to make sure we don't have two callbacks at the same time
    char msg[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      msg[i] = (char)payload[i];
    }
    msg[length] = '\0';
    char* topic_command = topic + strlen(heishamonSettings.mqtt_topic_base) + 1; //strip base plus seperator from topic
    if (strcmp(topic_command, mqtt_send_raw_value_topic) == 0)
    { // send a raw hex string
      byte *rawcommand;
      rawcommand = (byte *) malloc(length);
      memcpy(rawcommand, msg, length);

      sprintf_P(log_msg, PSTR("sending raw value"));
      log_message(log_msg);
      send_command(rawcommand, length);
      free(rawcommand);
    } else if (strncmp(topic_command, mqtt_topic_s0, 2) == 0)  // this is a s0 topic, check for watthour topic and restore it
    {
      char* topic_s0_watthour_port = topic_command + 17; //strip the first 17 "s0/WatthourTotal/" from the topic to get the s0 port
      int s0Port = String(topic_s0_watthour_port).toInt();
      float watthour = String(msg).toFloat();
      restore_s0_Watthour(s0Port, watthour);
      //unsubscribe after restoring the watthour values
      char mqtt_topic[256];
      sprintf(mqtt_topic, "%s", topic);
      if (mqtt_client.unsubscribe(mqtt_topic)) {
        log_message((char*)"Unsubscribed from S0 watthour restore topic");
      }
    } else if (strncmp(topic_command, mqtt_topic_commands, 8) == 0)  // check for optional pcb commands
    {
      char* topic_sendcommand = topic_command + 9; //strip the first 9 "commands/" from the topic to get what we need
      send_heatpump_command(topic_sendcommand, msg, send_command, log_message, heishamonSettings.optionalPCB);
    }
    mqttcallbackinprogress = false;
  }
}

void setupOTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(heishamonSettings.wifi_hostname);

  // Set authentication
  ArduinoOTA.setPassword(heishamonSettings.ota_password);

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

int8_t webserver_cb(struct webserver_t *client, void *dat) {
  switch (client->step) {
    case WEBSERVER_CLIENT_REQUEST_METHOD: {
        if (strcmp((char *)dat, "POST") == 0) {
          client->route = 110;
        }
        return 0;
      } break;
    case WEBSERVER_CLIENT_REQUEST_URI: {
        if (strcmp((char *)dat, "/") == 0) {
          client->route = 1;
        } else if (strcmp((char *)dat, "/tablerefresh") == 0) {
          client->route = 10;
        } else if (strcmp((char *)dat, "/json") == 0) {
          client->route = 20;
        } else if (strcmp((char *)dat, "/reboot") == 0) {
          client->route = 30;
        } else if (strcmp((char *)dat, "/debug") == 0) {
          client->route = 40;
          log_message((char*)"Debug URL requested");
        } else if (strcmp((char *)dat, "/wifiscan") == 0) {
          client->route = 50;
        } else if (strcmp((char *)dat, "/togglelog") == 0) {
          client->route = 1;
          log_message((char*)"Toggled mqtt log flag");
          heishamonSettings.logMqtt ^= true;
        } else if (strcmp((char *)dat, "/togglehexdump") == 0) {
          client->route = 1;
          log_message((char*)"Toggled hexdump log flag");
          heishamonSettings.logHexdump ^= true;
        } else if (strcmp((char *)dat, "/hotspot-detect.html") == 0 ||
                   strcmp((char *)dat, "/fwlink") == 0 ||
                   strcmp((char *)dat, "/generate_204") == 0 ||
                   strcmp((char *)dat, "/gen_204") == 0 ||
                   strcmp((char *)dat, "/popup") == 0) {
          client->route = 80;
        } else if (strcmp((char *)dat, "/factoryreset") == 0) {
          client->route = 90;
        } else if (strcmp((char *)dat, "/command") == 0) {
          RESTmsg.clear();
          client->route = 100;
        } else if (client->route == 110) {
          // Only accept settings POST requests
          if (strcmp((char *)dat, "/savesettings") == 0) {
            client->route = 110;
          } else if (strcmp((char *)dat, "/firmware") == 0) {
            client->route = 150;

            Update.runAsync(true);
            if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
              Update.printError(Serial1);
            }
          } else {
            return -1;
          }
        } else if (strcmp((char *)dat, "/settings") == 0) {
          client->route = 120;
        } else if (strcmp((char *)dat, "/getsettings") == 0) {
          client->route = 130;
        } else if (strcmp((char *)dat, "/firmware") == 0) {
          client->route = 140;
        } else {
          client->route = 0;
        }

        return 0;
      } break;
    case WEBSERVER_CLIENT_ARGS: {
        struct arguments_t *args = (struct arguments_t *)dat;
        switch (client->route) {
          case 10: {
              if (strcmp((char *)args->name, "1wire") == 0) {
                client->route = 11;
              } else if (strcmp((char *)args->name, "s0") == 0) {
                client->route = 12;
              }
            } break;
          case 100: {
              unsigned char cmd[256] = { 0 };
              char cpy[args->len + 1];
              char log_msg[256] = { 0 };
              unsigned int len = 0;

              memset(&cpy, 0, args->len + 1);
              snprintf((char *)&cpy, args->len, "%.*s", args->len, args->value);

              for (uint8_t x = 0; x < sizeof(commands) / sizeof(commands[0]); x++) {
                if (strcmp((char *)args->name, commands[x].name) == 0) {
                  len = commands[x].func(cpy, cmd, log_msg);
                  RESTmsg = RESTmsg + log_msg + "\n";
                  log_message(log_msg);
                  send_command(cmd, len);
                }
              }

              memset(&cmd, 256, 0);
              memset(&log_msg, 256, 0);

              if (heishamonSettings.optionalPCB) {
                //optional commands
                for (uint8_t x = 0; x < sizeof(optionalCommands) / sizeof(optionalCommands[0]); x++) {
                  if (strcmp((char *)args->name, optionalCommands[x].name) == 0) {
                    len = optionalCommands[x].func(cpy, log_msg);
                    RESTmsg = RESTmsg + log_msg + "\n";
                    log_message(log_msg);
                  }
                }
              }
            } break;
          case 110: {
              return cacheSettings(client, args);
            } break;
          case 150: {
              if (uploadpercentage != (unsigned int)(((float)client->readlen / (float)client->totallen) * 100)) {
                uploadpercentage = (unsigned int)(((float)client->readlen / (float)client->totallen) * 100);
                sprintf_P(log_msg, PSTR("Uploading new firmware: %d%%"), uploadpercentage);
                log_message(log_msg);
              }
              if (!Update.hasError() && strcmp((char *)args->name, "firmware") == 0) {
                if (Update.write((uint8_t *)args->value, args->len) != args->len) {
                  Update.printError(Serial1);
                }
              }
            } break;
        }
      } break;
    case WEBSERVER_CLIENT_HEADER: {
        struct arguments_t *args = (struct arguments_t *)dat;
        return 0;
      } break;
    case WEBSERVER_CLIENT_WRITE: {
        switch (client->route) {
          case 0: {
              webserver_send(client, 404, (char *)"text/plain", 0);
              return 0;
            } break;
          case 1: {
              return handleRoot(client, readpercentage, mqttReconnects, &heishamonSettings);
            } break;
          case 10:
          case 11:
          case 12: {
              return handleTableRefresh(client, actData);
            } break;
          case 20: {
              return handleJsonOutput(client, actData);
            } break;
          case 30: {
              return handleReboot(client);
            } break;
          case 40: {
              return handleDebug(client, (char *)data, 203);
            } break;
          case 50: {
              return handleWifiScan(client);
            } break;
          case 80: {
              return handleSettings(client);
            } break;
          case 90: {
              return handleFactoryReset(client);
            } break;
          case 100: {
              if (client->content == 0) {
                webserver_send(client, 200, (char *)"text/plain", 0);
                char *str = (char *)RESTmsg.c_str();
                webserver_send_content(client, (char *)str, strlen(str));
                RESTmsg.clear();
              }
              return 0;
            } break;
          case 110: {
              int ret = saveSettings(client, &heishamonSettings);
              switch (client->route) {
                case 111: {
                    return settingsNewPassword(client, &heishamonSettings);
                  } break;
                case 112: {
                    return settingsReconnectWifi(client, &heishamonSettings);
                  } break;
                case 113: {
                    webserver_send(client, 301, (char *)"text/plain", 0);
                  } break;
              }
              return 0;
            } break;
          case 111: {
              return settingsNewPassword(client, &heishamonSettings);
            } break;
          case 112: {
              return settingsReconnectWifi(client, &heishamonSettings);
            } break;
          case 120: {
              return handleSettings(client);
            } break;
          case 130: {
              return getSettings(client, &heishamonSettings);
            } break;
          case 140: {
              return showFirmware(client);
            } break;
          case 150: {
              if (uploadpercentage != (unsigned int)(((float)client->readlen / (float)client->totallen) * 100)) {
                uploadpercentage = (unsigned int)(((float)client->readlen / (float)client->totallen) * 100);
                sprintf_P(log_msg, PSTR("Uploading new firmware: %d%%"), uploadpercentage);
                log_message(log_msg);
              }
              if (Update.end(true)) {
                log_message((char *)"Update Success");
                timerqueue_insert(15, 0, -2); // Start reboot sequence
                return showFirmwareSuccess(client);
              } else {
                Update.printError(Serial1);
                return showFirmwareFail(client);
              }
            } break;
          default: {
              webserver_send(client, 301, (char *)"text/plain", 0);
            } break;
        }
        return -1;
      } break;
    case WEBSERVER_CLIENT_CREATE_HEADER: {
        struct header_t *header = (struct header_t *)dat;
        switch (client->route) {
          case 113: {
              header->ptr += sprintf((char *)header->buffer, "Location: /settings");
              return -1;
            } break;
          case 0:
          case 60:
          case 70: {
              header->ptr += sprintf((char *)header->buffer, "Location: /");
              return -1;
            } break;
          default: {
              header->ptr += sprintf((char *)header->buffer, "Access-Control-Allow-Origin: *");
            } break;
        }
        return 0;
      } break;
    default: {
        return 0;
      } break;
  }
  return 0;
}

void setupHttp() {
  webserver_start(80, &webserver_cb, 0);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(3000, 3000, 2);
}

void doubleResetDetect() {
  if (drd.detect()) {
    Serial.println("Double reset detected, clearing config."); //save to print on std serial because serial switch didn't happen yet
    LittleFS.begin();
    LittleFS.format();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    Serial.println("Config cleared. Please reset to configure this device...");
    //initiate debug led indication for factory reset
    pinMode(2, FUNCTION_0); //set it as gpio
    pinMode(2, OUTPUT);
    while (true) {
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
      delay(100);
    }

  }
}

void setupSerial() {
  //boot issue's first on normal serial
  Serial.begin(115200);
  Serial.flush();
}

void setupSerial1() {
  if (heishamonSettings.logSerial1) {
    //debug line on serial1 (D4, GPIO2)
    Serial1.begin(115200);
    Serial1.println(F("Starting debugging"));
  }
  else {
    pinMode(2, FUNCTION_0); //set it as gpio
  }
}

void switchSerial() {
  Serial.println(F("Switching serial to connect to heatpump. Look for debug on serial1 (GPIO2) and mqtt log topic."));
  //serial to cn-cnt
  Serial.flush();
  Serial.end();
  Serial.begin(9600, SERIAL_8E1);
  Serial.flush();
  //swap to gpio13 (D7) and gpio15 (D8)
  Serial.swap();
  //turn on GPIO's on tx/rx for later use
  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);

  setupGPIO(heishamonSettings.gpioSettings); //switch extra GPIOs to configured mode

  //enable gpio15 after boot using gpio5 (D1) which enables the level shifter for the tx to panasonic
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
}

void setupMqtt() {
  mqtt_client.setBufferSize(1024);
  mqtt_client.setSocketTimeout(10); mqtt_client.setKeepAlive(5); //fast timeout, any slower will block the main loop too long
  mqtt_client.setServer(heishamonSettings.mqtt_server, atoi(heishamonSettings.mqtt_port));
  mqtt_client.setCallback(mqtt_callback);
}

void setupConditionals() {
  //load optional PCB data from flash
  if (heishamonSettings.optionalPCB) {
    if (loadOptionalPCB(optionalPCBQuery, OPTIONALPCBQUERYSIZE)) {
      log_message((char*)"Succesfully loaded optional PCB data from saved flash!");
    }
    else {
      log_message((char*)"Failed to load optional PCB data from flash!");
    }
    delay(1500); //need 1.5 sec delay before sending first datagram
    send_optionalpcb_query(); //send one datagram already at start
  }

  //these two after optional pcb because it needs to send a datagram fast after boot
  if (heishamonSettings.use_1wire) initDallasSensors(log_message, heishamonSettings.updataAllDallasTime, heishamonSettings.waitDallasTime, heishamonSettings.dallasResolution);
  if (heishamonSettings.use_s0) initS0Sensors(heishamonSettings.s0Settings);
}

void timer_cb(int nr) {
  sprintf_P(log_msg, PSTR("%d seconds timer interval"), nr);
  log_message(log_msg);

  if (nr > 0) {
    timerqueue_insert(nr, 0, nr);
  } else {
    switch (nr) {
      case -1: {
          LittleFS.begin();
          LittleFS.format();
          WiFi.disconnect(true);
          timerqueue_insert(1, 0, -2);
        } break;
      case -2: {
          ESP.restart();
        } break;
    }
  }

}

void setup() {
  //first get total memory before we do anything
  getFreeMemory();

  //set boottime
  char *up = getUptime();
  free(up);

  setupSerial();
  setupSerial1();

  Serial.println();
  Serial.println(F("--- HEISHAMON ---"));
  Serial.println(F("starting..."));

  //double reset detect from start
  doubleResetDetect();

  WiFi.printDiag(Serial);
  //initiate a wifi scan at boot to prefill the wifi scan json list
  byte numSsid = WiFi.scanNetworks();
  getWifiScanResults(numSsid);

  loadSettings(&heishamonSettings);

  setupWifi(&heishamonSettings);

  setupMqtt();
  setupHttp();

  switchSerial(); //switch serial to gpio13/gpio15
  WiFi.printDiag(Serial1);

  setupConditionals(); //setup for routines based on settings

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  //timerqueue tests
  //timerqueue_insert(1, 0, 1);
  //timerqueue_insert(5, 0, 5);
  //timerqueue_insert(60, 0, 60);

  //maybe necessary but for now disable. CZ-TAW1 sends this query on boot
  //if (!heishamonSettings.listenonly) send_initial_query();
}

void send_initial_query() {
  String message = F("Requesting initial start query");
  log_message((char*)message.c_str());
  send_command(initialQuery, INITIALQUERYSIZE);
}

void send_panasonic_query() {
  String message = F("Requesting new panasonic data");
  log_message((char*)message.c_str());
  send_command(panasonicQuery, PANASONICQUERYSIZE);
}

void send_optionalpcb_query() {
  String message = F("Sending optional PCB data");
  log_message((char*)message.c_str());
  send_command(optionalPCBQuery, OPTIONALPCBQUERYSIZE);
}


void read_panasonic_data() {
  if (sending && ((unsigned long)(millis() - sendCommandReadTime) > SERIALTIMEOUT)) {
    log_message((char*)"Previous read data attempt failed due to timeout!");
    sprintf_P(log_msg, PSTR("Received %d bytes data"), data_length);
    log_message(log_msg);
    if (heishamonSettings.logHexdump) logHex(data, data_length);
    if (data_length == 0) {
      timeoutread++;
      totalreads++; //at at timeout we didn't receive anything but did expect it so need to increase this for the stats
    } else {
      tooshortread++;
    }
    data_length = 0; //clear any data in array
    sending = false; //receiving the answer from the send command timed out, so we are allowed to send a new command
  }
  if ( (heishamonSettings.listenonly || sending) && (Serial.available() > 0)) readSerial();
}

void loop() {
  webserver_loop();

  // check wifi
  check_wifi();
  // Handle OTA first.
  ArduinoOTA.handle();
  // handle Websockets
  webSocket.loop();

  mqtt_client.loop();

  read_panasonic_data();

  if ((!sending) && (cmdnrel > 0)) { //check if there is a send command in the buffer
    log_message((char *)"Sending command from buffer");
    popCommandBuffer();
  }

  if (heishamonSettings.use_1wire) dallasLoop(mqtt_client, log_message, heishamonSettings.mqtt_topic_base);

  if (heishamonSettings.use_s0) s0Loop(mqtt_client, log_message, heishamonSettings.mqtt_topic_base, heishamonSettings.s0Settings);

  if ((!sending) && (!heishamonSettings.listenonly) && (heishamonSettings.optionalPCB)) send_optionalpcb_query(); //send this as fast as possible or else we could get warnings on heatpump

  // run the data query only each WAITTIME
  if ((unsigned long)(millis() - lastRunTime) > (1000 * heishamonSettings.waitTime)) {
    lastRunTime = millis();
    //check mqtt
    if ( (WiFi.isConnected()) && (!mqtt_client.connected()) )
    {
      log_message((char *)"Lost MQTT connection!");
      mqtt_reconnect();
    }

    //log stats
    if (totalreads > 0 ) readpercentage = (((float)goodreads / (float)totalreads) * 100);
    String message = F("Heishamon stats: Uptime: ");
    char *up = getUptime();
    message += up;
    free(up);
    message += F(" ## Free memory: ");
    message += getFreeMemory();
    message += F("% ");
    message += ESP.getFreeHeap();
    message += F(" bytes ## Wifi: ");
    message += getWifiQuality();
    message += F("% (RSSI: ");
    message += WiFi.RSSI();
    message += F(") ## Mqtt reconnects: ");
    message += mqttReconnects;
    message += F(" ## Correct data: ");
    message += readpercentage;
    message += F("%");
    log_message((char*)message.c_str());

    String stats = F("{\"uptime\":");
    stats += String(millis());
    stats += F(",\"voltage\":");
    stats += ESP.getVcc() / 1024.0;
    stats += F(",\"free memory\":");
    stats += getFreeMemory();
    stats += F(",\"free heap\":");
    stats += ESP.getFreeHeap();
    stats += F(",\"wifi\":");
    stats += getWifiQuality();
    stats += F(",\"mqtt reconnects\":");
    stats += mqttReconnects;
    stats += F(",\"total reads\":");
    stats += totalreads;
    stats += F(",\"good reads\":");
    stats += goodreads;
    stats += F(",\"bad crc reads\":");
    stats += badcrcread;
    stats += F(",\"bad header reads\":");
    stats += badheaderread;
    stats += F(",\"too short reads\":");
    stats += tooshortread;
    stats += F(",\"too long reads\":");
    stats += toolongread;
    stats += F(",\"timeout reads\":");
    stats += timeoutread;
    stats += F("}");
    sprintf(mqtt_topic, "%s/stats", heishamonSettings.mqtt_topic_base);
    mqtt_client.publish(mqtt_topic, stats.c_str(), MQTT_RETAIN_VALUES);

    //get new data
    if (!heishamonSettings.listenonly) send_panasonic_query();

    //Make sure the LWT is set to Online, even if the broker have marked it dead.
    sprintf(mqtt_topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
    mqtt_client.publish(mqtt_topic, "Online");

    if (WiFi.isConnected()) {
      MDNS.announce();
    }
  }

  timerqueue_update();
}
