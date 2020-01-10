[![Jion the Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H Series Aquarea lucht-water warmtepomp protocool

*Hulp bij het vertalen naar andere talen is zeer welkom.*


## Aansluit details:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (van onder naar boven) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Waar kun je connectoren kopen
[RS-Online orders](Connectors_RSO.md)

[Conrad orders](Connectors_Conrad.md)

Voeg tevens wat 24 AWG afgeschermde 4 aderige kabel toe (lengte zelf te bepalen).


## Hoe te bestellen
Momenteel zijn de PCB's in (beta) productie. wij adviseren om nog even te wachten met bestellen omdat de testen nog lopen. Binnenkort kun je een van de PCB ontwerpen bestellen rechtstreeks bij de project eigenaren. Maar natuurlijk delen wij hier de schema's ook. De volgende mogelijke oplossingen zijn of worden momenteel getest. \
[PCD Designs from the project members](PCB_Designs.md)


## Gebruik van het test arduino image
De huidige arduino test image is in staat om gegevens te lezen van de Panasonic Aquarea H-series CN-CNT connector. \
Deze image moet gebouwd worden met filesystem ondersteuning op de esp8266, dus selecteer de juiste flash optie in de arduino ide. \
Als de ESP voor de eerste keer start wordt er een open-wifi-hotspot zichtbaar waardoor het mogelijk is om de juiste wifi network en mqtt server settings in te stellen. \
Als je om wat voor reden ook terug wilt naar de startup instelling, een dubbele reset op de esp8266 binnen 0.1 seconde is voldoende. De ESP zal het filesysteem formatteren, de wifi settings verwijderen en de wifi hotspot opnieuw opstarten. \ 
Na het configurenen van de wifi en MQTT settings en een reboot is de wemos in staat om gegevens te lezen van en te zenden naar de warmtepomp. Deze communicatie vanuit de warmtepomp gaat naar de wemos op de in- en uitgang GPIO13/GPIO15. \
De standaard seriele in- en uitgang (tx/rx) van de wemos wordt alleen gebruikt voor het laden van de software en voor een aantal debug meldingen bij het opstarten.


## libs voor het bouwen van de test arduino image
boards: \
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[libs we use](LIBSUSED.md)


## MQTT topics
[Huidige lijst van gedocumenteerde MQTT onderwerpen kun je hier vinden](MQTT-Topics.md)


## Protocol info packet:
Om informatie te krijgen van de warmtepomp, moet het volgende "magic packet" verstuurd worden naar de CN-CNT: 

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Protocol byte decrypt info:
[Huidige lijst van gedocumenteerde bytes decrypted kan hier gevonden worden](ProtocolByteDecrypt.md)

## Integration Examples for Opensource automation systems
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)
