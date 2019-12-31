# MQTT Topics for Panasonic Aquarea firmware

## Sensors:

Topic | Response
--- | ---
panasonic_heat_pump/sdc/PumpFlow | L/min
panasonic_heat_pump/sdc/mode_state | Current operating mode, valid responses are Heat, DHW, Heat+DHW, Auto+DHW, Cool+DHW
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
panasonic_heat_pump/sdc/quiet_mode_state | Quiet mode state, valid responses are 0-3
panasonic_heat_pump/sdc/Holiday | Holiday mode, valid responses are 84=Off and 100=On

## Commands:
Topic | Values
--- | ---