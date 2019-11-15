#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>



// Replace with your network credentials
const char* wifi_ssid     = "ssid";
const char* wifi_password = "password";
const char* wifi_hostname = "panasonic_heat_pump";
const char* ota_password  = "panasonic";

const char* mqtt_server   = "IP";
const int   mqtt_port     = 1883;
const char* mqtt_username = "user";
const char* mqtt_password = "password";

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

// mqtt
WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);

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
  for (int i=0; i < length; i++) {
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

void setup() {  
  Serial.begin(9600,SERIAL_8E1);
   Serial.flush();
  //Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.hostname(wifi_hostname);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

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

  mqtt_client.setServer(mqtt_server, mqtt_port);
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
  if (outputMqttLog)
  {
    mqtt_client.publish(mqtt_logtopic, string); 
  } 
}

void readSerial()
{
    if(Serial.available() > 0){
      log_message("serial available");
    Serial.readBytes(data, 203);
    while(Serial.available()){
      delay(2);
      Serial.read();
    }
    
  }
}
