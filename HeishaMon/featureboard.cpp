#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "commands.h"
#include "featureboard.h"

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define MAXTEMPDIFFPERSEC 0.5 // what is the allowed temp difference per second which is allowed (to filter bad values)

#define MINREPORTEDS0TIME 5000 // how often s0 Watts are reported (not faster than this)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

int dallasDevicecount = 0;

unsigned long nextalldatatime_dallas = 0;

unsigned long dallasTimer = 0;
unsigned int updateAllDallasTime = 30000; // will be set using heishmonSettings
unsigned int dallasTimerWait = 30000; // will be set using heishmonSettings

//volatile pulse detectors for s0
volatile unsigned long new_pulse_s0[2];


unsigned long s0Timer = 0;

void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*), unsigned int updateAllDallasTimeSettings, unsigned int dallasTimerWaitSettings) {
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
  DS18B20.setWaitForConversion(false); //async 1wire during next loops
}

void readNewDallasTemp(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  char log_msg[256];
  char mqtt_topic[256];
  bool updatenow = false;

  if (millis() > nextalldatatime_dallas) {
    updatenow = true;
    nextalldatatime_dallas = millis() + (1000 * updateAllDallasTime);
  }

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

void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {
  if (millis() > (dallasTimer - 1000)) {
    DS18B20.requestTemperatures(); // get temperatures for next run 1 second before getting the temperatures (async)
  }
  if (millis() > dallasTimer) {
    log_message((char*)"Requesting new 1wire temperatures");
    dallasTimer = millis() + (1000*dallasTimerWait);
    readNewDallasTemp(actDallasData, mqtt_client, log_message, mqtt_topic_base);
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
  actS0Data[0].nextReport = millis() + MINREPORTEDS0TIME; //initial report after interval, not directly at boot

  pinMode(actS0Data[1].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Data[1].gpiopin), onS0Pulse2, RISING);
  actS0Data[1].nextReport = millis() + MINREPORTEDS0TIME; //initial report after interval, not directly at boot
}

void s0Loop(s0Data actS0Data[], PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base) {

  unsigned long millisThisLoop = millis();

  for (int i = 0 ; i < NUM_S0_COUNTERS ; i++) {
    //first handle new detected pulses
    noInterrupts();
    unsigned long new_pulse = new_pulse_s0[i];
    interrupts();
    unsigned long pulseInterval = new_pulse - actS0Data[i].lastPulse;
    if (pulseInterval > 50L) { //50ms debounce filter, this also prevents division by zero to occur a few lines further down the road if pulseInterval = 0
      if (actS0Data[i].lastPulse > 0) { //Do not calculate watt for the first pulse since reboot because we will always report a too high watt. Better to show 0 watt at first pulse.
        actS0Data[i].watt = (3600000000.0 / pulseInterval) / actS0Data[i].ppkwh;
      }
      actS0Data[i].lastPulse = new_pulse;
      actS0Data[i].pulses++;
      if ((actS0Data[i].nextReport - millisThisLoop) > MINREPORTEDS0TIME) { //loop was in standby interval
        actS0Data[i].nextReport = 0; // report now
      }
      Serial1.print("S0 port "); Serial1.print(i); Serial1.print(" detected pulse. Pulses since last report: "); Serial1.println(actS0Data[i].pulses);
    }

    //then report after nextReport
    if (millisThisLoop > actS0Data[i].nextReport) {
      
      unsigned long lastePulseInterval = millisThisLoop - actS0Data[i].lastPulse;
      unsigned long calcMaxWatt = (3600000000.0 / lastePulseInterval) / actS0Data[i].ppkwh;

      if (actS0Data[i].watt < ((3600 * 1000 / actS0Data[i].ppkwh) / actS0Data[i].lowerPowerInterval) ) { //watt is lower than possible in lower power interval time
        //Serial1.println("===In standby mode===");
        actS0Data[i].nextReport = millisThisLoop + 1000 * actS0Data[i].lowerPowerInterval;
        if ((actS0Data[i].watt)/2 > calcMaxWatt) {
          //Serial1.println("===Previous standby watt is too high. Lowering watt, divide by two===");
          actS0Data[i].watt = calcMaxWatt / 2;
        }
      }
      else {
        actS0Data[i].nextReport = millisThisLoop + MINREPORTEDS0TIME;
        if (actS0Data[i].watt > calcMaxWatt) { 
          //Serial1.println("===Previous watt is too high. Setting watt to max possible watt===");
          actS0Data[i].watt = calcMaxWatt;
        }
      }

      //calculate the watthour since last message
      float Watthour = (actS0Data[i].pulses * ( 1000 / actS0Data[i].ppkwh));
      actS0Data[i].pulses = 0; //per message we report new wattHour, so pulses should be zero at start new message

      //report using mqtt
      char log_msg[256];
      char mqtt_topic[256];
      sprintf(log_msg, "Measured Watthour on S0 port %d: %.2f", (i + 1),  Watthour ); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s/Watthour/%d", mqtt_topic_base, mqtt_topic_s0, (i + 1)); mqtt_client.publish(mqtt_topic, String(Watthour).c_str(), MQTT_RETAIN_VALUES);
      sprintf(log_msg, "Calculated Watt on S0 port %d: %u", (i + 1), actS0Data[i].watt); log_message(log_msg);
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
