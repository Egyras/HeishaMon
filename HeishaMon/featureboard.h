#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define UPDATEALLTIME_DALLAS 300000 // how often all dallas data is cleared and so resend to mqtt
#define MAX_DALLAS_SENSORS 15
#define ONE_WIRE_BUS 4  // DS18B20 pin

struct dallasData {
  float temperature = -127;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  String address = "";
};


#define MAX_S0_COUNTERS 2

struct s0Data {
  byte gpiopin = 255; 
  unsigned int ppkwh = 1000; //pulses per kWh of the connected meter
  unsigned int watt = 0;
  unsigned long lastBlink = 0;
  unsigned long pulseInterval = 0;
};


void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*));
void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*));
String dallasJsonOutput(dallasData actDallasData[]);
String dallasTableOutput(dallasData actDallasData[]);
