[![Jion us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)

# Panasonic H Series Aquarea air-water Wärmepumpen Protokoll

*Hilfe bei der Übersetzung in weitere Sprachen ist willkommen.*

# Aktuelle Version
Die aktuelle Version findest du hier: [README.md](README.md)

# Nutzung der aktuellen Software
Die aktuelle Arduino Software liest die Daten der CN-CNT Schnittstelle der Panasonic Aquarea H-series Geräte. \
Beim Build des Images must du darauf achten, dass du die Optionen für die Nutzung des Dateisystems auf dem ESP8266 für das Flashen in der IDE auswählst.

Nach dem ersten Start wird ein offener WiFi-Hotspot angeboten. Verbinde dich mit diesem Hotspot und konfiguriere dein eigenes Netzwerk und die Daten (IP, Login) deines MQTT-Servers. \
Wenn du dein Board auf die Werkseinstellungen zurücksetzen möchtest, drücke bitte innerhalb von 0,1 Sekunden den Reset Taster zwei mal. Dabei wird das lokale Filesystem formatiert und die Konfiguration für das WiFi Netz gelöscht. Danach startet das Gerät neu und du kannst es wie nach dem ersten Start über den offenen Hotspot neu konfigurieren. \
Nach der Konfiguration und Neustart beginnt die Kommunikation mit deiner Wärmepumpe. GPIO13/GPIO15 werden für die serielle Verbindung benutzt, die USB Schnittstelle bleibt frei. \
Serial 1 (GPIO2) ist ein serialer Port und kann genutzt werden, um Debug Meldungen auszulesen. (GND und TX vom Board)

Alle von der Wärmepumpe empfangenen Daten werden an MQTT Topics gesendet. Die Auflistung aller verwendeten Topics findest du weiter unten. Zusätzlich werden in dem Topic 'panasonic_heat_pump/log' logging Daten und ein Hexdump der empfangenen Daten bereitgestellt. Diese Funktion kann auf dem Webportal von HeishaMon aktiviert werden.

Du kannst darüber hinaus an GPIO4 1-wire Temperatursensoren anschließen. Die Messwerte der Temperatursensoren werden an die Topics 'panasonic_heat_pump/1wire/sensorid' gesendet.

Ein Firmware Update ist sehr einfach über das Firmware Menü mit deinem Browser möglich. Nach der Anmeldung mit dem Benutzernamen 'admin' und dem von dir beim ersten Setup vergebenen Passwort kannst du die aktuelle Firmwaredatei auf das Gerät laden.

Alle Daten kannst du auch unter http://heishamon.local/json als json Datei abrufen. Sollte bei dir MDNS nicht funktionieren, ersetze bitte heishamon.local durch die IP deines Gerätes.

Im Bereich 'integrations' findest du Beispiele zur Integration von HeishMon in dein Hausautomatisierungs System.

# Weitere Information
Hier findest du technische Informationen zum Projekt um dir Kabel und Platine selbst anzufertigen.

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
Aktuell sind mehrere Platinen in der Produktion (beta). Wir empfehlen, noch unsere Tests abzuwarten.
Es wird bald möglich sein, Platinen von den Projektmitgliedern zu erhalten. Die Layouts der Platinen und die Schaltpläne werden veröffentlicht.
Hier findest du die Platinen, die im Moment getestet werden. \
[PCD Designs  from the project members](PCB_Designs.md) \
[Picture Wemos D1 beta](WEMOSD1.JPG) \
[Picture ESP12-F](New_PCB.jpeg)



## Build und Test Arduino Image
boards: \
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[libs we use](LIBSUSED.md)


## MQTT topics
[Current list of documented MQTT topics can be found here](MQTT-Topics.md)

## DS18b20 1-wire Temperatursensor
Die Software ünterstützt DS18B20 1-wire Temperatur Sensoren. Der 1-wire Anschluß erfolgt an GPIO4 mit einem 4.7kohm pull-up Widerstand. Die Daten werden alle 30 Sekunden an den mqtt Server mit dem Topic panasonic_heat_pump/1wire/"sensor-hex-address" gesendet.


## Protokoll Info Packet:
Um Daten von der Wärmepumpe zu erhalten wird dieses "magic" Packet an die CN-CNT Schnittstelle gesendet:

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Protokoll Byte Decrypt Info:
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Beispiele für Opensource Automatisierungssysteme
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)

[IOBroker Anleitung](Integrations/ioBroker_manual)

[Domoticz](Integrations/Domoticz)
