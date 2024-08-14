#include "gpio.h"
#include "src/common/progmem.h"
#include "src/common/stricmp.h"
const char* mqtt_topic_gpio PROGMEM = "gpio";

void log_message(char* string);

void setupGPIO(gpioSettingsStruct gpioSettings) {
  for (int i = 0 ;  i < NUMGPIO ; i++) {
    pinMode(gpioSettings.gpioPin[i], gpioSettings.gpioMode[i]);
  }
}

void mqttGPIOCallback(char* topic, char* value) {
  log_message(_F("GPIO: MQTT message received"));
#ifdef ESP32  
  if (strcmp_P(PSTR("relay/one"), topic) == 0) {
    log_message(_F("GPIO: MQTT message received 'relay/one'"));
    digitalWrite(relayOnePin,((stricmp((char*)"true", value) == 0) || (stricmp((char*)"on", value) == 0)  || (stricmp((char*)"enable", value) == 0)|| (String(value).toInt() == 1 )));
  } else if (strcmp_P(PSTR("relay/two"), topic) == 0) {
    log_message(_F("GPIO: MQTT message received 'relay/two'"));
    digitalWrite(relayTwoPin,((stricmp((char*)"true", value) == 0) || (stricmp((char*)"on", value) == 0)  || (stricmp((char*)"enable", value) == 0)|| (String(value).toInt() == 1 )));
  }
#endif
}