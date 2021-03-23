struct SmartControlSettingsStruct {
  bool enableHeatCurve = false; //Enable or dissable heating curve control from Heishamon

  short avgHourHeatCurve = 0; // Outside temperature average of hours for heating curve control
  short heatCurveTargetHigh = 60; // Heating curve target high temperature
  short heatCurveTargetLow = 20; // Heating curve target low temperature
  short heatCurveOutHigh = 15; // Heating curve outside high temperature
  short heatCurveOutLow = -20; // Heating curve outside low temperature

  short heatCurveLookup[36] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Lookup table for heating curve
};

String getAvgOutsideTemp(void);

void smartControlLoop(void (*log_message)(char*), SmartControlSettingsStruct SmartControlSettings, String actData[], unsigned long goodreads);
