
This directory contains released and test versions for the HeishaMon software. 

The LittleFS versions will, after updating to this version, reset your HeishaMon to factory default due to change to a new filesystem for the configuration.

From version 1.0 some topics are changed so you need to update your automation for this.
The sensors are now in /main/ (before /sdc/) and the commands are expected in /commands/ (before in root topic). Check MQTT-Topics.md for the overview of all topics

The latest production release is v3.1. If you decide to try out a later development version, you should be able to restore the firmware using a USB-TTL cable as sometimes upgrading to a development versions seems to fail and brick the heishamon pcb.

The beta version 3.2-beta-1 and higher are to fix Listen Only mode (parallel mode with CZ-TAW) on K and L series heatpumps

