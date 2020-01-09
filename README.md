[![Jion the Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H Series Aquarea air-water heat pump protocol

Eine deutschsprachige [README_DE.md](README_DE.md) findest du hier. 

Hilfe bei der Übersetzung auch für weitere Sprachen willkommen. \
Help on translation to other languages are welcome.



## Connection details:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (from top to bottom) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Where to get connectors

CN-CNT female connector PAP-05V-S - JST Female Connector Housing - PA, 2mm Pitch, 5 Way, 1 Row - https://uk.rs-online.com/web/p/pcb-connector-housings/4766798/ \
Pre-made crimp leads 150 mm crimp-free end lead PA 2.0 can be used - https://uk.rs-online.com/web/p/pre-crimped-leads/5128721/ \
The Panasonic CZ-TAW1 appears to use a JST B05B-XASK-1 male header - https://uk.rs-online.com/web/p/pcb-headers/6027679/

For Conrad orders:

| JST Type | Conrad article number | Description |
| ----- | ---- | ----- |
|B05B-XASK-1| 741366 | Header matching CZ-TAW1, for soldering on PCB. S05B-XASK-1 (not orderable at conrad) is the side-ways version	|
|B05B-PASK-1| 1426194 | Header matching CN-CNT (that what is on the heatpump itself). Only need this if you want to build a proxy-forward PCB|
|XAP-05V-1| 741233 | Cable connector on CZ-TAW1/PCB side |
|PAP-05V-1| 1426227 | Cable connector on CN-CNT side | 
|BXA-01T-P0.6| 741295 |Connector pins on CZ-TAW1/PCB side|
|SPHD-002T-P0.5| 1426240 | Connector pins on CN-CNT side |

And add some 24 AWG shielded 4-conductors cable.

## How to connect

Currently the PCB's are in (beta) production. We suggest to wait a while for them to be tested. Soon you will be cable to order one of the PCB design directly from the project owners but ofcourse we will share the schematics also. For now these are some schematics we are testing or have tested.

PCB design based on through hole soldering, Wemos shield type. With basic components (resitors and transistors). https://easyeda.com/igor_6537/panasonic-cn-cnt-to-wemos-through-hole

PCB design based on SMD soldering, Wemos shield type. With SMD components (mosfet instead of transistors) https://easyeda.com/igor_6537/panasonic-cn-cnt-to-wemos

PCB design based on SMD soldering, ESP-12f type.  https://easyeda.com/kompiuteriu/new-cn-cnt

## using the test arduino image
The current arduino test image is able to read from the Panasonic Aquarea H-series CN-CNT connector. \
You need to build this image with support for a filesystem on the esp8266 so select the correct flash option in arduino ide for that. \
When starting for the first time a open-wifi-hotspot will be visible allowing you to config your wifi network and your mqtt server. \
If you ever want to factory reset, just double reset the esp8266 within 0.1 second. It will then format the filesystem and remove the wifi setting and start the wifi hotspot again. \
After configuring and booting the image will be able to read and talk to your heatpump. The GPIO13/GPIO15 connection will be used for communications so you can keep your computer/uploader connected to the board if you want. \
Serial 1 (GPIO2) can be used to connect another serial line (GND and TX from the board only) to read some debugging data.

## libs for building the test arduino image
boards: \
esp8266 by esp8266 community version 2.6.3 https://github.com/esp8266/Arduino/releases/tag/2.6.3

libs: \
wifimanager by tzapu version 0.15.0-beta https://github.com/tzapu/WiFiManager/releases/tag/0.15.0-beta \
pubsubclient by nick o'leary version 2.7.0 https://github.com/knolleary/pubsubclient/releases/tag/v2.7 \
doubleresetdetect by jens-christian skibakk version 1.0.0 https://github.com/jenscski/DoubleResetDetect/releases/tag/1.0.0 \
arduinojson by benoit blanchon version 6.13.0 https://github.com/bblanchon/ArduinoJson/releases/tag/v6.13.0

## MQTT topics
[Current list of documented MQTT topics can be found here](MQTT-Topics.md)

## Integration Examples for Opensource automation systems
[Openhab2](Integrations/Openhab2)
[Home Assistant](Integrations/Home Assistant/)


## Protocol info packet:

To get information from a heat pump, "magic" packet should be send to CN-CNT: 

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`

## Protocol byte decrypt info:

[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)

