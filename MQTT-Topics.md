# MQTT Topics for Panasonic Aquarea firmware

## Availability topic
panasonic_heat_pump/LWT will return Online when ESP is online, otherwise it will automatically return Offline

## Sensors:

Topic | Response
--- | ---
panasonic_heat_pump/log | Log of responses from pump
panasonic_heat_pump/sdc/PumpFlow | Water pump flow, measured in L/min
panasonic_heat_pump/sdc/mode_state | Current operating mode, valid responses are Heat, DHW, Cool, Auto, Heat+DHW, Auto+DHW, Cool+DHW
panasonic_heat_pump/sdc/InletTemp | Inlet water temperature in °C
panasonic_heat_pump/sdc/ActWatOutTemp | Outlet water temperature in °C
panasonic_heat_pump/sdc/WatOutTarTemp | Outlet water target temperature in °C
panasonic_heat_pump/sdc/CompFreq | Current compressor frequency
panasonic_heat_pump/sdc/TankSetTemp | Tank temperature setpoint in °C
panasonic_heat_pump/sdc/ActTankTemp | Actual tank temperature in °C
panasonic_heat_pump/sdc/OperatingTime | Pump operating time in Hours
panasonic_heat_pump/sdc/OperationsNumber | Pump start/stop counter
panasonic_heat_pump/sdc/HeatShiftTemp | Heatshift temperature (-5 to 5) in °C
panasonic_heat_pump/sdc/ActOutTemp | Outside ambient temperature measured by compressor in °C
panasonic_heat_pump/sdc/Eproduce | Heat power produced in Watts
panasonic_heat_pump/sdc/Econsum | Power consume by compressor in Watts
panasonic_heat_pump/sdc/powerfull_mode_state | Powerfull state in minutes, valid responses are 0, 30, 60 or 90
panasonic_heat_pump/sdc/quiet_mode_state | Quiet mode state, valid responses are 0, 1, 2, 3
panasonic_heat_pump/sdc/Holiday | Holiday mode, valid responses are 84=Off and 100=On
panasonic_heat_pump/sdc/valve_state | 3-way valve mode, valid responses are Room, Tank or Defrost
panasonic_heat_pump/sdc/OutPipeTemp | Outdoor pipe temperature used for defrost

## Commands:
Topic | Description | Values
--- | --- | ---
panasonic_heat_pump/SetHoliday | Set holiday mode on or off | 84 = Off, 100 = On
panasonic_heat_pump/SetQuietMode | Set quiet mode level | 0, 1, 2 or 3
panasonic_heat_pump/SetPowerfull | Set powerfull mode run time in minutes | 0, 30, 60 or 90
panasonic_heat_pump/SetShiftTemperature | Set heatshift temperature | -5 to 5
panasonic_heat_pump/SetMode | Sets operating mode | Heat, Cool, DHW, AUto, Heat+DHW, Auto+DHW or Cool+DHW
panasonic_heat_pump/SetForceDHW | Forces DHW mode only | 1
panasonic_heat_pump/SetTankTemp | Set tank target temperature | 40 - 75
panasonic_heat_pump/SetCoolTemp | Set cooldown temperature | 5 - 20
panasonic_heat_pump/SetForceDefrost | Forces defrost routine | 1
panasonic_heat_pump/SetForceSterilization | Forces tank sterilization routine | 1
