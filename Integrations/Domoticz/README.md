# Heishamon in Domoticz

There are two options to integrate HeishaMon into Domoticz. You can choose for the domoticz plugin or a nodered flow. The domoticz plugin is plug-and-play but the nodered flow will allow you to more fine tune your system to your needs.


## Plugin option

A domoticz plugin (https://github.com/MarFanNL/HeishamonMQTT) is available which, once installed, will create the necessary device and will talk to mqtt for you.


## Nodered option

Prerequisite:
- Domoticz with mqtt setup
- node-red with mosquitto (optionally with https://flows.nodered.org/node/node-red-contrib-influxdb installed)
- Working PCB with Heishamon firmware installed


Installation instructions:

In Influx:
- create a database called "Panasonic" (go to SSH console -> typ "influx" -> typ "create database Panasonic" -> typ "show databases" and see if Panasonic is listed)

In Node-red:
- import the 'json' file 
- adjust the IDX values in the Node-Red function 'global setup' to the IDX values of your install. 
- note: you have to adjust values under "SensorMapping" (values from Panasonic to Domoticz) and "ActionMapping" (Domoticz commands to Panasonic). These IDX values can be the same. 
- If nodered is on other server then the influx or MQTT server then change these IP addresses in the outgoing and incoming nodes.
- If you are not using influx out open the "influx out" node and disable it (bottom of the screen)

For example create these devices in Domoticz:
- Create the following dummy sensors in Domoticz and use the IDX values in the corresponding lines in the setup node.
  Temperature:
  - Outlet
  - Inlet
  - Outdoor
  Percentage:
  - Pumpspeed 
  - Compressor frequency
  Counter:
  - startstops 
  - working hours
  
- Create the following switches:
  - On Off switch (switch heatpump on/off)
  - Selector switch (to set the silent mode: Off, silent 1, Silent 2, Silent 3)
  - Thermostat Setpoint (to set the heatpump setpoint or temperature shift)

