[![Join us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)

# Panasonic H Series Aquarea air-water Wärmepumpen Protokoll

*Hilfe bei der Übersetzung in weitere Sprachen ist willkommen.*

# Aktuelle Version
Die aktuelle Version findest du hier: [README.md](README.md)

# Nutzung der aktuellen Software
Die aktuelle Arduino Software liest die Daten der CN-CNT Schnittstelle der Panasonic Aquarea H-Series Geräte. \
Beim Build des Images musst du darauf achten, dass du die Optionen für die Nutzung des Dateisystems auf dem ESP8266 für das Flashen in der IDE auswählst.

Nach dem ersten Start wird ein offener WiFi-Hotspot angeboten. Verbinde dich mit diesem Hotspot und konfiguriere dein eigenes Netzwerk sowie die Daten (IP, Login) deines MQTT-Servers. \
Wenn du dein Board auf die Werkseinstellungen zurücksetzen möchtest, drücke bitte innerhalb von 0,1 Sekunden den Reset-Taster zweimal. Dabei wird das lokale Dateisystem formatiert und die Konfiguration für das WiFi-Netz gelöscht. Danach startet das Gerät neu und du kannst es wie nach dem ersten Start über den offenen Hotspot neu konfigurieren. \
Nach der Konfiguration und dem Neustart beginnt die Kommunikation mit deiner Wärmepumpe. GPIO13/GPIO15 werden für die serielle Verbindung benutzt, die USB-Schnittstelle bleibt frei. \
Serial 1 (GPIO2) ist ein serieller Port und kann genutzt werden, um Debug-Meldungen auszulesen. (GND und TX vom Board)

Alle von der Wärmepumpe empfangenen Daten werden an MQTT-Topics gesendet. Die Auflistung aller verwendeten Topics findest du weiter unten. Zusätzlich werden in dem Topic 'panasonic_heat_pump/log' Logging-Daten und ein Hexdump der empfangenen Daten bereitgestellt. Diese Funktion kann auf dem Webportal von HeishaMon aktiviert werden.

Du kannst darüber hinaus an GPIO4 1-Wire-Temperatursensoren anschließen. Die Messwerte der Temperatursensoren werden an die Topics 'panasonic_heat_pump/1wire/sensorid' gesendet.

Ein Firmware-Update ist sehr einfach über das Firmware-Menü mit deinem Browser möglich. Nach der Anmeldung mit dem Benutzernamen 'admin' und dem von dir beim ersten Setup vergebenen Passwort kannst du die aktuelle Firmware-Datei auf das Gerät laden.

Alle Daten kannst du auch unter http://heishamon.local/json als JSON-Datei abrufen. Sollte bei dir mDNS nicht funktionieren, ersetze bitte heishamon.local durch die IP deines Gerätes.

Im Bereich 'Integrations' findest du Beispiele zur Integration von HeishaMon in dein Hausautomatisierungssystem.

# Weitere Information
Hier findest du technische Informationen zum Projekt, um dir Kabel und Platine selbst anzufertigen.

## Verbindungsdetails:
CN-CNT TTL UART 9600,8,E,1  \
Pin-out (from top to bottom) \
1 - +5V (250mA)  \
2 - 0-5V TX  \
3 - 0-5 RX  \
4 - +12V (250mA) \
5 - GND

## Stecker und Buchsen
[RS-Online orders](Connectors_RSO.md)

[Conrad orders](Connectors_Conrad.md)

Zusätzlich ist ein geschirmtes 4-adriges 24 AWG Kabel erforderlich. 


## Verbindung herstellen
Aktuell sind mehrere Platinen in der Produktion (Beta). Wir empfehlen, noch unsere Tests abzuwarten.
Es wird bald möglich sein, Platinen von den Projektmitgliedern zu erhalten. Die Layouts der Platinen und die Schaltpläne werden veröffentlicht.
Hier findest du die Platinen, die im Moment getestet werden:
[PCB-Designs der Projektmitglieder](PCB_Designs.md) \
[Bild Wemos D1 Beta](WEMOSD1.JPG) \
[Bild ESP12-F](New_PCB.jpeg)



## Build und Test des Arduino-Images
Boards:
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[Von uns verwendete Bibliotheken](LIBSUSED.md)


## MQTT-Topics
[Aktuelle Liste der dokumentierten MQTT-Topics findest du hier](MQTT-Topics.md)

## DS18B20 1-Wire-Temperatursensor
Die Software unterstützt DS18B20 1-Wire-Temperatursensoren. Der 1-Wire-Anschluss erfolgt an GPIO4 mit einem 4,7kΩ Pull-up-Widerstand. Die Daten werden alle 30 Sekunden an den MQTT-Server mit dem Topic panasonic_heat_pump/1wire/"sensor-hex-address" gesendet.


## Protokoll-Info-Packet:
Um Daten von der Wärmepumpe zu erhalten, wird dieses "Magic"-Packet an die CN-CNT-Schnittstelle gesendet:

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Protokoll-Byte-Decrypt-Info:
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Beispiele für Open-Source-Automatisierungssysteme
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)

[ioBroker-Anleitung](Integrations/ioBroker_manual)

[Domoticz](Integrations/Domoticz)
