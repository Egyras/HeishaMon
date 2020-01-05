#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define UPDATEALLTIME 300000 // how often all data is cleared and so resend to mqtt
#define MQTT_RETAIN_VALUES 1


void decode_heatpump_data(char* data, DynamicJsonDocument &actData, PubSubClient &mqtt_client, void (*log_message)(char*));
