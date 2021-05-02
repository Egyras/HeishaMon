#include "commands.h"
#include "smartcontrol.h"

unsigned long heatCurveTimer = 0;
bool heatCurveFirst = true;
short avgOutsideTemp = 0;
short avgOutsideTempArray[96] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// returns the calculated average outside temperature
String getAvgOutsideTemp() {
  char avgOutsideTempStr[55];
  sprintf_P(avgOutsideTempStr, PSTR("Current calculated average outside temperature: %dC."), avgOutsideTemp);
  return String(avgOutsideTempStr);
}

bool send_command(byte* command, int length);

void smartControlLoop(void (*log_message)(char*), SmartControlSettingsStruct SmartControlSettings, String actData[], unsigned long goodreads) {
  if (goodreads > 0) {
    char log_msg[256];
    if ((millis() - heatCurveTimer > (1000 * 1800)) || heatCurveFirst) { //every 0.5h
      log_message((char*)"Calculate new outside temperature average");
      heatCurveTimer = millis();

      short currentOutsideTemp = actData[14].toInt();
      if (heatCurveFirst) {
        log_message((char*)"Fill average outside temperature array");
        heatCurveFirst = false;
        for (unsigned int i = 0 ; i < 96 ; i++) {
          avgOutsideTempArray[i] = currentOutsideTemp;
        }
      } else {
        log_message((char*)"Add current outside temperature to average array");
        for (unsigned int i = 95 ; i > 0 ; i--) {
          avgOutsideTempArray[i] = avgOutsideTempArray[i - 1];
        }
        avgOutsideTempArray[0] = currentOutsideTemp;
      }

      long outsideTempSum = 0;
      for (unsigned int i = 0 ; i < ((SmartControlSettings.avgHourHeatCurve * 2) + 1); i++) {
        outsideTempSum = outsideTempSum + avgOutsideTempArray[i];
      }
      avgOutsideTemp = int(outsideTempSum / (SmartControlSettings.avgHourHeatCurve * 2));
      sprintf_P(log_msg, PSTR("Current calculated average outside temperature: %dC."), avgOutsideTemp);
      log_message(log_msg);

      log_message((char*)"Send new heat request temperature setpoint");
      short heatRequest = int(SmartControlSettings.heatCurveLookup[35]);
      if (avgOutsideTemp > 15) {
        heatRequest = int(SmartControlSettings.heatCurveLookup[35]);
      } else if (avgOutsideTemp < -20) {
        heatRequest = int(SmartControlSettings.heatCurveLookup[0]);
      } else {
        heatRequest = int(SmartControlSettings.heatCurveLookup[(avgOutsideTemp + 20)]);
      }
      sprintf(log_msg, "Current heat request temperature: %dC.", heatRequest); log_message(log_msg);

      log_msg[0] = 0;
      unsigned char cmd[256] = { 0 };
      char msg[3];
      unsigned int len = 0;
      sprintf(msg, "%d", heatRequest);
      len = set_z1_heat_request_temperature(msg, cmd, log_msg);
      log_message(log_msg);
      send_command(cmd, len);
    }
  }
}
