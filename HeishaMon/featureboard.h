#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define MAX_DALLAS_SENSORS 15
struct dallasData {
  float temperature = -127;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  String address = "";
};

void dallasLoop(dallasData actDallasData[], PubSubClient &mqtt_client, void (*log_message)(char*));
void initDallasSensors(dallasData actDallasData[], void (*log_message)(char*));
String dallasJsonOutput(dallasData actDallasData[]);
