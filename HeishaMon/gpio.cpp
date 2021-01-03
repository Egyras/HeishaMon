#include "gpio.h"

void setupGPIO(gpioSettingsStruct gpioSettings) {
  for (int i = 0 ; i++ ; i < NUMGPIO) {
    pinMode(gpioSettings.gpioPin[i], gpioSettings.gpioMode[i]);
  }
}
