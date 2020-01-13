[![Jion us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H Series Aquarea air-water heat pump protocol

Eine deutschsprachige [README_DE.md](README_DE.md) findest du hier. \
Een nederlandse vertaling [README_NL.md](README_NL.md) vind je hier.

*Help on translation to other languages are welcome.*

# Current releases
Current beta release is version 0.2b. The [compiled binary](binaries/HeishaMon.ino.d1-v0.2b.bin) can be installed on a Wemos D1 mini. Or download the code compile it yourself (see required libraries below).


## Connection details:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (from top to bottom) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Where to get connectors
[RS-Online orders](Connectors_RSO.md)

[Conrad orders](Connectors_Conrad.md)

Use some 24 AWG shielded 4-conductors cable.


## How to connect
Currently the PCB's are in (beta) production. We suggest to wait a while for them to be tested. \
Soon you will be cable to order one of the PCB design directly from the project owners but ofcourse \
we will share the schematics also. For now these are some schematics we are testing or have tested. \
[PCD Designs from the project members](PCB_Designs.md) \
[Picture Wemos D1 beta](WEMOSD1.JPG)


## Using the test arduino image
The current arduino test image is able to read from the Panasonic Aquarea H-series CN-CNT connector. \
You need to build this image with support for a filesystem on the esp8266 so select the correct flash option in arduino ide for that. \
When starting for the first time a open-wifi-hotspot will be visible allowing you to config your wifi network and your mqtt server. \
If you ever want to factory reset, just double reset the esp8266 within 0.1 second. It will then format the filesystem and remove the wifi setting and start the wifi hotspot again. \
After configuring and booting the image will be able to read and talk to your heatpump. The GPIO13/GPIO15 connection will be used for communications so you can keep your computer/uploader connected to the board if you want. \
Serial 1 (GPIO2) can be used to connect another serial line (GND and TX from the board only) to read some debugging data.


## Building the test arduino image
boards: \
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[libs we use](LIBSUSED.md)


## MQTT topics
[Current list of documented MQTT topics can be found here](MQTT-Topics.md)


## Protocol info packet:
To get information from heat pump, "magic" packet should be send to CN-CNT:

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Protocol byte decrypt info:
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Integration Examples for Opensource automation systems
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)



