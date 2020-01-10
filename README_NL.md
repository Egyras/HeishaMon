[![Jion the Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H Series Aquarea lucht-water warmtepomp protocool

Eine deutschsprachige [README_DE.md](README_DE.md) findest du hier. \
Een nederlandse vertaling [README_NL.md](README_NL.md) kun je hier vinden

Hilfe bei der Übersetzung auch für weitere Sprachen willkommen. \
Hulp bij het vertalen naar andere talen is zeer welkom. \
Help on translation to other languages are welcome.



## Aansluit details:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (van onder naar boven) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Waar kun je connectoren kopen

CN-CNT female connector PAP-05V-S - JST Female Connector Housing - PA, 2mm Pitch, 5 Way, 1 Row - https://uk.rs-online.com/web/p/pcb-connector-housings/4766798/ \
Pre-made crimp leads 150 mm crimp-free end lead PA 2.0 can be used - https://uk.rs-online.com/web/p/pre-crimped-leads/5128721/ \
De Panasonic CZ-TAW1 lijkt de JST B05B-XASK-1 male header te gebruiken - https://uk.rs-online.com/web/p/pcb-headers/6027679/

De connectoren zijn ook te bestellen bij Conrad:

| JST Type | Conrad article number | Description |
| ----- | ---- | ----- |
|B05B-XASK-1| 741366 | Header behorende bij CZ-TAW1, voor het solderen op de PCB. S05B-XASK-1 (niet verkrijgbaar bij Conrad) is de zijwaartse versie	|
|B05B-PASK-1| 1426194 | Header matching CN-CNT (dat zit op de warmtepomp print). Ia alleen nodig als je een proxy-forward PCB wilt maken|
|XAP-05V-1| 741233 | Kabel connector op de CZ-TAW1/PCB zijde |
|PAP-05V-1| 1426227 | Kabel connector op de CN-CNT zijde | 
|BXA-01T-P0.6| 741295 |Aansluit pennen op de CZ-TAW1/PCB zijde|
|SPHD-002T-P0.5| 1426240 | Aansluit pennen op de CN-CNT zijde |

Voeg tevens wat 24 AWG afgeschermde 4 aderige kabel toe (lengte zelf te bepalen).

## Hoe te bestellen

Momenteel zijn de PCB's in (beta) productie. wij adviseren om nog even te wachten met bestellen omdat de testen nog lopen. Binnenkort kun je een van de PCB ontwerpen bestellen rechtstreeks bij de project eigenaren. Maar natuurlijk delen wij hier de schema's ook. De volgende mogelijke oplossingen zijn of worden momenteel getest.

PCB ontwerp gebaseerd op draad componenten, Wemos shield type. Met basis componenten (weerstanden en transistoren). https://easyeda.com/igor_6537/panasonic-cn-cnt-to-wemos-through-hole

PCB ontwerp gebaseerd op SMD componenten, Wemos shield type. Met SMD componenten (mosfet in plaats van transistoren) https://easyeda.com/igor_6537/panasonic-cn-cnt-to-wemos

PCB ontwerp gebaseer op SMD componenten, ESP-12f type.  https://easyeda.com/kompiuteriu/new-cn-cnt

## gebruik van het test arduino image
De huidige arduino test image is in staat om gegevens te lezen van de Panasonic Aquarea H-series CN-CNT connector. \
Deze image moet gebouwd worden met filesystem ondersteuning op de esp8266, dus selecteer de juiste flash optie in de arduino ide. \
Als de ESP voor de eerste keer start wordt er een open-wifi-hotspot zichtbaar waardoor het mogelijk is om de juiste wifi network en mqtt server settings in te stellen. \
Als je om wat voor reden ook terug wilt naar de startup instelling, een dubbele reset op de esp8266 binnen 0.1 seconde is voldoende. De ESP zal het filesysteem formatteren, de wifi settings verwijderen en de wifi hotspot opnieuw opstarten. \ 
Na het configurenen van de wifi en MQTT settings en een reboot is de wemos in staat om gegevens te lezen van en te zenden naar de warmtepomp. Deze communicatie vanuit de warmtepomp gaat naar de wemos op de in- en uitgang GPIO13/GPIO15. \
De standaard seriele in- en uitgang (tx/rx) van de wemos wordt alleen gebruikt voor het laden van de software en voor een aantal debug meldingen bij het opstarten.

## libs voor het bouwen van de test arduino image
boards: \
esp8266 by esp8266 community version 2.6.3 https://github.com/esp8266/Arduino/releases/tag/2.6.3

libs: \
wifimanager by tzapu version 0.15.0-beta https://github.com/tzapu/WiFiManager/releases/tag/0.15.0-beta \
pubsubclient by nick o'leary version 2.7.0 https://github.com/knolleary/pubsubclient/releases/tag/v2.7 \
doubleresetdetect by jens-christian skibakk version 1.0.0 https://github.com/jenscski/DoubleResetDetect/releases/tag/1.0.0 \
arduinojson by benoit blanchon version 6.13.0 https://github.com/bblanchon/ArduinoJson/releases/tag/v6.13.0

## MQTT topics
[Huidige lijst van gedocumenteerde MQTT onderwerpen kun je hier vinden](MQTT-Topics.md)

## Integration Examples for Opensource automation systems
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)


## Protocol info packet:

Om informatie te krijgen van de warmtepomp, moet het volgende "magic packet" verstuurd worden naar de CN-CNT: 

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`

## Protocol byte decrypt info:

[Huidige lijst van gedocumenteerde bytes decrypted kan hier gevonden worden](ProtocolByteDecrypt.md)

