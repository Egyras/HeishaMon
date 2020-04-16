#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define UPDATEALLTIME_DALLAS 300000 // how often all dallas data is cleared and so resend to mqtt
#define MAX_DALLAS_SENSORS 15
#define ONE_WIRE_BUS 4  // DS18B20 pin, for now a static config - should be in config menu later
#define NUM_S0_COUNTERS 2
#define DEFAULT_S0_PIN_1 12  // S0_1 pin, for now a static config - should be in config menu later
#define DEFAULT_S0_PIN_2 14  // S0_2 pin, for now a static config - should be in config menu later
#define MIN_S0_WATT 100 //below this, the s0 reports 0 watt

struct dallasData {
  float temperature = -127;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  String address = "";
};


struct s0Data {
  byte gpiopin = 255; 
  unsigned int ppkwh = 1000; //pulses per Wh of the connected meter
  unsigned int pulses = 0; //number of pulses since last report
  unsigned int watt = 0; //calculated average power
  unsigned int minWatt = MIN_S0_WATT; //report 0 Watt below this filter
  unsigned long lastPulse = 0; //last pulse in millis
  unsigned long pulseInterval = 0; //interval in between last two pulses in millis
};


void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*));
void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*));
String dallasJsonOutput(dallasData actDallasData[]);
String dallasTableOutput(dallasData actDallasData[]);
void initS0Sensors(s0Data actS0Data[]);
void s0Loop(s0Data actS0Data[],PubSubClient &mqtt_client, void (*log_message)(char*));
String s0TableOutput(s0Data actS0Data[]);
String s0JsonOutput(s0Data actS0Data[]);
