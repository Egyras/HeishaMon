[![Jion us on Slack chat room](https://img.shields.io/badge/Slack-Join%20the%20chat%20room-orange)](https://join.slack.com/t/panasonic-wemos/shared_invite/enQtODg2MDY0NjE1OTI3LTgzYjkwMzIwNTAwZTMyYzgwNDQ1Y2QxYjkwODg3NjMyN2MyM2ViMDM3Yjc3OGE3MGRiY2FkYzI4MzZiZDVkNGE)

# Panasonic H Series Aquarea air-water Wärmepumpen Protokoll

*Hilfe bei der Übersetzung in weitere Sprachen ist willkommen.*


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
[PCD Designs  from the project members](PCB_Designs.md)


## Nutzung der aktuellen Software
Die aktuelle Arduino Software liest die Daten der CN-CNT Schnittstelle der Panasonic Aquarea H-series Geräte. \
Beim Build des Images muss darauf geachtet werden, dass die Optionen für die Nutzung des Dateisystems auf dem ESP8266 für das Flashen in der IDE gewählt werden. \
Nach dem ersten Start wird ein offener WiFi-Hotspot angeboten. Verbinde dich mit diesem Hotspot und konfiguriere dein eigenes Netzwerk und die IP deines MQTT-Servers. \
Wenn du dein Board auf die Werkseinstellungen zurücksetzen möchtest, drücke bitte innerhalb von 0,1 Sekunden den Reset Taster zwei mal. Dabei wird das lokale Filesystem formatiert und die Konfiguration für das WiFi Netz gelöscht. Danach startet das Gerät neu und du kannst es wie nach dem ersten Start über den offenen Hotspot neu konfigurieren. \
Nach der Konfiguration und Neustart beginnt die Kommunikation mit deiner Wärmepumpe. GPIO13/GPIO15 werden für die serielle Verbindung benutzt, die USB Schnittstelle bleibt frei. \
Serial 1 (GPIO2) ist ein serialer Port und kann genutzt werden, um Debug Meldungen zu erhalten. (GND und TX vom Board)


## Build und Test Arduino Image
boards: \
esp8266 by esp8266 community version 2.6.3 [Arduino](https://github.com/esp8266/Arduino/releases/tag/2.6.3)

[libs we use](LIBSUSED.md)


## MQTT topics
[Current list of documented MQTT topics can be found here](MQTT-Topics.md)


## Protokoll Info Packet:
Um Daten von der Wärmepumpe zu erhalten muß dieses "magic" Packet an die CN-CNT Schnittstelle gesendet werden:

`71 6c 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`


## Protokoll Byte Decrypt Info:
[Current list of documented bytes decrypted can be found here](ProtocolByteDecrypt.md)


## Beispiele für Opensource Automatisierungssysteme
[Openhab2](Integrations/Openhab2)

[Home Assistant](https://github.com/Egyras/HeishaMon/tree/master/Integrations/Home%20Assistant)

