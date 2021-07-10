#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include "commands.h"
#include "dallas.h"

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define MAXTEMPDIFFPERSEC 0.5 // what is the allowed temp difference per second which is allowed (to filter bad values)

#define DALLASASYNC 0 //async dallas yes or no (default no, because async seems to break 1wire sometimes with current code)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

//global array for 1wire data
dallasDataStruct* actDallasData = 0;
int dallasDevicecount = 0;


unsigned long lastalldatatime_dallas = 0;

unsigned long dallasTimer = 0;
unsigned int updateAllDallasTime = 30000; // will be set using heishmonSettings
unsigned int dallasTimerWait = 30000; // will be set using heishmonSettings

void initDallasSensors(void (*log_message)(char*), unsigned int updateAllDallasTimeSettings, unsigned int dallasTimerWaitSettings) {
  char log_msg[256];
  updateAllDallasTime = updateAllDallasTimeSettings;
  dallasTimerWait = dallasTimerWaitSettings;
  DS18B20.begin();
  dallasDevicecount  = DS18B20.getDeviceCount();
  sprintf_P(log_msg, PSTR("Number of 1wire sensors on bus: %d"), dallasDevicecount); log_message(log_msg);
  if ( dallasDevicecount > MAX_DALLAS_SENSORS) {
    dallasDevicecount = MAX_DALLAS_SENSORS;
    sprintf_P(log_msg, PSTR("Reached max 1wire sensor count. Only %d sensors will provide data."), dallasDevicecount);
    log_message(log_msg);
  }

  //init array
  actDallasData = new dallasDataStruct [dallasDevicecount];
  for (int j = 0 ; j < dallasDevicecount; j++) {
    DS18B20.getAddress(actDallasData[j].sensor, j);
  }

  DS18B20.requestTemperatures();
  for (int i = 0 ; i < dallasDevicecount; i++) {
    actDallasData[i].address[16] = '\0';
    for (int x = 0; x < 8; x++)  {
      // zero pad the address if necessary
      sprintf(&actDallasData[i].address[x * 2], "%02x", actDallasData[i].sensor[x]);
    }
    sprintf_P(log_msg, PSTR("Found 1wire sensor: %s"), actDallasData[i].address ); log_message(log_msg);
  }
  if (DALLASASYNC) DS18B20.setWaitForConversion(false); //async 1wire during next loops
}

void readNewDallasTemp(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  char log_msg[256];
  char mqtt_topic[256];
  char valueStr[20];
  bool updatenow = false;

  if ((unsigned long)(millis() > lastalldatatime_dallas) >  (1000 * updateAllDallasTime)) {
    updatenow = true;
    lastalldatatime_dallas = millis();
  }
  if (!(DALLASASYNC)) DS18B20.requestTemperatures();
  for (int i = 0; i < dallasDevicecount; i++) {
    float temp = DS18B20.getTempC(actDallasData[i].sensor);
    if (temp < -120.0) {
      sprintf(log_msg, "Error 1wire sensor offline: %s", actDallasData[i].address); log_message(log_msg);
    } else {
      float allowedtempdiff = (((millis() - actDallasData[i].lastgoodtime)) / 1000.0) * MAXTEMPDIFFPERSEC;
      if ((actDallasData[i].temperature != -127.0) and ((temp > (actDallasData[i].temperature + allowedtempdiff)) or (temp < (actDallasData[i].temperature - allowedtempdiff)))) {
        sprintf_P(log_msg, PSTR("Filtering 1wire sensor temperature (%s). Delta to high. Current: %.2f Last: %.2f"), actDallasData[i].address, temp, actDallasData[i].temperature);
        log_message(log_msg);
      } else {
        actDallasData[i].lastgoodtime = millis();
        if ((updatenow) || (actDallasData[i].temperature != temp )) {  //only update mqtt topic if temp changed or after each update timer
          actDallasData[i].temperature = temp;
          sprintf(log_msg, PSTR("Received 1wire sensor temperature (%s): %.2f"), actDallasData[i].address, actDallasData[i].temperature);
          log_message(log_msg);
          sprintf(valueStr, "%.2f", actDallasData[i].temperature);
          sprintf(mqtt_topic, "%s/%s/%s", mqtt_topic_base, mqtt_topic_1wire, actDallasData[i].address); mqtt_client.publish(mqtt_topic, valueStr, MQTT_RETAIN_VALUES);
        }
      }
    }
  }
}

void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  if ((DALLASASYNC) && ((unsigned long)(millis() - dallasTimer) > ((1000 * dallasTimerWait)-1000)) ) {
    DS18B20.requestTemperatures(); // get temperatures for next run 1 second before getting the temperatures (async)
  }
  if ((unsigned long)(millis() - dallasTimer) > (1000 * dallasTimerWait)) {
    log_message((char*)"Requesting new 1wire temperatures");
    dallasTimer = millis();
    readNewDallasTemp(mqtt_client, log_message, mqtt_topic_base);
  }
}

String dallasJsonOutput() {
  String output = F("[");
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + F("{");
    output = output + F("\"Sensor\": \"") + actDallasData[i].address + F("\",");
    output = output + F("\"Temperature\": \"") + actDallasData[i].temperature + F("\"");
    output = output + F("}");
    if (i < dallasDevicecount - 1) output = output + F(",");
  }
  output = output + "]";
  return output;
}

String dallasTableOutput() {
  String output = "";
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + F("<tr>");
    output = output + F("<td>") + actDallasData[i].address + F("</td>");
    output = output + F("<td>") + actDallasData[i].temperature + F("</td>");
    output = output + F("</tr>");
  }
  return output;
}
