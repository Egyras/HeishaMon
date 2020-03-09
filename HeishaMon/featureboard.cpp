#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "commands.h"
#include "featureboard.h"

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define FETCHTEMPSTIME 5000 // how often Dallas temps are read in msec
#define MAXTEMPDIFFPERSEC 0.5 // what is the allowed temp difference per second which is allowed (to filter bad values)



OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

int dallasDevicecount = 0;

unsigned long nextalldatatime_dallas = 0;

unsigned long dallasTimer = 0;

void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*)) {
  char log_msg[256];
  DS18B20.begin();
  dallasDevicecount  = DS18B20.getDeviceCount();
  sprintf(log_msg, "Number of 1wire sensors on bus: %d", dallasDevicecount); log_message(log_msg);

  for (int j = 0 ; j < dallasDevicecount; j++) {
    DS18B20.getAddress(actDallasData[j].sensor, j);
  }

  DS18B20.requestTemperatures();
  for (int i = 0 ; i < dallasDevicecount; i++) {
    for (uint8_t x = 0; x < 8; x++)  {
      // zero pad the address if necessary
      if (actDallasData[i].sensor[x] < 16) actDallasData[i].address = actDallasData[i].address + "0";
      actDallasData[i].address = actDallasData[i].address  + String(actDallasData[i].sensor[x], HEX);
    }
    sprintf(log_msg, "Found 1wire sensor: %s", actDallasData[i].address.c_str() ); log_message(log_msg);
  }
}

void readNewDallasTemp(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];
  bool updatenow = false;

  if (millis() > nextalldatatime_dallas) {
    updatenow = true;
    nextalldatatime_dallas = millis() + UPDATEALLTIME_DALLAS;
  }
  
  DS18B20.requestTemperatures();
  for (int i = 0; i < dallasDevicecount; i++) {
    float temp = DS18B20.getTempC(actDallasData[i].sensor);
    if (temp < -120) {
      sprintf(log_msg, "Error 1wire sensor offline: %s", actDallasData[i].address.c_str()); log_message(log_msg);
    } else {
      int allowedtempdiff = (((millis() - actDallasData[i].lastgoodtime)) / 1000) * MAXTEMPDIFFPERSEC;
      if ((actDallasData[i].temperature != -127) and ((temp > (actDallasData[i].temperature + allowedtempdiff)) or (temp < (actDallasData[i].temperature - allowedtempdiff)))) {
        sprintf(log_msg, "Filtering 1wire sensor temperature (%s). Delta to high. Current: %s Last: %s", actDallasData[i].address.c_str(), String(temp).c_str(), String(actDallasData[i].temperature).c_str()); log_message(log_msg);
      } else {
        actDallasData[i].lastgoodtime = millis();
        if ((updatenow) || (actDallasData[i].temperature != temp )) {  //only update mqtt topic if temp changed or after each update timer
          actDallasData[i].temperature = temp;
          sprintf(log_msg, "Received 1wire sensor temperature (%s): %s", actDallasData[i].address.c_str(), String(actDallasData[i].temperature).c_str()); log_message(log_msg);
          sprintf(mqtt_topic, "%s/%s", mqtt_topic_1wire, actDallasData[i].address.c_str()); mqtt_client.publish(mqtt_topic, String(actDallasData[i].temperature).c_str(), MQTT_RETAIN_VALUES);
        }
      }
    }
  }
}

void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*)) {
  if (millis() > dallasTimer) {
    log_message((char*)"Requesting new 1wire temperatures");
    dallasTimer = millis() + FETCHTEMPSTIME;
    readNewDallasTemp(actDallasData, mqtt_client, log_message);
  }
}

String dallasJsonOutput(dallasData actDallasData[]) {
  String output = "[";
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + "{";
    output = output + "\"Sensor\": \"" + actDallasData[i].address + "\",";
    output = output + "\"Temperature\": \"" + actDallasData[i].temperature + "\"";
    output = output + "}";
    if (i < dallasDevicecount-1) output = output + ",";
  }
  output = output + "]";
  return output;
}

String dallasTableOutput(dallasData actDallasData[]) {
  String output = "";
  for (int i = 0; i < dallasDevicecount; i++) {
    output = output + "<tr>";
    output = output + "<td>" + actDallasData[i].address + "</td>";
    output = output + "<td>" + actDallasData[i].temperature + "</td>";
    output = output + "</tr>";
  }
  return output;
}

void onS0Pulse1(){

}

void onS0Pulse2() {

}

void initS0Sensors(s0Data actS0Data[], void (*log_message)(char*)) {
  actS0Data[0].gpiopin = 12;
  pinMode(actS0Data[0].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[0].gpiopin), onS0Pulse1, RISING);
  
  actS0Data[1].gpiopin = 14;
  pinMode(actS0Data[1].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[1].gpiopin), onS0Pulse2, RISING);
    
}
