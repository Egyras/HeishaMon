[![Jion the Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic H Series Aquarea lucht-water warmtepomp protocool

Eine deutschsprachige [README_DE.md](README_DE.md) findest du hier.
Een Nederlandse [README_NL.md] (README_NL.md) kun je hier vinden

Hulp bij het vertalen naar andere talen is zeer welkom.
Help on translation to other languages are welcome.

##Huidige releases

De huidige beta release versie is 0.5b. Het gecompileerde Bin file kan worden geinstalleerd op een WemosD1 mini (over het algemeen op ieder ESP8266 gebaseerd unit.). Je kunt ook de code zelf compileren (zie hieronder welke bibliotheken nodig zijn).

## gebruik van de software

De huidige arduino test image is in staat om gegevens te lezen van de Panasonic Aquarea H-series CN-CNT connector. 
Als je de image zelf wil compileren gebruik dan in ieder geval de genoemde libraries met filesysteem ondersteuning op de esp8266, dus selecteer de juiste flash optie in de arduino ide. 
Als de ESP voor de eerste keer start wordt er een open-wifi-hotspot zichtbaar waardoor het mogelijk is om de juiste wifi network en mqtt server settings in te stellen. 
Als je om wat voor reden ook terug wilt naar de startup instelling, een dubbele reset op de esp8266 binnen 0.1 seconde is voldoende. De ESP zal het filesysteem formatteren, de wifi settings verwijderen en de wifi hotspot opnieuw opstarten. 
Na het configureren en booten  is de wemos in staat om gegevens te lezen van en te zenden naar de warmtepomp. Deze communicatie vanuit de warmtepomp gaat naar de wemos op de in- en uitgang GPIO13/GPIO15. Je computer/uploader kan dus aangesloten blijven.
Serial 1 kan worden gebruikt voor een andere seriele aansluiting ( alleenGND en TX van de ESP )om de debugging data te lezen.

Alle ontvangen data wordt naar verschillende MQTT topics verstuurd. (zie de MQTT topics hoofdstuk voor verdere details). Er is ook een  panasonic_heat_pump/log' mqtt topic die debug logging verschaft en een hexdump van de ontvangen pakketten levert. (moet in het webportaal aangezet worden)

Ook is het mogelijk om een 1 wire netwerk aan te sluiten op GPIO4 welke dan in een separate mqtt topic de gegevens verzend. ( panasonic_heat_pump/1wire/sensorid). Zie voor meer informatie het ds18b20 1-wire support hoofdstuk.

Het updaten van de software is simpel Ga naar het firmware menu en na ingave van username 'admin' en het wachtwoord wat tijdens de setup is opgegeven en je kunt dan de juiste binaire files uploaden.

Een json output van alle ontvangen data ( warmtepomp en 1-wire) is beschikbaar op de url http://heishamon.local/json (vervang heishamon.local met het ip adres van jou heishamon apparaat als MDNS niet werkt voor jou).

In de integratie folder kun je voorbeelden vinden hoe je jou automatiserings platform aan de heishamon.

## Verdere informatie
Hieronder kun je technische informatie vinden over het project. Hoe je eigen kabels te maken. Hoe je eigen PCB's te maken enz. 

## Aansluit details:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (van onder naar boven) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Waar kun je connectoren kopen
RS-Online orders
Conrad orders
Gebruik  24 AWG afgeschermde 4 aderige kabel.

Voeg tevens wat 24 AWG afgeschermde 4 aderige kabel toe (lengte zelf te bepalen).

## Hoe aan te sluiten

Momenteel zijn de PCB's in (beta) productie. wij adviseren om nog even te wachten met bestellen omdat de testen nog lopen. Binnenkort kun je een van de PCB ontwerpen bestellen rechtstreeks bij de project eigenaren. Maar natuurlijk delen wij hier de schema's ook. De volgende mogelijke oplossingen zijn of worden momenteel getest.

PCD Designs from the project members 
Picture Wemos D1 beta
Picture ESP12-F



## Het bouwen van de test arduino image
boards: \
esp8266 by esp8266 community version 2.6.3  Arduino  
libs we use


## MQTT topics
[Huidige lijst van gedocumenteerde MQTT onderwerpen kun je hier vinden]

## DS18b20 1-wire support
De software ondersteund ook ds18b20 1-wire temperatuur sensor uitlezing. Een standaard 1-wire configuratie (met 4.7kohm pull-up weerstand) aangesloten op GPIO4 leest iedere 30 secondende waarden en stuurt die naar de panasonic_heat_pump/1wire/"sensor-hex-address" topic.

## Protocol info packet:

Om informatie te krijgen van de warmtepomp, moet het volgende "magic packet" verstuurd worden naar de CN-CNT: 

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`

## Protocol byte decrypt info:

[Huidige lijst van gedocumenteerde bytes decrypted kan hier gevonden worden]
## Integration Examples for Opensource automation systems

[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)

[IOBroker Manual](Integrations/ioBroker_manual)

[Domoticz](Integrations/Domoticz)



