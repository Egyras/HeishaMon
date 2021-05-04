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


//to read bus voltage in stats
ADC_MODE(ADC_VCC);

// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 0.1

// address to the block in the RTC user memory
// change it if it collides with another usage
// of the address block
#define DRD_ADDRESS 0x00


#define SERIALTIMEOUT 2000 // wait until all 203 bytes are read, must not be too long to avoid blocking the code

ESP8266WebServer httpServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;

settingsStruct heishamonSettings;

bool sending = false; // mutex for sending data
bool mqttcallbackinprogress = false; // mutex for processing mqtt callback
unsigned long nextMqttReconnectAttempt = 0;
#define MQTTRECONNECTTIMER 30000
unsigned long nexttime = 0;

unsigned long allowreadtime = 0; //set to millis value during send, allow to wait millis for answer
unsigned long goodreads = 0;
unsigned long totalreads = 0;
unsigned long badcrcread = 0;
unsigned long badheaderread = 0;
unsigned long tooshortread = 0;
unsigned long toolongread = 0;
unsigned long timeoutread = 0;
float readpercentage = 0;

// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 255
char data[MAXDATASIZE];
byte  data_length = 0;

// store actual data in an String array
String actData[NUMBER_OF_TOPICS];
String actOptData[NUMBER_OF_OPT_TOPICS];

// log message to sprintf to
char log_msg[256];

// mqtt topic to sprintf and then publish to
char mqtt_topic[256];

int mqttReconnects = 0;

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

void mqtt_reconnect()
{
  unsigned long now = millis();
  if (now > nextMqttReconnectAttempt) { //only try reconnect each MQTTRECONNECTTIMER seconds or on boot when nextMqttReconnectAttempt is still 0
    nextMqttReconnectAttempt = now + MQTTRECONNECTTIMER;
    if ((WiFi.status() != WL_CONNECTED) || (! WiFi.localIP()) ) {
      log_message((char *)"Lost WiFi connection!");
      if (!heishamonSettings.optionalPCB) { //do not reboot if optional pcb emulation is active because it is more important to keep transmitting data packages to heatpump
        log_message((char *)"Rebooting...");
        delay(1000);
        ESP.restart();
      }
    }
    log_message((char*)"Reconnecting to mqtt server ...");
    char topic[256];
    sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
    if (mqtt_client.connect(heishamonSettings.wifi_hostname, heishamonSettings.mqtt_username, heishamonSettings.mqtt_password, topic, 1, true, "Offline"))
    {
      mqttReconnects++;
      MDNS.begin(heishamonSettings.wifi_hostname); //assume reconnect on wifi so maybe need mdns restart
      sprintf(topic, "%s/%s/#", heishamonSettings.mqtt_topic_base, mqtt_topic_commands);
      mqtt_client.subscribe(topic);
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_send_raw_value_topic);
      mqtt_client.subscribe(topic);
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
      mqtt_client.publish(topic, "Online");
      sprintf(topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_iptopic);
      mqtt_client.publish(topic, WiFi.localIP().toString().c_str(), true);
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
#define LOGHEXBYTESPERLINE 32  // please be aware of max mqtt message size - 32 bytes per line does not work
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
  if((!sending) && cmdnrel > 0) {
    send_command(cmdbuffer[cmdstart].data, cmdbuffer[cmdstart].length);
    cmdstart = (cmdstart + 1) % (MAXCOMMANDSINBUFFER);
    cmdnrel--;
  }
}

void pushCommandBuffer(byte* command, int length) {
  if(cmdnrel+1 > MAXCOMMANDSINBUFFER) {
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
  allowreadtime = millis() + SERIALTIMEOUT; //set allowreadtime when to timeout the answer of this command
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

void setupHttp() {
  httpUpdater.setup(&httpServer, heishamonSettings.update_path, heishamonSettings.update_username, heishamonSettings.ota_password);
  httpServer.on("/", [] {
    handleRoot(&httpServer, readpercentage, mqttReconnects, &heishamonSettings);
  });
  httpServer.on("/command", [] {
    handleREST(&httpServer, heishamonSettings.optionalPCB);
  });
  httpServer.on("/tablerefresh", [] {
    handleTableRefresh(&httpServer, actData);
  });
  httpServer.on("/json", [] {
    handleJsonOutput(&httpServer, actData);
  });
  httpServer.on("/factoryreset", [] {
    handleFactoryReset(&httpServer);
  });
  httpServer.on("/reboot", [] {
    handleReboot(&httpServer);
  });
  httpServer.on("/debug", [] {
    handleDebug(&httpServer, data, 203);
  });
  httpServer.on("/settings", [] {
    handleSettings(&httpServer, &heishamonSettings);
  });
  httpServer.on("/smartcontrol", [] {
    handleSmartcontrol(&httpServer, &heishamonSettings, actData);
  });
  httpServer.on("/togglelog", [] {
    log_message((char*)"Toggled mqtt log flag");
    heishamonSettings.logMqtt ^= true;
    httpServer.sendHeader("Location", String("/"), true);
    httpServer.send ( 302, "text/plain", "");
    httpServer.client().stop();
  });
  httpServer.on("/togglehexdump", [] {
    log_message((char*)"Toggled hexdump log flag");
    heishamonSettings.logHexdump ^= true;
    httpServer.sendHeader("Location", String("/"), true);
    httpServer.send ( 302, "text/plain", "");
    httpServer.client().stop();
  });
  httpServer.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(3000, 3000, 2);
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

  //enable gpio15 after boot using gpio5 (D1)
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
}

void setupMqtt() {
  mqtt_client.setBufferSize(1024);
  mqtt_client.setSocketTimeout(10); mqtt_client.setKeepAlive(5); //fast timeout, any slower will block the main loop too long
  mqtt_client.setServer(heishamonSettings.mqtt_server, atoi(heishamonSettings.mqtt_port));
  mqtt_client.setCallback(mqtt_callback);
  mqtt_reconnect();
}

void setup() {
  setupSerial();
  setupWifi(drd, &heishamonSettings);
  MDNS.begin(heishamonSettings.wifi_hostname);
  MDNS.addService("http", "tcp", 80);
  setupSerial1();
  setupOTA();
  setupMqtt();
  setupHttp();

  switchSerial(); //switch serial to gpio13/gpio15

  //load optional PCB data from flash
  if (heishamonSettings.optionalPCB) {
    if (loadOptionalPCB(optionalPCBQuery, OPTIONALPCBQUERYSIZE)) {
      log_message((char*)"Succesfully loaded optional PCB data from saved flash!");
    }
    else {
      log_message((char*)"Failed to load optional PCB data from flash!");
    }
    delay(1500); //need 1.5 sec delay before sending first datagram
    send_optionalpcb_query(); //send one datagram already at boot
  }

  //these two after optional pcb because it needs to send a datagram fast after boot
  if (heishamonSettings.use_1wire) initDallasSensors(log_message, heishamonSettings.updataAllDallasTime, heishamonSettings.waitDallasTime);
  if (heishamonSettings.use_s0) initS0Sensors(heishamonSettings.s0Settings, mqtt_client, heishamonSettings.mqtt_topic_base);

  // wait waittime for the first start in main loop
  nexttime = millis() + (1000 * heishamonSettings.waitTime);
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
  if (sending && (millis() > allowreadtime)) {
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
  // Handle OTA first.
  ArduinoOTA.handle();
  // then handle HTTP
  httpServer.handleClient();
  // handle Websockets
  webSocket.loop();
  // Allow MDNS processing
  MDNS.update();

  mqtt_client.loop();

  read_panasonic_data();

  if ((!sending) && (cmdnrel > 0)) { //check if there is a send command in the buffer
    log_message((char *)"Sending command from buffer");
    popCommandBuffer();
  }

  if (heishamonSettings.use_1wire) dallasLoop(mqtt_client, log_message, heishamonSettings.mqtt_topic_base);

  if (heishamonSettings.use_s0) s0Loop(mqtt_client, log_message, heishamonSettings.mqtt_topic_base, heishamonSettings.s0Settings);

  if (heishamonSettings.SmartControlSettings.enableHeatCurve) smartControlLoop(log_message, heishamonSettings.SmartControlSettings, actData, goodreads);

  if ((!sending) && (!heishamonSettings.listenonly) && (heishamonSettings.optionalPCB)) send_optionalpcb_query(); //send this as fast as possible or else we could get warnings on heatpump

  // run the data query only each WAITTIME
  if (millis() > nexttime) {
    nexttime = millis() + (1000 * heishamonSettings.waitTime);
    //check mqtt
    if (!mqtt_client.connected())
    {
      log_message((char *)"Lost MQTT connection!");
      mqtt_reconnect();
    }

    //log stats
    if (totalreads > 0 ) readpercentage = (((float)goodreads / (float)totalreads) * 100);
    String message = F("Heishamon stats: Uptime: ");
    message += getUptime();
    message += F(" ## Free memory: ");
    message += getFreeMemory();
    message += F("% ");
    message += ESP.getFreeHeap();
    message += F(" bytes ## Wifi: ");
    message += getWifiQuality();
    message += F("% ## Mqtt reconnects: ");
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

    MDNS.announce();
    //Make sure the LWT is set to Online, even if the broker have marked it dead.
    sprintf(mqtt_topic, "%s/%s", heishamonSettings.mqtt_topic_base, mqtt_willtopic);
    mqtt_client.publish(mqtt_topic, "Online");
  }
}
