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

//volatile pulse detectors for s0
volatile unsigned long new_pulse_s0[2];


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
          sprintf(mqtt_topic, "%s/%s/%s", mqtt_topic_base, mqtt_topic_1wire, actDallasData[i].address.c_str()); mqtt_client.publish(mqtt_topic, String(actDallasData[i].temperature).c_str(), MQTT_RETAIN_VALUES);
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

//These are the interrupt routines. Make them as short as possible so we don't block other interrupts (for example serial data)
ICACHE_RAM_ATTR void onS0Pulse1() {
  new_pulse_s0[0] = millis();
}

ICACHE_RAM_ATTR void onS0Pulse2() {
  new_pulse_s0[1] = millis();
}

void initS0Sensors(s0Data actS0Data[]) {

  pinMode(actS0Data[0].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[0].gpiopin), onS0Pulse1, RISING);
  actS0Data[0].lastPulse = millis();


  pinMode(actS0Data[1].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[1].gpiopin), onS0Pulse2, RISING);
  actS0Data[1].lastPulse = millis();
}

void s0Loop(s0Data actS0Data[], PubSubClient &mqtt_client, void (*log_message)(char*)) {

  for (int i = 0 ; i < NUM_S0_COUNTERS ; i++) {
    //first handle new detected pulses
    if (new_pulse_s0[i] > 0) {
      actS0Data[i].pulseInterval = new_pulse_s0[i] - actS0Data[i].lastPulse;
      actS0Data[i].watt = (3600000000.0 / actS0Data[i].pulseInterval) / actS0Data[i].ppkwh;
      actS0Data[i].lastPulse = new_pulse_s0[i];
      actS0Data[i].pulses++; 
      new_pulse_s0[i] = 0;
      //Serial1.print("S0 port "); Serial1.print(i); Serial1.print(" detected pulse. Pulses since last report: "); Serial1.println(actS0Data[i].pulses);
    }

    //then report after each reportInterval
    if (millis() > actS0Data[i].nextReport) {
      actS0Data[i].nextReport = millis() + 1000 * actS0Data[i].reportInterval; // reportInterval per s0 port depends on imp/kwh and minimal watt value)

      //check if new pulse is too late for the current measured watt so we need to artificially lower the reported watt
      unsigned long lastePulseInterval = millis() - actS0Data[i].lastPulse;
      unsigned long calcMaxWatt = (3600000000.0 / lastePulseInterval) / actS0Data[i].ppkwh;
      if (actS0Data[i].watt > calcMaxWatt) { // If last known watt is bigger then calculated max watt then last known watt is invalid, report halve of calculated max
        //Serial1.println("===Previous watt is too high. Dividing by two now===");
        actS0Data[i].watt = calcMaxWatt / 2;
      }

      //check if watt is lower than which was possible during this period, so we we assume connected device is in standby
      if (actS0Data[i].watt < ((3600 * 1000 / actS0Data[i].ppkwh) / actS0Data[i].reportInterval) ) {
        //Serial1.println("===In standby mode===");
        if (!actS0Data[i].inStandby) {
          //Serial1.println("===For the first time standby mode===");
          actS0Data[i].inStandby = true;
          actS0Data[i].startStandby = actS0Data[i].lastPulse;
        }
        if (actS0Data[i].startStandby < actS0Data[i].lastPulse) { //during standby a pulse was received so we know now the exact standby usage
          //Serial1.println("===Standby watt is known===");
          unsigned long standbyInterval = actS0Data[i].lastPulse - actS0Data[i].startStandby;
          actS0Data[i].watt = (3600000000.0 / standbyInterval) / actS0Data[i].ppkwh;
          actS0Data[i].startStandby = actS0Data[i].lastPulse;
        }
        else { // we must guess the standby watt
          //Serial1.println("===Standby watt is guessed===");
          unsigned long standbyInterval = millis() - actS0Data[i].startStandby;
          unsigned long standbyWatt = (3600000000.0 / standbyInterval) / actS0Data[i].ppkwh;
          if (standbyWatt < actS0Data[i].watt) { //update the last known watt with the now lower calculated standby watt
            //Serial1.println("===Standby watt guessed is updated===");
            actS0Data[i].watt = standbyWatt;
          }
        }
      }
      else {
        actS0Data[i].inStandby = false;
        actS0Data[i].startStandby = 0;
      }

      //calculate the watthour since last message
      float Watthour = (actS0Data[i].pulses * ( 1000 / actS0Data[i].ppkwh));
      actS0Data[i].pulses = 0; //per message we report new wattHour, so pulses should be zero at start new message

      //report using mqtt
      char log_msg[256];
      char mqtt_topic[256];
      sprintf(log_msg, "Measured Watthour on S0 port %d: %.2f", (i + 1),  Watthour ); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s/Watthour/%d", mqtt_topic_base, mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, String(Watthour).c_str(), MQTT_RETAIN_VALUES);
      sprintf(log_msg, "Calculated Watt on S0 port %d: %lu", (i + 1), actS0Data[i].watt); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s/Watt/%d", mqtt_topic_base, mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, String(actS0Data[i].watt).c_str(), MQTT_RETAIN_VALUES);
    }
  }
}

String s0TableOutput(s0Data actS0Data[]) {
  String output = "";
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + "<tr>";
    output = output + "<td>" + (i + 1) + "</td>";
    output = output + "<td>" + actS0Data[i].watt + "</td>";
    output = output + "</tr>";
  }
  return output;
}

String s0JsonOutput(s0Data actS0Data[]) {
  String output = "[";
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + "{";
    output = output + "\"S0 port\": \"" + (i + 1) + "\",";
    output = output + "\"Watt\": \"" + actS0Data[i].watt + "\"";
    output = output + "}";
    if (i < NUM_S0_COUNTERS - 1) output = output + ",";
  }
  output = output + "]";
  return output;
}
