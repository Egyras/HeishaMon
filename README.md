[![Join us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H & J Series Aquarea air-water heat pump protocol

This project makes it possible to read information from Panasonic Aquarea heat pump and report the data either to an MQTT server or as JSON format over HTTP.

Eine deutschsprachige [README_DE.md](README_DE.md) findest du hier. \
Een nederlandse vertaling [README_NL.md](README_NL.md) vind je hier. \
Suomen kielellä [README_FI.md](README_FI.md) luettavissa täällä.

*Help on translation to other languages is welcome.*

# Current releases
Current release is version 2. The [compiled binary](binaries/HeishaMon.ino.d1-v2.0.bin) can be installed on a Wemos D1 mini, on the HeishaMon PCB and generally on any ESP8266 based board compatible with Wemos build settings (at least 4MB flash). You can also download the code and compile it yourself (see required libraries below).


# Using the software
HeishaMon is able to communicate with the Panasonic Aquarea H & J-series. [Confirmed by users types of HP you can find here](HeatPumpType.md) \
If you want to compile this image yourself be sure to use the mentioned libraries and support for a filesystem on the esp8266 so select the correct flash option in arduino ide for that.

When starting, without a configured wifi, an open-wifi-hotspot will be visible allowing you to configure your wifi network and your MQTT server. Configuration page will be located at http://192.168.4.1 . \

After configuring and booting the image will be able to read and talk to your heatpump. The GPIO13/GPIO15 connection will be used for communications so you can keep your computer/uploader connected to the board if you want. \
Serial 1 (GPIO2) can be used to connect another serial line (GND and TX from the board only) to read some debugging data.

All received data will be sent to different MQTT topics (see below for topic descriptions). There is also a 'panasonic_heat_pump/log' MQTT topic which provides debug logging and a hexdump of the received packets (if enabled in the web portal).

You can connect a 1wire network on GPIO4 which will report in seperate MQTT topics (panasonic_heat_pump/1wire/sensorid).

The software is also able to measure Watt on a S0 port of two kWh meters. You only need to connect GPIO12 and GND to the S0 of one kWh meter and if you need a second kWh meter use GPIO14 and GND. It will report on MQTT topic panasonic_heat_pump/s0/Watt/1 and panasonic_heat_pump/s0/Watt/2 and also in the JSON output. You can replace 'Watt' in the previous topic with 'Watthour' to get consumption counter in WattHour (per mqtt message) or to 'WatthourTotal' to get the total consumption measured in WattHour.

Updating the firmware is as easy as going to the firmware menu and, after authentication with username 'admin' and password 'heisha' (or other provided during setup), uploading the binary there.

A json output of all received data (heatpump and 1wire) is available at the url http://heishamon.local/json (replace heishamon.local with the ip address of your heishamon device if MDNS is not working for you).

Within the 'integrations' folder you can find examples how to connect your automation platform to the HeishaMon.


# Factory reset
A factory reset can be performed on the web interface but if the web interface is unavailable you can perform a double reset. The double reset should be performed not too fast but also not too slow. Usually halve a second between both resets should do the trick. To indicate that the double reset performed a factory reset, the blue led will flash rapidly (You need to press reset again now to restart HeishaMon back to normal where a WiFi hotspot should be visible again).

# Further information
Below you can find some technical details about the project. How to build your own cables. How to build your own PCB etc.

## Connection details:
Communication can be established thru one of the two sockets: CN-CNT or CN-NMODE. If you have an existing Panasonic CZ-TAW1 WiFi interface that you want to replace with HeishaMon, it is only a matter of plugging the cable out from CZ-TAW1 and reconnecting to your HeishaMon device. However it is not possible to use HeishaMon and the original CZ-TAW1 module together as an active device. It is however possible to put HeishaMon on "Listen Only" mode which will allow HeishaMon and the original CZ-TAW1 module to co-exist. The only downside to this is that HeishaMon is unable to send commands and use the optional PCB option.

Communication parameters: TTL 5V UART 9600,8,E,1  \
 \
CN-CNT Pin-out (from top to bottom) \
1 - +5V (250mA)  \
2 - 0-5V TX (from heatpump) \
3 - 0-5V RX (to heatpump)\
4 - +12V (250mA) \
5 - GND \
 \
CN-NMODE Pin-out (from left to right) \
"Warning! As printed on the PCB, the left pin is pin 4 and right pin is pin 1. Do not count 1 to 4 from left!  \
4 - +5V (250mA)  \
3 - 0-5V TX (from heatpump) \
2 - 0-5V RX (to heatpump) \
1 - GND

HeishaMon will receive power from the Panasonic over the cable (5v power).

## Long distance connection
It it possible to connect the HeishaMon over a long distance. Up to 5 meter is working with normal cabling. For longer distances a TTL-to-RS485 configuration as show in the picture below is possible. The however requires HeishaMon to be powered externally using 5v power (for example from an USB cable).

![TTL-over-RS485 HeishaMon long distance](optional-long-distance-heishamon.png)


## Where to get connectors
[RS-Online orders](Connectors_RSO.md)

[Conrad orders](Connectors_Conrad.md)

Use some 24 AWG shielded 4-conductors cable.


## The HeishaMon hardware itself
The PCB's needed to connect to the heatpump are designed by project members and are listed below. The most important part of the hardware is a level shifting between 5v from the Panasonic to 3.3v of the HeishaMon and a GPIO13/GPIO15 enable line after boot. \
[PCD Designs from the project members](PCB_Designs.md) \
[Picture Wemos D1 beta](WEMOSD1.JPG) \
[Picture ESP12-F](NewHeishamon.JPG)

To make things easy you can order a completed PCB from some project members: \
[Tindie shop](https://www.tindie.com/stores/thehognl/) from Igor Ybema (aka TheHogNL) based in the Netherlands

## Building the arduino image yourself
boards: \
esp8266 by esp8266 community version 3.0.2 [Arduino](https://github.com/esp8266/Arduino/releases/tag/3.0.2)

All the [libs we use](LIBSUSED.md) necessary for compiling.


## MQTT topics
[Current list of documented MQTT topics can be found here](MQTT-Topics.md)

## DS18b20 1-wire support
The software also supports ds18b20 1-wire temperature sensors reading. A proper 1-wire configuration (with 4.7kohm pull-up resistor) connected to GPIO4 will be read each configured secs (minimal 5) and send at the panasonic_heat_pump/1wire/"sensor-hex-address" topic. On the pre-made boards this 4.7kohm resistor is already installed.


## Protocol byte decrypt info:
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Integration Examples for Opensource automation systems
[Openhab2](Integrations/Openhab2)

[Home Assistant](Integrations/Home%20Assistant)

[IOBroker Manual](Integrations/ioBroker_manual)

[Domoticz](Integrations/Domoticz)


