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


unsigned long nextalldatatime_dallas = 0;

unsigned long dallasTimer = 0;
unsigned int updateAllDallasTime = 30000; // will be set using heishmonSettings
unsigned int dallasTimerWait = 30000; // will be set using heishmonSettings

void initDallasSensors(void (*log_message)(char*), unsigned int updateAllDallasTimeSettings, unsigned int dallasTimerWaitSettings) {
  char log_msg[256];
  updateAllDallasTime = updateAllDallasTimeSettings;
  dallasTimerWait = dallasTimerWaitSettings;
  DS18B20.begin();
  dallasDevicecount  = DS18B20.getDeviceCount();
  sprintf(log_msg, "Number of 1wire sensors on bus: %d", dallasDevicecount); log_message(log_msg);
  if ( dallasDevicecount > MAX_DALLAS_SENSORS) {
    dallasDevicecount = MAX_DALLAS_SENSORS;
    sprintf(log_msg, "Reached max 1wire sensor count. Only %d sensors will provide data.", dallasDevicecount); log_message(log_msg);
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
    sprintf(log_msg, "Found 1wire sensor: %s", actDallasData[i].address ); log_message(log_msg);
  }
  if (DALLASASYNC) DS18B20.setWaitForConversion(false); //async 1wire during next loops
}

void readNewDallasTemp(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  char log_msg[256];
  char mqtt_topic[256];
  char valueStr[20];
  bool updatenow = false;

  if (millis() > nextalldatatime_dallas) {
    updatenow = true;
    nextalldatatime_dallas = millis() + (1000 * updateAllDallasTime);
  }
  if (!(DALLASASYNC)) DS18B20.requestTemperatures();
  for (int i = 0; i < dallasDevicecount; i++) {
    float temp = DS18B20.getTempC(actDallasData[i].sensor);
    if (temp < -120.0) {
      sprintf(log_msg, "Error 1wire sensor offline: %s", actDallasData[i].address); log_message(log_msg);
    } else {
      float allowedtempdiff = (((millis() - actDallasData[i].lastgoodtime)) / 1000.0) * MAXTEMPDIFFPERSEC;
      if ((actDallasData[i].temperature != -127.0) and ((temp > (actDallasData[i].temperature + allowedtempdiff)) or (temp < (actDallasData[i].temperature - allowedtempdiff)))) {
        sprintf(log_msg, "Filtering 1wire sensor temperature (%s). Delta to high. Current: %.2f Last: %.2f", actDallasData[i].address, temp, actDallasData[i].temperature); log_message(log_msg);
      } else {
        actDallasData[i].lastgoodtime = millis();
        if ((updatenow) || (actDallasData[i].temperature != temp )) {  //only update mqtt topic if temp changed or after each update timer
          actDallasData[i].temperature = temp;
          sprintf(log_msg, "Received 1wire sensor temperature (%s): %.2f", actDallasData[i].address, actDallasData[i].temperature); log_message(log_msg);
          sprintf(valueStr, "%.2f", actDallasData[i].temperature);
          sprintf(mqtt_topic, "%s/%s/%s", mqtt_topic_base, mqtt_topic_1wire, actDallasData[i].address); mqtt_client.publish(mqtt_topic, valueStr, MQTT_RETAIN_VALUES);
        }
      }
    }
  }
}

void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  if ((DALLASASYNC) && (millis() > (dallasTimer - 1000))) {
    DS18B20.requestTemperatures(); // get temperatures for next run 1 second before getting the temperatures (async)
  }
  if (millis() > dallasTimer) {
    log_message((char*)"Requesting new 1wire temperatures");
    dallasTimer = millis() + (1000 * dallasTimerWait);
    readNewDallasTemp(mqtt_client, log_message, mqtt_topic_base);
  }
}

String dallasJsonOutput() {
  String output = "[";
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + "{";
    output = output + "\"Sensor\": \"" + actDallasData[i].address + "\",";
    output = output + "\"Temperature\": \"" + actDallasData[i].temperature + "\"";
    output = output + "}";
    if (i < dallasDevicecount - 1) output = output + ",";
  }
  output = output + "]";
  return output;
}

String dallasTableOutput() {
  String output = "";
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + "<tr>";
    output = output + "<td>" + actDallasData[i].address + "</td>";
    output = output + "<td>" + actDallasData[i].temperature + "</td>";
    output = output + "</tr>";
  }
  return output;
}
