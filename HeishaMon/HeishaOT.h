#include <PubSubClient.h>

// opentherm
const int inOTPin = 3; //RX pin from ESP8266
const int outOTPin = 1; //TX pin from ESP8266

// mqtt
extern const char* mqtt_topic_opentherm;

struct heishaOTDataStruct {
  //master value
  bool chEnable; //is central heating enabled by thermostat
  float roomTemp; //what is measured room temp by thermostat
  float roomTempSet; //what is request room temp setpoint by thermostat
  float chSetpoint; //what is calculated Ta setpoint by thermostat
  float dhwSetpoint; //what is DHW setpoint by thermostat
  //slave values
  float outsideTemp = 0; //provides measured outside temp to thermostat
  float inletTemp = 0; //provides measured Tr temp to thermostat
  float outletTemp = 0; //provides measured Ta temp to thermostat
  float boilerTemp = 0; //provides measured boiler water temp to thermostat
  float DHWTemp = 0; //provides measured dhw water temp to to thermostat
  bool flameState = false; //provides current flame state to thermostat
  bool chState = false; //provides if boiler is in centrale heating state
  bool dhwState = false; //provides if boiler is in dhw heating state
};

void HeishaOTSetup();
void HeishaOTLoop(char *actDat, PubSubClient &mqtt_client, char* mqtt_topic_base);
void mqttOTCallback(char* topic, char* value);
