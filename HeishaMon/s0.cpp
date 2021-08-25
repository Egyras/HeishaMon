#include <PubSubClient.h>
#include "commands.h"
#include "s0.h"
#include <RingBuf.h>

#define MQTT_RETAIN_VALUES 1 // do we retain 1wire values?

#define MINREPORTEDS0TIME 5000 // how often s0 Watts are reported (not faster than this)

//global array for s0 data
s0DataStruct actS0Data[NUM_S0_COUNTERS];

//global array for s0 Settings
s0SettingsStruct actS0Settings[NUM_S0_COUNTERS];

//volatile pulse timer ringbuffer for s0
//pushing timers into a 10 long buffer so we hopefully don't miss pulses while processing noise
struct s0Pulse {
  unsigned long startPulse;
  unsigned long endPulse;
};
RingBuf<s0Pulse, 10> pulseRingBuffer[2];
volatile unsigned long fallingPulseS0_1;
volatile unsigned long fallingPulseS0_2;


//These are the interrupt routines. Make them as short as possible so we don't block other interrupts (for example serial data)
//We are using seperate rising and falling routines as 'changing' doesn't tell us if it was a start of a pulse (falling) or end (rising)
IRAM_ATTR void onS0Pulse1Rising(); //predefine
IRAM_ATTR void onS0Pulse2Rising(); //predefine

IRAM_ATTR void onS0Pulse1Falling() {
  fallingPulseS0_1 = millis();
  attachInterrupt(digitalPinToInterrupt(actS0Settings[0].gpiopin), onS0Pulse1Rising, RISING);
}

IRAM_ATTR void onS0Pulse1Rising() {
  unsigned long risingPulse = millis();
  s0Pulse newPulse = {fallingPulseS0_1, risingPulse};
  pulseRingBuffer[0].push(newPulse);
  attachInterrupt(digitalPinToInterrupt(actS0Settings[0].gpiopin), onS0Pulse1Falling, FALLING);
}

IRAM_ATTR void onS0Pulse2Falling() {
  fallingPulseS0_2 = millis();
  attachInterrupt(digitalPinToInterrupt(actS0Settings[1].gpiopin), onS0Pulse2Rising, RISING);
}

IRAM_ATTR void onS0Pulse2Rising() {
  unsigned long risingPulse = millis();
  s0Pulse newPulse = {fallingPulseS0_2, risingPulse};
  pulseRingBuffer[1].push(newPulse);
  attachInterrupt(digitalPinToInterrupt(actS0Settings[1].gpiopin), onS0Pulse2Falling, FALLING);
}

void initS0Sensors(s0SettingsStruct s0Settings[]) {
  //setup s0 port 1
  actS0Settings[0].gpiopin = s0Settings[0].gpiopin;
  actS0Settings[0].ppkwh = s0Settings[0].ppkwh;
  actS0Settings[0].lowerPowerInterval = s0Settings[0].lowerPowerInterval;

  pinMode(actS0Settings[0].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Settings[0].gpiopin), onS0Pulse1Falling, FALLING);
  actS0Data[0].nextReport = millis() + MINREPORTEDS0TIME; //initial report after interval, not directly at boot

  //setup s0 port 2
  actS0Settings[1].gpiopin = s0Settings[1].gpiopin;
  actS0Settings[1].ppkwh = s0Settings[1].ppkwh;
  actS0Settings[1].lowerPowerInterval = s0Settings[1].lowerPowerInterval;
  pinMode(actS0Settings[1].gpiopin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(actS0Settings[1].gpiopin), onS0Pulse2Falling, FALLING);
  actS0Data[1].nextReport = millis() + MINREPORTEDS0TIME; //initial report after interval, not directly at boot
}

void restore_s0_Watthour(int s0Port, float watthour) {
  if ((s0Port == 1) || (s0Port == 2)) {
    unsigned int newTotal = int(watthour * (actS0Settings[s0Port - 1].ppkwh / 1000.0));
    if (newTotal > actS0Data[s0Port - 1].pulsesTotal) actS0Data[s0Port - 1].pulsesTotal = newTotal;
  }
}

void s0SettingsCorrupt(s0SettingsStruct s0Settings[], void (*log_message)(char*)) {
  for (int i = 0 ; i < NUM_S0_COUNTERS ; i++) {
    if ((s0Settings[i].gpiopin != actS0Settings[i].gpiopin) || (s0Settings[i].ppkwh != actS0Settings[i].ppkwh) || (s0Settings[i].lowerPowerInterval != actS0Settings[i].lowerPowerInterval)) {
      char log_msg[256];
      sprintf_P(log_msg, PSTR("S0 settings got corrupted, rebooting!") ); log_message(log_msg);
      delay(1000);
      ESP.restart();
    }
  }
}

void s0Loop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, s0SettingsStruct s0Settings[]) {

  //check for corruption
  s0SettingsCorrupt(s0Settings, log_message);

  unsigned long millisThisLoop = millis();

  for (int i = 0 ; i < NUM_S0_COUNTERS ; i++) {
    char tmp_log_msg[256];
    unsigned long pulseInterval = 0;

    s0Pulse curPulse;

    //first handle new detected pulses
    if (pulseRingBuffer[i].lockedPop(curPulse)) {
      unsigned long curPulseWidth = curPulse.endPulse - curPulse.startPulse;
      if ( (curPulseWidth >  actS0Settings[i].minimalPulseWidth) && (curPulseWidth < 10 * actS0Settings[i].minimalPulseWidth) ) { //within pulse width and pulse width * 10
        pulseInterval = curPulse.startPulse - actS0Data[i].lastPulse;
        if (pulseInterval > actS0Settings[i].minimalPulseWidth ) {
          sprintf_P(tmp_log_msg, PSTR("S0 port %i pulse counted as valid! (pulse width: %lu, interval: %lu)"),  i + 1, curPulseWidth, pulseInterval);
          log_message(tmp_log_msg);
          actS0Data[i].goodPulses++;
          if (actS0Data[i].lastPulse > 0) { //Do not calculate watt for the first pulse since reboot because we will always report a too high watt. Better to show 0 watt at first pulse.
            actS0Data[i].watt = (3600000000.0 / pulseInterval) / actS0Settings[i].ppkwh;
          }
          actS0Data[i].lastPulse = curPulse.startPulse;
          actS0Data[i].pulses++;

          if ((unsigned long)(actS0Data[i].nextReport - millisThisLoop) > MINREPORTEDS0TIME) { //loop was in standby interval
            actS0Data[i].nextReport = 0; // report now
          }
        } else {
          sprintf_P(tmp_log_msg, PSTR("S0 port %i pulse counted as invalid! (pulse width: %lu, interval: %lu)"),  i + 1, curPulseWidth, pulseInterval);
          log_message(tmp_log_msg);
          actS0Data[i].badPulses++;
        }
      } else {
        sprintf_P(tmp_log_msg, PSTR("S0 port %i pulse reset. Noise detected! (pulse width: %lu)"),  i + 1, curPulseWidth);
        log_message(tmp_log_msg);
        actS0Data[i].badPulses++;
      }


    }


    //then report after nextReport
    if (millisThisLoop > actS0Data[i].nextReport) {

      unsigned long lastPulseInterval = millisThisLoop - actS0Data[i].lastPulse;
      unsigned long calcMaxWatt = (3600000000.0 / lastPulseInterval) / actS0Settings[i].ppkwh;

      if (actS0Data[i].watt < ((3600000.0 / actS0Settings[i].ppkwh) / actS0Settings[i].lowerPowerInterval) ) { //watt is lower than possible in lower power interval time
        actS0Data[i].nextReport = millisThisLoop + 1000 * actS0Settings[i].lowerPowerInterval;
        if ((actS0Data[i].watt) / 2 > calcMaxWatt) {
          actS0Data[i].watt = calcMaxWatt / 2;
        }
      }
      else {
        actS0Data[i].nextReport = millisThisLoop + MINREPORTEDS0TIME;
        if (actS0Data[i].watt > calcMaxWatt) {
          actS0Data[i].watt = calcMaxWatt;
        }
      }

      float Watthour = (actS0Data[i].pulses * ( 1000.0 / actS0Settings[i].ppkwh));
      actS0Data[i].pulsesTotal = actS0Data[i].pulsesTotal + actS0Data[i].pulses;
      actS0Data[i].pulses = 0; //per message we report new wattHour, so pulses should be zero at start new message


      //report using mqtt
      char log_msg[256];
      char mqtt_topic[256];
      char valueStr[20];
      sprintf_P(log_msg, PSTR("Measured Watthour on S0 port %d: %.2f"), (i + 1),  Watthour );
      log_message(log_msg);
      sprintf(valueStr, "%.2f", Watthour);
      sprintf_P(mqtt_topic, PSTR("%s/%s/Watthour/%d"), mqtt_topic_base, mqtt_topic_s0, (i + 1));
      mqtt_client.publish(mqtt_topic, valueStr, MQTT_RETAIN_VALUES);
      float WatthourTotal = (actS0Data[i].pulsesTotal * ( 1000.0 / actS0Settings[i].ppkwh));
      sprintf(log_msg, PSTR("Measured total Watthour on S0 port %d: %.2f"), (i + 1),  WatthourTotal );
      log_message(log_msg);
      sprintf(valueStr, "%.2f", WatthourTotal);
      sprintf(mqtt_topic, PSTR("%s/%s/WatthourTotal/%d"), mqtt_topic_base, mqtt_topic_s0, (i + 1));
      mqtt_client.publish(mqtt_topic, valueStr, MQTT_RETAIN_VALUES);
      sprintf(log_msg, PSTR("Calculated Watt on S0 port %d: %u"), (i + 1), actS0Data[i].watt);
      log_message(log_msg);
      sprintf(valueStr, "%u",  actS0Data[i].watt);
      sprintf(mqtt_topic, PSTR("%s/%s/Watt/%d"), mqtt_topic_base, mqtt_topic_s0, (i + 1));
      mqtt_client.publish(mqtt_topic, valueStr, MQTT_RETAIN_VALUES);
    }
  }
}

String s0TableOutput() {
  String output = F("");
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + F("<tr>");
    output = output + F("<td>") + (i + 1) + F("</td>");
    output = output + F("<td>") + actS0Data[i].watt + F("</td>");
    output = output + F("<td>") + (actS0Data[i].pulses * ( 1000.0 / actS0Settings[i].ppkwh)) + F("</td>");
    output = output + F("<td>") + (actS0Data[i].pulsesTotal * ( 1000.0 / actS0Settings[i].ppkwh)) + F("</td>");
    output = output + F("<td>") + (100 * (actS0Data[i].goodPulses + 1) / (actS0Data[i].goodPulses + actS0Data[i].badPulses + 1)) + F("% </td>");
    output = output + F("</tr>");
  }
  return output;
}

String s0JsonOutput() {
  String output = F("[");
  for (int i = 0; i < NUM_S0_COUNTERS; i++) {
    output = output + F("{");
    output = output + F("\"S0 port\": \"") + (i + 1) + F("\",");
    output = output + F("\"Watt\": \"") + actS0Data[i].watt + F("\",");
    output = output + F("\"Watthour\": \"") + (actS0Data[i].pulses * ( 1000.0 / actS0Settings[i].ppkwh)) + F("\",");
    output = output + F("\"WatthourTotal\": \"") + (actS0Data[i].pulsesTotal * ( 1000.0 / actS0Settings[i].ppkwh)) + F("\"");
    output = output + F("\"Pulse Quality\": \"") + (100 * (actS0Data[i].goodPulses + 1) / (actS0Data[i].goodPulses + actS0Data[i].badPulses + 1)) + F("\"");
    output = output + F("}");
    if (i < NUM_S0_COUNTERS - 1) output = output + F(",");
  }
  output = output + F("]");
  return output;
}
