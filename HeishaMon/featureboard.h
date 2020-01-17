#include <PubSubClient.h>

void dallasLoop(PubSubClient &mqtt_client, void (*log_message)(char*));
void initDallasSensors(void (*log_message)(char*));
