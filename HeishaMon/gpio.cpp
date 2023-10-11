#include "gpio.h"

void setupGPIO(gpioSettingsStruct gpioSettings)
{
  for (int i = 0; i < NUMGPIO; i++)
  {
    pinMode(gpioSettings.gpioPin[i], gpioSettings.gpioMode[i]);
  }
}
