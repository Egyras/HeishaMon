[![Join us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)


# Panasonic Aquarea H-sarjan vesi-ilmalämpöpumpun protokollaa lukeva IoT-laite

Tämä ESP8266-projekti mahdollistaa Panasonic Aquarea vesi-ilmalämpöpumppujen toimittamien tietojen raportoimisen MQTT-palvelimelle. Se tarjoaa myös JSON-muotoisen tiedon, jota voit pollata, jos et halua käyttää MQTT:tä.

Ajantasainen, englanninkielinen [README.md](README.md) löytyy täältä.

# Julkaistu versio

Tämän hetkinen beta-versio on 0.6b. [Käännetty binääri](binaries/HeishaMon.ino.d1-v0.6.bin) voidaan asentaa Wemos D1 minille tai luultavasti mille tahansa muulle ESP8266-pohjaiselle kehitysalustalle (ei takuita toimivuudesta). Voit myös ladata lähdekoodin ja kääntää omat binäärit (katso tarvittavat kirjastot alta).

# Käyttö

Nykyinen Arduino-softa pystyy kommunikoimaan Panasonicin Aquarea H-sarjan (luultavasti myös J-sarjan, sillä piirikortit näyttää identtiseltä). \
Jos haluat kääntää oman softan, niin lataa kaikki tarvittavat kirjastot ja muista myös filesystem-tuki ESP8266:lle Arduino IDE:ssä.

Kun ensimmäisen kerran kytket virrat laitteeseen, kytkeydy avoimeen WiFi-hotspotiin, jota käytetään ensimmäisen käynnistyksen yhteydessä asetusten tekemiseen. Asetussivu löytyy osoitteesta http://192.168.4.1 ja sieltä voit määrittää WiFi-verkon ja MQTT-palvelimen asetukset.

Laitteen voi palauttaa tehdasasetuksille painamalla kaksi kertaa reset-nappia nopeasti. Silloin tiedostojärjestelmä tyhjennetään ja WiFi-asetukset palautetaan ennalleen ja voit taas liittyä avoimeen WiFi-verkkoon asetusten tekemistä varten.

Kun olet tehnyt asetukset, laite käynnistyy uudestaan ja liittyy määrittelemääsi WiFi-verkkoon. Se alkaa kyselemään lämpöpumpulta tietoja säännöllisesti ja raportoi tiedot MQTT-palvelimelle. GPIO13/GPIO15-pinnejä käytetään tiedonsiirtoon ja voit pitää tietokoneesi kytkettynä USB-portissa, jos haluat lukea debug-tietoja tai päivittää firmwarea.

Kaikki vastaanotettu tieto raportoidaan eri MQTT-aiheiden alle (katso koko lista alempaa). Web UI:n kautta voi myös kytkeä päälle lokituksen ja viestien hexadumppauksen, jolloin nämä tiedot raportoidaan aiheen 'panasonic_heat_pump/log' alle.

Voit myös liittää 1-wire sensorin GPIO4-liitäntään ja sen arvo raporoidaan omassa MQTT-aiheessaan (panasonic_heat_pump/1wire/sensorid).

HeishaMon voi myös lukea kahden kWh-mittarin S0-portista sähkönkulutusta. Kytke GPIO12 ja GND kWh-mittarin S0-liitäntään ja jos tarvitset vielä toisen liitännän käytä GPIO14:ta siihen. Kulutus raportoidaan MQTT-aiheiden 'panasonic_heat_pump/s0/Watt/1' ja 'panasonic_heat_pump/s0/Watt/2' alla sekä JSON-tulosteessa. Hetkellisen tehon sijasta näet kulutuksen (kWh), jos vaihdat topiciin 'Watt'in sijasta 'Watthour'.

Firmwaren voi päivittää laitteen konfigurointisivun kautta kirjautumalla tunnuksella 'admin' ja valitsemallasi salasanalla (oletus on heisha).

JSON-muodossa lämpöpumpun tilan voi hakea osoitteesta http://heishamon.local/json (korvaa heishamon.local laitteesi IP-osoitteella, jos MDNS ei toimi verkossasi).

'integrations' hakemistosta löydät esimerkkejä laitteen integroimisesta erilaisiin kotiautomaatiojärjestelmiin.

# Lisätietoa

Alla vielä hieman teknisiä lisätietoja projektista, esimerkiksi kaapelinrakennusohjeet ja apua piirilevyn suunnitteluun.

## Yhteys lämpöpumppuun

Yhteys voidaan muodostaa CN-CNT tai CN-NMODE liitäntöjen kautta. Molempia ei voida käyttää samanaikaisesti. Huomaa, että jos sinulla on Panasonicin CZ-TAW1 WiFi-sovitin käytössäsi, voit vain kytkeä HeishaMon-laitteen sen sijasta käyttäen samaa kaapelia.

Yhteysasetukset: TTL 5V UART 9600,8,E,1 \
\
CN-CNT Pin-out (ylhäältä alas) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5V RX  \
4 - +12V (250mA) \
5 - GND \
 \
CN-NMODE Pin-out (vasemmalta oikealle) \
"Huom! Kuten piirilevyllä sanotaan, vasen pinni on numero 4 ja oikealla reunassa on pinni 1. \
4 - +5V (250mA)  \
3 - 0-5V TX  \
2 - 0-5V RX  \
1 - GND

## Mistä ostaa liittimiä 
[RS-Online orders](Connectors_RSO.md)

[Conrad orders](Connectors_Conrad.md)

Käytä 24 AWG suojattua 4-johtimista kaapelia.

## Piirilevyn valmistaminen 
The PCB's needed to connect to the heatpump are designed by project members and are listed below. \
[PCD Designs from the project members](PCB_Designs.md) \
[Picture Wemos D1 beta](WEMOSD1.JPG) \
[Picture ESP12-F](NewHeishamon.JPG)

Jos haluat päästä helpolla, voit myös ostaa valmiin piirilevyn tai kaapelin, joita projektin jäsenet ovat tehneet: \
[Tindie shop](https://www.tindie.com/stores/thehognl/) - Igor Ybema (aka TheHogNL) Hollannissa \
[Trab.dk shop](https://www.trab.dk/en/search?controller=search&orderby=position&orderway=desc&search_query=panasonic&submit_search=) - Morten Trab Tanskassa


## Arduino-imagen kääntäminen 
boards: \
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[Käytetyt kirjastot](LIBSUSED.md)


## MQTT-aiheet 
[Lista tämän hetkisistä MQTT-aiheista](MQTT-Topics.md)

## DS18b20 1-wire tuki

Softa tukee myös DS18B20 1-wire lämpötilasensoreita. Kunnollinen 1-wire sensori (sis. 4.7 kOhm pull-up vastuksen) liitettynä GPIO4-liitäntään luetaan joka 30 sekunnin välein ja tieto palautetaan 'panasonic_heat_pump/1wire/"sensor-hex-address"' aiheessa.
 
## Protocol info -paketti:
Saadaksesi tietoa pumpulta, täytyy sille lähettää "magic" paketti:

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Viestin formaatti: 
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Integraatio-esimerkkejä Open Source -kotiautomaatioon
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)

[IOBroker Manual](Integrations/ioBroker_manual)

[Domoticz](Integrations/Domoticz)

