#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define UPDATEALLTIME_DALLAS 300000 // how often all dallas data is cleared and so resend to mqtt
#define MAX_DALLAS_SENSORS 15
#define ONE_WIRE_BUS 4  // DS18B20 pin, for now a static config - should be in config menu later
#define NUM_S0_COUNTERS 2
#define S0_PIN_1 12  // S0_1 pin, for now a static config - should be in config menu later
#define S0_PIN_2 14  // S0_2 pin, for now a static config - should be in config menu later

struct dallasData {
  float temperature = -127;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  String address = "";
};





struct s0Data {
  byte gpiopin = 255; 
  unsigned int ppwh = 1; //pulses per Wh of the connected meter
  unsigned int watt = 0;
  unsigned long lastBlink = 0;
  unsigned long pulseInterval = 0;
};


void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*));
void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*));
String dallasJsonOutput(dallasData actDallasData[]);
String dallasTableOutput(dallasData actDallasData[]);
void initS0Sensors();
void s0Loop(PubSubClient &mqtt_client, void (*log_message)(char*));
String s0TableOutput();
String s0JsonOutput();
