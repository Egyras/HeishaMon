#include <PubSubClient.h>
#include "src/common/webserver.h"

// opentherm
const int inOTPin = 3; //RX pin from ESP8266
const int outOTPin = 1; //TX pin from ESP8266

// mqtt
extern const char* mqtt_topic_opentherm;

struct heishaOTDataStruct {
  //WRITE values
  bool chEnable; //is central heating enabled by thermostat
  float roomTemp; //what is measured room temp by thermostat
  float roomTempSet; //what is request room temp setpoint by thermostat
  float chSetpoint; //what is calculated Ta setpoint by thermostat
  //READ AND WRITE values
  float dhwSetpoint = 65; //what is DHW setpoint by thermostat
  float maxTSet = 65; //max ch setpoint
  //READ values
  float outsideTemp = 0; //provides measured outside temp to thermostat
  float inletTemp = 0; //provides measured Treturn temp to thermostat
  float outletTemp = 0; //provides measured Tout (boiler) temp to thermostat
  float dhwTemp = 0; //provides measured dhw water temp to to thermostat
  bool flameState = false; //provides current flame state to thermostat
  bool chState = false; //provides if boiler is in centrale heating state
  bool dhwState = false; //provides if boiler is in dhw heating state
};

void HeishaOTSetup();
void HeishaOTLoop(char *actDat, PubSubClient &mqtt_client, char* mqtt_topic_base);
void mqttOTCallback(char* topic, char* value);
void openthermTableOutput(struct webserver_t *client);
void openthermJsonOutput(struct webserver_t *client);
