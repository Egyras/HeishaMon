# MQTT Topics for HeishaMon

## Availability Topic:

ID | Topic | Response
--- | --- | ---
|| LWT | Online/Offline (automatically returns to Offline if connection with the HeishaMon lost)

## Log Topic:

ID | Topic | Response
--- | --- | ---
LOG1 | log | response from headpump (level switchable)

## Sensor Topics:

ID | Topic | Response/Description
:--- | --- | ---
TOP0 | sdc/Heatpump_State | Heatpump state (0=off, 1=on)
TOP1 | sdc/Pump_Flow | Pump flow (l/min)
TOP2 | sdc/Force_DHW_State | DHW status (0=off, 1=on)
TOP3 | sdc/Quiet_Mode_Schedule | Quiet mode schedule (0=inactive, 1=active)
TOP4 | sdc/Operating_Mode_State | Operating mode (0=Heat only, 1=Cool only, 2=Auto, 3=DHW only, 4=Heat+DHW, 5=Cool+DHW, 6=Auto+DHW)
TOP5 | sdc/Main_Inlet_Temp | Main inlet water temperature (°C)
TOP6 | sdc/Main_Outlet_Temp | Main outlet water temperature (°C)
TOP7 | sdc/Main_Target_Temp | Main outlet water target temperature (°C)
TOP8 | sdc/Compressor_Freq | Compressor frequency (Hz)
TOP9 | sdc/DHW_Target_Temp | DHW target temperature (°C)
TOP10 | sdc/DHW_Temp | Actual DHW temperature (°C)
TOP11 | sdc/Operations_Hours | Heatpump operating time (Hour)
TOP12 | sdc/Operations_Counter | Heatpump starts (counter)
TOP13 | sdc/Main_Schedule_State | Main thermostat schedule state (inactive - active)
TOP14 | sdc/Outside_Temp | Outside ambient temperature (°C)
TOP15 | sdc/Heat_Energy_Production | Thermal heat power production (Watt)
TOP16 | sdc/Heat_Energy_Consumption | Elektrical heat power consumption at heat mode (Watt)
TOP17 | sdc/Powerful_Mode_Time | Powerful state in minutes (0, 1, 2 or 3 x 30min)
TOP18 | sdc/Quiet_Mode_Level | Quiet mode level (0, 1, 2, 3)
TOP19 | sdc/Holiday_Mode_State | Holiday mode (0=off, 1=scheduled, 2=active)
TOP20 | sdc/ThreeWay_Valve_State | 3-way valve mode (0=Room, 1=DHW)
TOP21 | sdc/Outside_Pipe_Temp | Outside pipe temperature (°C)
TOP22 | sdc/DHW_Heat_Delta | DHW heating delta (-12 to -2) (K)
TOP23 | sdc/Heat_Delta | Heat delta (K)
TOP24 | sdc/Cool_Delta | Cool delta (K)
TOP25 | sdc/DHW_Holiday_Shift_Temp | DHW Holiday shift temperature  (-15 to +15)
TOP26 | sdc/Defrosting_State | Defrost state (0=off, 1=on)
TOP27 | sdc/Z1_Heat_Request_Temp | Zone 1 Heat Requested shift temp (-5 to 5) or direct heat temp (20 to max)
TOP28 | sdc/Z1_Cool_Request_Temp | Zone 1 Cool Requested shift temp (-5 to 5) or direct cool temp (5 to 20)
TOP29 | sdc/Z1_Heat_Curve_Target_High_Temp | Target temperature at lowest point on the heating curve (°C)
TOP30 | sdc/Z1_Heat_Curve_Target_Low_Temp | Target temperature at highest point on the heating curve (°C)
TOP31 | sdc/Z1_Heat_Curve_Outside_High_Temp | Lowest outside temperature on the heating curve (°C)
TOP32 | sdc/Z1_Heat_Curve_Outside_Low_Temp | Highest outside temperature on the heating curve (°C)
TOP33 | sdc/Room_Thermostat_Temp | Remote control thermostat temp (°C)
TOP34 | sdc/Z2_Heat_Request_Temp | Zone 2 Heat Requested shift temp (-5 to 5) or direct heat temp (20 to max)
TOP35 | sdc/Z2_Cool_Request_Temp | Zone 2 Cool Requested shift temp (-5 to 5) or direct cool temp (5 to 20)
TOP36 | sdc/Z1_Water_Temp | Zone 1 Water outlet temperature (°C)
TOP37 | sdc/Z2_Water_Temp | Zone 2 Water outlet temperature (°C)
TOP38 | sdc/Cool_Energy_Production | Thermal cooling power production (Watt)
TOP39 | sdc/Cool_Energy_Consumption | Elektrical cooling power consumption (Watt)
TOP40 | sdc/DHW_Energy_Production | Thermal DHW power production (Watt)
TOP41 | sdc/DHW_Energy_Consumption | Elektrical DHW power consumption (Watt)
TOP42 | sdc/Z1_Water_Target_Temp | Zone 1 water target temperature (°C)
TOP43 | sdc/Z2_Water_Target_Temp | Zone 2 water target temperature (°C)
TOP44 | sdc/Error | Last active Error from Heat Pump
TOP45 | sdc/Room_Holiday_Shift_Temp | Room heating Holiday shift temperature (-15 to 15)
TOP46 | sdc/Buffer_Temp | Actual Buffer temperature (°C)
TOP47 | sdc/Solar_Temp | Actual Solar temperature (°C)
TOP48 | sdc/Pool_Temp | Actual Pool temperature (°C)
TOP49 | sdc/Main_Hex_Outlet_Temp | Outlet 2, after heat exchanger water temperature (°C)
TOP50 | sdc/Discharge_Temp | Discharge Temperature (°C)
TOP51 | sdc/Inside_Pipe_Temp | Inside pipe temperature (°C)
TOP52 | sdc/Defrost_Temp | Defrost temperature (°C)
TOP53 | sdc/Eva_Outlet_Temp | Eva Outlet temperature (°C)
TOP54 | sdc/Bypass_Outlet_Temp | Bypass Outlet temperature (°C)
TOP55 | sdc/Ipm_Temp | Ipm temperature (°C)
TOP56 | sdc/Z1_Temp | Zone1: Actual Temperature (°C) 
TOP57 | sdc/Z2_Temp | Zone2: Actual Temperature (°C) 
TOP58 | sdc/DHW_Heater_State | When enabled, backup/booster heater can be used for DHW heating (disabled - enabled)
TOP59 | sdc/Room_Heater_State | When enabled, backup heater can be used for room heating (disabled - enabled)
TOP60 | sdc/Internal_Heater_State | Internal backup heater state (inactive - active)
TOP61 | sdc/External_Heater_State | External backup/booster heater state (inactive - active)
TOP62 | sdc/Fan1_Motor_Speed | Fan 1 Motor rotation speed (R/Min)
TOP63 | sdc/Fan2_Motor_Speed | Fan 2 Motor rotation speed (R/Min)
TOP64 | sdc/High_Pressure | High Pressure (Kgf/Cm2)
TOP65 | sdc/Pump_Speed | Pump Rotation Speed (R/Min)
TOP66 | sdc/Low_Pressure | Low Pressure (Kgf/Cm2)
TOP67 | sdc/Compressor_Current | Compressor/Outdoor unit Current (Ampere)
TOP68 | sdc/Force_Heater_State | Force heater status (0=inactive, 1=active)
TOP69 | sdC/Sterilization_State | Sterilisation State (0=inactive, 1=active)
TOP70 | sdC/Sterilization_Temp | Sterilisation Temperature (°C)
TOP71 | sdC/Sterilization_Max_Time | Sterilisation maximum time (minutes)
TOP72 | sdc/Z1_Cool_Curve_Target_High_Temp | Target temperature at lowest point on the cooling curve (°C)
TOP73 | sdc/Z1_Cool_Curve_Target_Low_Temp | Target temperature at highest point on the cooling curve (°C)
TOP74 | sdc/Z1_Cool_Curve_Outside_High_Temp | Lowest outside temperature on the cooling curve (°C)
TOP75 | sdc/Z1_Cool_Curve_Outside_Low_Temp | Highest outside temperature on the cooling curve (°C)
TOP76 | sdc/Heating_Mode | Compensation / Direct mode for heat (0 = compensation curve, 1 = direct)
TOP77 | sdc/Heating_Off_Outdoor_Temp | Above this outdoor temperature all heating is turned off(5 to 35 °C)
TOP78 | sdc/Heater_On_Outdoor_Temp | Below this temperature the backup heater is allowed to be used by heatpump heating logic(-15 to 20 °C)
TOP79 | sdc/Heat_To_Cool_Temp | Outdoor temperature to switch from heat to cool mode when in auto setting(°C)
TOP80 | sdc/Cool_To_Heat_Temp | Outdoor temperature to switch from cool to heat mode when in auto setting (°C)
TOP81 | sdc/Cooling_Mode | Compensation / Direct mode for cool (0 = compensation curve, 1 = direct)
TOP82 | sdc/Z2_Heat_Curve_Target_High_Temp | Target temperature at lowest point on the heating curve (°C)
TOP83 | sdc/Z2_Heat_Curve_Target_Low_Temp | Target temperature at highest point on the heating curve (°C)
TOP84 | sdc/Z2_Heat_Curve_Outside_High_Temp | Lowest outside temperature on the heating curve (°C)
TOP85 | sdc/Z2_Heat_Curve_Outside_Low_Temp | Highest outside temperature on the heating curve (°C)
TOP86 | sdc/Z2_Cool_Curve_Target_High_Temp | Target temperature at lowest point on the cooling curve (°C)
TOP87 | sdc/Z2_Cool_Curve_Target_Low_Temp | Target temperature at highest point on the cooling curve (°C)
TOP88 | sdc/Z2_Cool_Curve_Outside_High_Temp | Lowest outside temperature on the cooling curve (°C)
TOP89 | sdc/Z2_Cool_Curve_Outside_Low_Temp | Highest outside temperature on the cooling curve (°C)
TOP90 | sdc/Room_Heater_Operations_Hours | Electric heater operating time for Room (Hour)
TOP91 | sdc/DHW_Heater_Operations_Hours | Electric heater operating time for DHW (Hour)

All Topics realated with state can have also value -1 - unknown - but only in ubnormal situations.

## Command Topics:

 ID |Topic | Description | Value/Range
:--- | :--- | --- | ---
SET1  | SetHeatpump | Set heatpump on or off | 0=off, 1=on
SET2  | SetHolidayMode | Set holiday mode on or off | 0=off, 1=on
SET3  | SetQuietMode | Set quiet mode level | 0, 1, 2 or 3
SET4  | SetPowerfulMode | Set powerful mode run time in minutes | 0=off, 1=30, 2=60 or 3=90
SET5  | SetZ1HeatRequestTemperature | Set Z1 heat shift or direct heat temperature | -5 to 5 or 20 to max
SET6  | SetZ1CoolRequestTemperature | Set Z1 cool shift or direct cool temperature | -5 to 5 or 20 to max
SET7  | SetZ2HeatRequestTemperature | Set Z2 heat shift or direct heat temperature | -5 to 5 or 20 to max
SET8  | SetZ2CoolRequestTemperature | Set Z2 cool shift or direct cool temperature | -5 to 5 or 20 to max
SET9  | SetOperationMode | Sets operating mode | 0=Heat only, 1=Cool only, 2=Auto, 3=DHW only, 4=Heat+DHW, 5=Cool+DHW, 6=Auto+DHW
SET10 | SetForceDHW | Forces DHW (Operating mode should be firstly set to one with DWH mode (3,4,5 or 6) to be effective. Plese look at SET9 )  | 0, 1
SET11 | SetDHWTemp | Set DHW target temperature | 40 - 75
SET12 | SetForceDefrost | Forces defrost routine | 0, 1
SET13 | SetForceSterilization | Forces DHW sterilization routine | 0, 1



*If you operate your Heisha with direct temperature setup: topics ending xxxRequestTemperature will set the absolute target temperature*
