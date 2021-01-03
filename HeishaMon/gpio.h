#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define NUMGPIO 3

struct gpioSettingsStruct {
  unsigned int gpioPin[NUMGPIO] = {1,3,16}; 
  unsigned int gpioMode[NUMGPIO] = {INPUT_PULLUP,INPUT_PULLUP,INPUT_PULLUP}; 

};

void setupGPIO(gpioSettingsStruct gpioSettings);
