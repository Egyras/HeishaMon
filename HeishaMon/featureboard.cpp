#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "commands.h"

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define FETCHTEMPSTIME 30000 // how often Dallas temps are read
#define MAXTEMPDIFFPERSEC 0.5 // what is the allowed temp difference per second which is allowed (to filter bad values)

#define ONE_WIRE_BUS 4  // DS18B20 pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

int dallasDevicecount = 0;
float dallasTemp[15];
unsigned long dallasTempTime[15];
DeviceAddress dallasSensor[15];
String dallasSensorAddress[15];

unsigned long dallasTimer = 0;

void initDallasSensors(void (*log_message)(char*)) {
  char log_msg[256];
  DS18B20.begin();
  dallasDevicecount  = DS18B20.getDeviceCount();
  sprintf(log_msg, "Number of 1wire sensors on bus: %d", dallasDevicecount); log_message(log_msg);

  for (int j = 0 ; j < dallasDevicecount; j++) {
    DS18B20.getAddress(dallasSensor[j], j);
  }

  DS18B20.requestTemperatures();
  for (int i = 0 ; i < dallasDevicecount; i++) {
    dallasTemp[i] = DS18B20.getTempC(dallasSensor[i]);
    dallasTempTime[i] = millis();
    dallasSensorAddress[i] = "";
    for (uint8_t x = 0; x < 8; x++)  {
      // zero pad the address if necessary
      if (dallasSensor[i][x] < 16) dallasSensorAddress[i] = dallasSensorAddress[i] + "0";
      dallasSensorAddress[i] = dallasSensorAddress[i]  + String(dallasSensor[i][x], HEX);
    }
    sprintf(log_msg, "Found 1wire sensor: %s with current temp: %s", dallasSensorAddress[i].c_str(), String(dallasTemp[i]).c_str()); log_message(log_msg);
  }
}

void readNewDallasTemp(PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];
  DS18B20.requestTemperatures();
  for (int i = 0; i < dallasDevicecount; i++) {
    float temp = DS18B20.getTempC(dallasSensor[i]);
    if (temp < -120) {
      sprintf(log_msg, "Error 1wire sensor offline: %s", dallasSensorAddress[i].c_str()); log_message(log_msg);
    } else {
      int allowedtempdiff = ((millis() - dallasTempTime[i]) * MAXTEMPDIFFPERSEC ) / 1000;
      if ((temp > (dallasTemp[i] + allowedtempdiff)) or (temp < (dallasTemp[i] - allowedtempdiff))) {
        sprintf(log_msg, "Filtering 1wire sensor temperature (%s). Delta to high. Last: %s Current: %s", dallasSensorAddress[i].c_str(), String(temp).c_str(), String(dallasTemp[i]).c_str()); log_message(log_msg);
      } else {
        dallasTemp[i] = temp;
        dallasTempTime[i] = millis();
        sprintf(log_msg, "Received 1wire sensor temperature (%s): %s", dallasSensorAddress[i].c_str(), String(dallasTemp[i]).c_str()); log_message(log_msg);
        sprintf(mqtt_topic, "%s/%s", mqtt_topic_1wire, dallasSensorAddress[i].c_str()); mqtt_client.publish(mqtt_topic, String(dallasTemp[i]).c_str(), MQTT_RETAIN_VALUES);
      }
    }
  }
}

void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*)) {
  if (millis() > dallasTimer) {
    log_message((char*)"Requesting new 1wire temperatures");
    dallasTimer = millis() + FETCHTEMPSTIME;
    readNewDallasTemp(mqtt_client, log_message);
  }
}
