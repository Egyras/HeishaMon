Version 0.1.1 -> First version
Version 0.1.2 -> Added COP devices and Error (Text)

To use Plugins in Domoticz please read : https://www.domoticz.com/wiki/Using_Python_plugins

To update :

1. place the (new) mqtt.py and plugin.py in the HeishamonMQTT folder (overwrite the old-files)
2. In Hardware, select the HeishamonMQTT plugin and click on Update 
   !!! Do NOT click on delete, all the devices will also be removed then !!!
3. Add new devices when needed


To Install :

1. Create a folder [HeishamonMQTT] in the plugin folder of domoticz
2. Restart domoticz
3. Add in the Hardware the Heishamon MQTT plug in.
4. All devices are created, add them from the devices


Current status commands:

SetHeatpump			Set heatpump on or off	0=off, 1=on					tested	
SetHolidayMode			Set holiday mode on or off	0=off, 1=on				Not implemented (yet)	
SetQuietMode			Set quiet mode level	0, 1, 2 or 3					tested	
SetPowerfulMode			Set powerful mode run time in minutes	0=off, 1=30, 2=60 or 3=90	tested	
SetZ1HeatRequestTemperature	Set Z1 heat shift or direct heat temperature	-5 to 5 or 20 to max	tested	!! No check on value
SetZ1CoolRequestTemperature	Set Z1 cool shift or direct cool temperature	-5 to 5 or 20 to max	not tested	!! No check on value
SetZ2HeatRequestTemperature	Set Z2 heat shift or direct heat temperature	-5 to 5 or 20 to max	not tested	!! No check on value
SetZ2CoolRequestTemperature	Set Z2 cool shift or direct cool temperature	-5 to 5 or 20 to max	not tested	!! No check on value
SetOperationMode		Sets operating mode	0=Heat only, 1=Cool only, 2=Auto, 3=DHW only
				, 4=Heat+DHW, 5=Cool+DHW, 6=Auto+DHW					not tested	
SetForceDHW			Forces DHW (Operating mode should be firstly set to one with DWH mode (3,4,5 or 6) to be effective. Plese look at SET9 )	0, 1	not tested	
SetDHWTemp			Set DHW target temperature	40 - 75					not tested	!! No check on value
SetForceDefrost			Forces defrost routine	0, 1						not tested	
SetForceSterilization		Forces DHW sterilization routine	0, 1				not tested	


ToDo:
- DHW state is Unknown, this is not correct.
- Add SetHolidayMode
- Accept only values between -5/+5 and > 20 for Set...Temperature
- Round Temperature for whole values
- Check ForceDHW for correct operation mode

