#include <PubSubClient.h>

// opentherm
const int inOTPin = 3; //RX pin from ESP8266
const int outOTPin = 1; //TX pin from ESP8266

struct heishaOTDataStruct {
  //master value
  bool chEnable;
  float roomTemp;
  float roomTempSet;
  float chSetpoint;
  //slave values
  float outsideTemp = 0;
  float inletTemp = 0;
  float outletTemp = 0;
  bool flameState = false;
};

void HeishaOTSetup();
void HeishaOTLoop(char *actDat, PubSubClient &mqtt_client, char* mqtt_topic_base);
