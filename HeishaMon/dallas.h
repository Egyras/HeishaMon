#ifndef _DALLAS_H_
#define _DALLAS_H_

#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "src/common/webserver.h"

#define MAX_DALLAS_SENSORS 15
#if defined(ESP8266)
#define ONE_WIRE_BUS 4
#elif defined(ESP32)
#define ONE_WIRE_BUS 3
#endif

struct dallasDataStruct {
  float temperature = -127.0;
  unsigned long lastgoodtime = 0;
  DeviceAddress sensor;
  char address[17];
  char alias[32] = "NOT SET";
};

void resetlastalldatatime_dallas();
void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base);
void initDallasSensors(void (*log_message)(char*), unsigned int updataAllDallasTimeSettings, unsigned int dallasTimerWaitSettings, unsigned int dallasResolution);
void dallasJsonOutput(struct webserver_t *client);
void dallasTableOutput(struct webserver_t *client);
void changeDallasAlias(char* address, char* alias);

#endif
