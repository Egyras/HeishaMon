#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "commands.h"
#include "featureboard.h"

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define FETCHTEMPSTIME 5000 // how often Dallas temps are read in msec
#define MAXTEMPDIFFPERSEC 0.5 // what is the allowed temp difference per second which is allowed (to filter bad values)

#define FETCHS0TIME 5000 // how often s0 Watts are reported

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

int dallasDevicecount = 0;

unsigned long nextalldatatime_dallas = 0;

unsigned long dallasTimer = 0;

//global array for s0 data, must be volatile due to interupt routines
volatile s0Data actS0Data[NUM_S0_COUNTERS];

unsigned long s0Timer = 0;

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
    if (i < dallasDevicecount - 1) output = output + ",";
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

ICACHE_RAM_ATTR void onS0Pulse1() {
  unsigned long newBlink = millis();
  if ((newBlink - actS0Data[0].lastBlink) < 10L) { // Sometimes we get multiple interrupts on RISING
    return;
  }
  actS0Data[0].pulseInterval = newBlink - actS0Data[0].lastBlink;
  actS0Data[0].watt = (3600000.0 / actS0Data[0].pulseInterval) / actS0Data[0].ppwh;
  actS0Data[0].lastBlink = newBlink;
}

ICACHE_RAM_ATTR void onS0Pulse2() {
  unsigned long newBlink = millis();
  if ((newBlink - actS0Data[1].lastBlink) < 10L) { // Sometimes we get multiple interrupts on RISING
    return;
  }
  actS0Data[1].pulseInterval = newBlink - actS0Data[1].lastBlink;
  actS0Data[1].watt = (3600000.0 / actS0Data[1].pulseInterval) / actS0Data[1].ppwh;
  actS0Data[1].lastBlink = newBlink;
}

void initS0Sensors() {
  actS0Data[0].gpiopin = S0_PIN_1;
  pinMode(actS0Data[0].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[0].gpiopin), onS0Pulse1, RISING);
  actS0Data[0].lastBlink = millis();

  actS0Data[1].gpiopin = S0_PIN_2;
  pinMode(actS0Data[1].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[1].gpiopin), onS0Pulse2, RISING);
  actS0Data[1].lastBlink = millis();
}

void s0Loop(PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];
  if (millis() > s0Timer) {
    s0Timer = millis() + FETCHS0TIME;
    for (int i = 0 ; i < NUM_S0_COUNTERS ; i++) {
      unsigned long interval = millis() - actS0Data[i].lastBlink;
      unsigned long calcMaxWatt = (3600000.0 / interval) / actS0Data[i].ppwh;

      // Check if last known measured watt is lower than calculated max watt
      if (actS0Data[i].watt < calcMaxWatt) {
        sprintf(log_msg, "Measured Watt on S0 port %d: %lu", (i + 1), actS0Data[i].watt); log_message(log_msg);
        sprintf(mqtt_topic, "%s/%d", mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, String(actS0Data[i].watt).c_str(), MQTT_RETAIN_VALUES);
      } else if ((actS0Data[i].watt / 2) < calcMaxWatt) {
        sprintf(log_msg, "Calculated Watt on S0 port %d: %lu", (i + 1), calcMaxWatt); log_message(log_msg);
        sprintf(mqtt_topic, "%s/%d", mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, String(calcMaxWatt).c_str(), MQTT_RETAIN_VALUES);
      } else {
        sprintf(log_msg, "Measured Watt on S0 port %d: %lu", (i + 1), 0); log_message(log_msg);
        sprintf(mqtt_topic, "%s/%d", mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, "0", MQTT_RETAIN_VALUES);
      }
    }
  }
}

String s0TableOutput() {
  String output = "";
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + "<tr>";
    output = output + "<td>" + (i + 1) + "</td>";
    unsigned long interval = millis() - actS0Data[i].lastBlink;
    unsigned long calcMaxWatt = (3600000.0 / interval) / actS0Data[i].ppwh;
    if (actS0Data[i].watt < calcMaxWatt) {
      output = output + "<td>" + actS0Data[i].watt + "</td>";
    } else if ((actS0Data[i].watt / 2) < calcMaxWatt) {
      output = output + "<td>" + calcMaxWatt + "</td>";
    } else {
      output = output + "<td>0</td>";
    }
    output = output + "</tr>";
  }
  return output;
}

String s0JsonOutput() {
  String output = "[";
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + "{";
    output = output + "\"S0 port\": \"" + (i + 1) + "\",";
    unsigned long interval = millis() - actS0Data[i].lastBlink;
    unsigned long calcMaxWatt = (3600000.0 / interval) / actS0Data[i].ppwh;
    if (actS0Data[i].watt < calcMaxWatt) {
      output = output + "\"Watt\": \"" + actS0Data[i].watt + "\"";
    } else if ((actS0Data[i].watt / 2) < calcMaxWatt) {
      output = output + "\"Watt\": \"" + calcMaxWatt + "\"";
    } else {
      output = output + "\"Watt\": \"0\"";
    }
    output = output + "}";
    if (i < NUM_S0_COUNTERS - 1) output = output + ",";
  }
  output = output + "]";
  return output;
}
