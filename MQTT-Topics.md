# MQTT Topics for Panasonic Aquarea firmware

## Availability topic
panasonic_heat_pump/LWT will return Online when ESP is online, otherwise it will automatically return Offline

## Sensors:
Check [states description](States.md) to understand the value in State topics.

ID | Topic | Response
--- | --- | ---
LOG1 | panasonic_heat_pump/log | Log of responses from pump
TOP1 | panasonic_heat_pump/sdc/Pump_Flow | Water pump flow, measured in L/min
TOP2 | panasonic_heat_pump/sdc/ForceDHW_State | Forced DHW status (off - on)
TOP3 | panasonic_heat_pump/sdc/Power_State | Current Power state (off - on)
TOP4 | panasonic_heat_pump/sdc/OpMode_State | Current operating mode, valid responses are Heat, DHW, Cool, Auto, Heat+DHW, Auto+DHW, Cool+DHW
TOP5 | panasonic_heat_pump/sdc/Z1_Flow_Inlet_Temp | Zone 1 Inlet water temperature in °C
TOP6 | panasonic_heat_pump/sdc/Z1_Flow_Outlet_Temp | Zone 1 Outlet water temperature in °C
TOP7 | panasonic_heat_pump/sdc/Z1_Flow_Target_Temp | Zone 1 Outlet water target temperature in °C
TOP8 | panasonic_heat_pump/sdc/Compressor_Freq | Current compressor frequency
TOP9 | panasonic_heat_pump/sdc/Tank_Target_Temp | Tank temperature setpoint in °C
TOP10 | panasonic_heat_pump/sdc/Tank_Temp | Actual Tank temperature in °C
TOP11 | panasonic_heat_pump/sdc/Operations_Hours | Pump operating time in Hours
TOP12 | panasonic_heat_pump/sdc/Operations_Counter | Pump start/stop counter
TOP13 | panasonic_heat_pump/sdc/MainSchedule_State | Main thermostat schedule state (inactive - active)
TOP14 | panasonic_heat_pump/sdc/Outside_Temp | Outside ambient temperature measured by compressor in °C
TOP15 | panasonic_heat_pump/sdc/Heat_Energy_Production | Thermal heat power produced in Watt
TOP16 | panasonic_heat_pump/sdc/Heat_Energy_Consumption | Elektrical power consume at heat mode in Watt (steps of 200)
TOP17 | panasonic_heat_pump/sdc/Powerfullmode_Time | Powerfull state in minutes, valid responses are 0, 1, 2 or 3
TOP18 | panasonic_heat_pump/sdc/Quietmode_Level | Quiet mode state, valid responses are 0, 1, 2, 3
TOP19 | panasonic_heat_pump/sdc/Holidaymode_State | Holiday mode, valid responses are 84=Off and 100=On
TOP20 | panasonic_heat_pump/sdc/Valve_State | 3-way valve mode, valid responses are Room, Tank or Defrost
TOP21 | panasonic_heat_pump/sdc/Outside_Pipe_Temp | Outdoor pipe temperature used for defrost
TOP22 | panasonic_heat_pump/sdc/Tank_Heat_Delta | Tank delta K
TOP23 | panasonic_heat_pump/sdc/Heat_Delta | Heat delta K
TOP24 | panasonic_heat_pump/sdc/Cool_Delta | Cool delta K
TOP25 | panasonic_heat_pump/sdc/Quietmode_State | Current silent mode state ( ???? )
TOP26 | panasonic_heat_pump/sdc/Defrosting_State | Current defrost state ( ???? )
TOP27 | panasonic_heat_pump/sdc/Z1_HeatShift_Temp | Zone 1 Heatshift (-5 to 5) or direct heat temperatur (20 to 55)
TOP28 | panasonic_heat_pump/sdc/Z1_CoolShift_Temp | Zone 1 Coolshift (-5 to 5) or direct cool temperatur (?? to ??)
TOP29 | panasonic_heat_pump/sdc/HCurve_OutHighTemp | Target temperatur °C at lowest point of the heating curve (eg. 34)
TOP30 | panasonic_heat_pump/sdc/HCurve_OutLowTemp | Target temperatur °C at highest point of the heating curve (eg. 24)
TOP31 | panasonic_heat_pump/sdc/HCurve_OutsHighTemp | Lowest outsite temperatur of the heating curve (eg. -12)
TOP32 | panasonic_heat_pump/sdc/HCurve_OutsLowTemp | Highest temperatur of the heating curve (eg. 15)
TOP33 | panasonic_heat_pump/sdc/Roomthermostat_Temp | Remote control thermostat temp
TOP34 | panasonic_heat_pump/sdc/Z2_HeatShift_Temp | Zone 2 Heatshift (-5 to 5) or direct heat temperatur (20 to 55)
TOP35 | panasonic_heat_pump/sdc/Z2_CoolShift_Temp | Zone 2 Coolshift (-5 to 5) or direct cool temperatur (?? to ??)
TOP36 | panasonic_heat_pump/sdc/Z1_Water_Temp | Zone 1 Actual (Water Outlet/Room/Pool) Temperature [°C]
TOP37 | panasonic_heat_pump/sdc/Z2_Water_Temp | Zone 2 Actual (Water Outlet/Room/Pool) Temperature [°C]
TOP38 | panasonic_heat_pump/sdc/Cool_Energy_Production | Thermal cool power produced in Watt
TOP39 | panasonic_heat_pump/sdc/Cool_Energy_Consumption | Elektrical power consume at cool mode in Watt (steps of 200)
TOP40 | panasonic_heat_pump/sdc/DHW_Energy_Production | Thermal DHW power produced in Watt
TOP41 | panasonic_heat_pump/sdc/DHW_Energy_Consumption | Elektrical power consume at DHW mode in Watt (steps of 200)
TOP42 | panasonic_heat_pump/sdc/Z1_Water_Target_Temp | Zone 1 water target temperature 
TOP43 | panasonic_heat_pump/sdc/Z2_Water_Target_Temp | Zone 2 water target temperature 


## Commands:
Topic | Description | Values
--- | --- | ---
panasonic_heat_pump/SetHoliday | Set holiday mode on or off | [Check states description](States.md)
panasonic_heat_pump/SetQuietMode | Set quiet mode level |[Check states description](States.md)
panasonic_heat_pump/SetPowerfull | Set powerfull mode run time in minutes | [Check states description](States.md)
panasonic_heat_pump/SetShiftTemperature | Set heatshift or direct heat temperature | -5 to 5 or 20 to 50
panasonic_heat_pump/SetOpMode | Sets operating mode | [Check states description](States.md)
panasonic_heat_pump/SetForceDHW | Set Force DHW mode | [Check states description](States.md)
panasonic_heat_pump/SetTankTemp | Set tank target temperature | 40 - 75
panasonic_heat_pump/SetCoolTemp | Set cooldown temperature | 5 - 20
panasonic_heat_pump/SetForceDefrost | Forces defrost routine | [Check states description](States.md)
panasonic_heat_pump/SetForceSterilization | Forces tank sterilization routine | [Check states description](States.md)
