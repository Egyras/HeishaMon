# MQTT Topics for HeishaMon

## Availability topic:

ID | Topic | Response
--- | --- | ---
|| LWT | Online/Offline (automatically returns to Offline if connection with the HeishaMon lost)

## Log topic:

ID | Topic | Response
--- | --- | ---
LOG1 | log | Log of responses from pump

## Sensors Topics:

ID | Topic | Response
--- | --- | ---
TOP0 | sdc/Heatpump_State | Heatpump state (0=off, 1=on)
TOP1 | sdc/Pump_Flow | Pump flow (l/min)
TOP2 | sdc/ForceDHW_State | DHW status (0=off, 1=on -1=unknown)
TOP3 | sdc/Quietmode_Schedule | Quiet mode schedule (0=inactive, 1=active)
TOP4 | sdc/OperatingMode_State | Operating mode (0=Heat only, 1=Cool only, 2=Auto, 3=Tank only, 4=Heat+DHW, 5=Cool+DHW, 6=Auto+DHW)
TOP5 | sdc/Water_Inlet_Temp | Inlet water temperature (°C)
TOP6 | sdc/Water_Outlet_Temp | Outlet water temperature (°C)
TOP7 | sdc/Water_Target_Temp | Outlet water target temperature (°C)
TOP8 | sdc/Compressor_Freq | Compressor frequency (Hz)
TOP9 | sdc/DHW_Target_Temp | Tank target temperature (°C)
TOP10 | sdc/DHW_Temp | Actual Tank temperature (°C)
TOP11 | sdc/Operations_Hours | Heatpump operating time (Hour)
TOP12 | sdc/Operations_Counter | Heatpump starts (counter)
TOP13 | sdc/MainSchedule_State | Main thermostat schedule state (inactive - active)
TOP14 | sdc/Outside_Temp | Outside ambient temperature (°C)
TOP15 | sdc/Heat_Energy_Production | Thermal heat power production (Watt)
TOP16 | sdc/Heat_Energy_Consumption | Elektrical heat power consumption at heat mode (Watt)
TOP17 | sdc/Powerfullmode_Time | Powerfull state in minutes (0, 1, 2 or 3 x 30min)
TOP18 | sdc/Quietmode_Level | Quiet mode level (0, 1, 2, 3)
TOP19 | sdc/Holidaymode_State | Holiday mode (84=off, 100=on)
TOP20 | sdc/ThreewayValve_State | 3-way valve mode (0=Room, 1=DHW)
TOP21 | sdc/Outside_Pipe_Temp | Outside pipe temperature (°C)
TOP22 | sdc/DHW_Heat_Delta | Tank delta (K)
TOP23 | sdc/Heat_Delta | Heat delta (K)
TOP24 | sdc/Cool_Delta | Cool delta (K)
TOP25 | sdc/DHW_Shift_Temp | Shift tank temperatur (tbd)
TOP26 | sdc/Defrosting_State | Defrost state (0=off, 1=on)
TOP27 | sdc/Z1_HeatShift_Temp | Zone 1 Heat Shift/Targettemp (-5 to 5) or direct heat temperatur (20 to 55)
TOP28 | sdc/Z1_CoolShift_Temp | Zone 1 Cool Shift/Targettemp (-5 to 5) or direct cool temperatur (?? to ??)
TOP29 | sdc/HCurve_OutHighTemp | Target temperatur at lowest point on the heating curve (eg. 34 °C)
TOP30 | sdc/HCurve_OutLowTemp | Target temperatur at highest point on the heating curve (eg. 24°C)
TOP31 | sdc/HCurve_OutsHighTemp | Lowest outside temperatur on the heating curve (eg. -12°C)
TOP32 | sdc/HCurve_OutsLowTemp | Highest outside temperatur on the heating curve (eg. 15°C)
TOP33 | sdc/Roomthermostat_Temp | Remote control thermostat temp (°C)
TOP34 | sdc/Z2_HeatShift_Temp | Zone 2 Heatshift (-5 to 5) or direct heat temperatur (20 to 55)
TOP35 | sdc/Z2_CoolShift_Temp | Zone 2 Coolshift (-5 to 5) or direct cool temperatur (?? to ??)
TOP36 | sdc/Z1_Water_Temp | Zone 1 Room/Pool outlet temperature (°C)
TOP37 | sdc/Z2_Water_Temp | Zone 2 Room/Pool outlet temperature (°C)
TOP38 | sdc/Cool_Energy_Production | Thermal cooling power production (Watt)
TOP39 | sdc/Cool_Energy_Consumption | Elektrical cooling power consumption (Watt)
TOP40 | sdc/DHW_Energy_Production | Thermal DHW power production (Watt)
TOP41 | sdc/DHW_Energy_Consumption | Elektrical DHW power consumption (Watt)
TOP42 | sdc/Z1_Water_Target_Temp | Zone 1 water target temperature (°C)
TOP43 | sdc/Z2_Water_Target_Temp | Zone 2 water target temperature (°C)
TOP44 | sdc/Error | Last active Error from Heat Pump
TOP45 | sdc/Holiday_Shift_Temp | Shift holiday temperatur (-15 to 15)
TOP46 | sdc/Buffer_Temp | Actual Buffer temperature (°C)
TOP47 | sdc/Solar_Temp | Actual Solar temperature (°C)
TOP48 | sdc/Pool_Temp | Actual Pool temperature (°C)
TOP49 | sdc/Water_Hex_Outlet_Temp | Outlet 2 heat exchanger water temperature (°C)
TOP50 | sdc/Discharge_Temp | Discharge Temperature (°C)
TOP51 | sdc/Inside_Pipe_Temp | Inside pipe temperature (°C)
TOP52 | sdc/Defrost_Temp | Defrost temperature (°C)
TOP53 | sdc/Eva_Outlet_Temp | Eva Outlet temperature (°C)
TOP54 | sdc/Bypass_Outlet_Temp | Bypass Outlet temperature (°C)
TOP55 | sdc/Ipm_Temp | Ipm temperature (°C)
TOP56 | sdc/Z1_Temp | Zone1: Actual Temperature (°C) 
TOP57 | sdc/Z2_Temp | Zone2: Actual Temperature (°C) 
TOP58 | sdc/DHW_Heater_State | Tank electric heater allowed state (disabled - enabled)
TOP59 | sdc/WaterHeater_State | Water electric heater allowed state (disabled - enabled)
TOP60 | sdc/InternalHeater_State | Internal heater state (inactive - active)
TOP61 | sdc/ExternalHeater_State | External heater state (inactive - active)
TOP62 | sdc/Fan1Motor_Speed | Fan 1 Motor speed (R/Min)
TOP63 | sdc/Fan2Motor_Speed | Fan 2 Motor speed (R/Min)
TOP64 | sdc/High_Pressure | High Pressure (Kgf/Cm2)
TOP65 | sdc/Pump_Speed | Pump Speed (R/Min)
TOP66 | sdc/Low_Pressure | Low Pressure (Kgf/Cm2)
TOP67 | sdc/Compressor_Current | Outdoor Current (Ampere)
TOP68 | sdc/ForceHeater_State | Force heater status (0=inactive, 1=active)

## Command Topics:

Topic | Description | Values
--- | --- | ---
SetHeatpump | Set heatpump on or off | 0=off, 1=on
SetHoliday | Set holiday mode on or off | 0=off, 1=on
SetQuietMode | Set quiet mode level | 0, 1, 2 or 3
SetPowerfull | Set powerfull mode run time in minutes | 0=off, 1=30, 2=60 or 3=90
SetShiftTemperature | Set heatshift or direct heat temperature | -5 to 5 or 20 to 50
SetOpMode | Sets operating mode | 0=Heat only, 1=Cool only, 2=Auto, 3=Tank only, 4=Heat+DHW, 5=Cool+DHW, 6=Auto+DHW
SetForceDHW | Forces DHW mode only | 1
SetTankTemp | Set tank target temperature | 40 - 75
SetCoolTemp | Set cooldown temperature | 5 - 20
SetForceDefrost | Forces defrost routine | 1
SetForceSterilization | Forces tank sterilization routine | 1



*If you operate your Heisha with direct temperature, the shift values correspond to the absolute temperature*