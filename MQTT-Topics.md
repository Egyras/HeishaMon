# MQTT Topics for Panasonic Aquarea firmware

## Availability topic
panasonic_heat_pump/LWT will return Online when ESP is online, otherwise it will automatically return Offline

## Sensors:

Topic | Response
--- | ---
panasonic_heat_pump/log | Log of responses from pump
panasonic_heat_pump/sdc/PumpFlow | Water pump flow, measured in L/min
panasonic_heat_pump/sdc/Mode_State | Current operating mode, valid responses are Heat, DHW, Cool, Auto, Heat+DHW, Auto+DHW, Cool+DHW
panasonic_heat_pump/sdc/Flow_Inlet_Temp | Inlet water temperature in °C
panasonic_heat_pump/sdc/Flow_Outlet_Temp | Outlet water temperature in °C
panasonic_heat_pump/sdc/Flow_Target_Temp | Outlet water target temperature in °C
panasonic_heat_pump/sdc/CompFreq | Current compressor frequency
panasonic_heat_pump/sdc/Tank_Target_Temp | Tank temperature setpoint in °C
panasonic_heat_pump/sdc/Tank_Temp | Actual Tank temperature in °C
panasonic_heat_pump/sdc/Operations_Hours | Pump operating time in Hours
panasonic_heat_pump/sdc/Operations_Counter | Pump start/stop counter
panasonic_heat_pump/sdc/HeatShiftTemp | Heatshift (-5 to 5) or direct heat (20 to 50) temperature in °C
panasonic_heat_pump/sdc/Outside_Temp | Outside ambient temperature measured by compressor in °C
panasonic_heat_pump/sdc/Energy_Production | Heat power produced in Watts
panasonic_heat_pump/sdc/Energy_Consumtion | Power consume by compressor in Watts
panasonic_heat_pump/sdc/Powerfull_Mode_State | Powerfull state in minutes, valid responses are 0, 30, 60 or 90
panasonic_heat_pump/sdc/Quiet_Mode_State | Quiet mode state, valid responses are 0, 1, 2, 3
panasonic_heat_pump/sdc/Holiday_Mode_State | Holiday mode, valid responses are 84=Off and 100=On
panasonic_heat_pump/sdc/Valve_State | 3-way valve mode, valid responses are Room, Tank or Defrost
panasonic_heat_pump/sdc/Outside_Pipe_Temp | Outdoor pipe temperature used for defrost
panasonic_heat_pump/sdc/Tank_Heat_Delta | Tank delta K
panasonic_heat_pump/sdc/Heat_Delta | Heat delta K
panasonic_heat_pump/sdc/Cool_Delta | Cool delta K
panasonic_heat_pump/sdc/Quiet_Mode_State | Current silent mode state ( ???? )
panasonic_heat_pump/sdc/Defrosting_State | Current defrost state ( ???? )
panasonic_heat_pump/sdc/Heat_Shift_Temp | Temperatur shift (-5 to 5) or direct heat temperatur (20 to 50)
panasonic_heat_pump/sdc/Cool_Shift_Temp | Temperatur shift (-5 to 5) or direct cool temperatur (?? to ??)

## Commands:
Topic | Description | Values
--- | --- | ---
panasonic_heat_pump/SetHoliday | Set holiday mode on or off | 84 = Off, 100 = On
panasonic_heat_pump/SetQuietMode | Set quiet mode level | 0, 1, 2 or 3
panasonic_heat_pump/SetPowerfull | Set powerfull mode run time in minutes | 0, 30, 60 or 90
panasonic_heat_pump/SetShiftTemperature | Set heatshift or direct heat temperature | -5 to 5 or 20 to 50
panasonic_heat_pump/SetMode | Sets operating mode | Heat, Cool, DHW, AUto, Heat+DHW, Auto+DHW or Cool+DHW
panasonic_heat_pump/SetForceDHW | Forces DHW mode only | 1
panasonic_heat_pump/SetTankTemp | Set tank target temperature | 40 - 75
panasonic_heat_pump/SetCoolTemp | Set cooldown temperature | 5 - 20
panasonic_heat_pump/SetForceDefrost | Forces defrost routine | 1
panasonic_heat_pump/SetForceSterilization | Forces tank sterilization routine | 1
