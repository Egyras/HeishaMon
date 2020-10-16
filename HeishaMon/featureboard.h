#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MAX_DALLAS_SENSORS 15
#define ONE_WIRE_BUS 4  // DS18B20 pin, for now a static config - should be in config menu later
#define NUM_S0_COUNTERS 2
#define DEFAULT_S0_PIN_1 12  // S0_1 pin, for now a static config - should be in config menu later
#define DEFAULT_S0_PIN_2 14  // S0_2 pin, for now a static config - should be in config menu later


struct dallasDataStruct {
  float temperature = -127.0;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  char address[17];
};

struct s0SettingsStruct {
  byte gpiopin = 255; 
  unsigned int ppkwh = 1000; //pulses per Wh of the connected meter
  unsigned int lowerPowerInterval = 60; //configurabel low power interval
  bool sum_s0_watthour = false; //count watthour upwards, do not reset between messages
};

struct s0DataStruct {
  unsigned int pulses = 0; //number of pulses since last report
  unsigned int watt = 0; //calculated average power
  unsigned long lastPulse = 0; //last pulse in millis
  unsigned long nextReport = 0; //next time we reported the s0 value in millis
};


void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base);
void initDallasSensors(void (*log_message)(char*), unsigned int updataAllDallasTimeSettings, unsigned int dallasTimerWaitSettings);
String dallasJsonOutput(void);
String dallasTableOutput(void);
void initS0Sensors(s0SettingsStruct s0Settings[], PubSubClient &mqtt_client, char* mqtt_topic_base);
void restore_s0_Watthour(int s0Port,float watthour);
void s0Loop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, s0SettingsStruct s0Settings[]);
String s0TableOutput(void);
String s0JsonOutput(void);
