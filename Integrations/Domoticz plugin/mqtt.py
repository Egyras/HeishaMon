# Based on https://github.com/emontnemery/domoticz_mqtt_discovery
# Based on https://github.com/emontnemery/domoticz_mqtt_discovery
# version: 1.0.1
#
# Changelog
# 1.0.1: Aligned with PEP8 styleguide


import Domoticz
import time
import json
try:
 import os
except:
 Domoticz.Debug("Your Python environment is incomplete!")

class MqttClientSH2:
    address = ""
    port = ""
    _connection = None
    isConnected = False
    on_mqtt_connected_cb = None
    on_mqtt_disconnected_cb = None
    on_mqtt_message_cb = None

    def __init__(self, address, port, client_id, on_mqtt_connected_cb, on_mqtt_disconnected_cb, on_mqtt_message_cb, on_mqtt_subscribed_cb):
        Domoticz.Debug("MqttClient::__init__")

        self.address = address
        self.port = port
        self.client_id = client_id if client_id != "" else self._generate_mqtt_client_id()
        self.on_mqtt_connected_cb = on_mqtt_connected_cb
        self.on_mqtt_disconnected_cb = on_mqtt_disconnected_cb
        self.on_mqtt_subscribed_cb = on_mqtt_subscribed_cb
        self.on_mqtt_message_cb = on_mqtt_message_cb

        self._open()

    def __str__(self):
        Domoticz.Debug("MqttClient::__str__")

        if (self._connection != None):
            return str(self._connection)
        else:
            return "None"

    def _generate_mqtt_client_id(self):
       retval = 'Domoticz_' + str(int(time.time()))+'_'
       try:
        rarray = list(os.urandom(4))
        for i in range(len(rarray)):
         retval += str(rarray[i])
       except:
        pass # there are nothing we can do
       return retval

    def _open(self):
        Domoticz.Debug("MqttClient::open")

        if (self._connection != None):
            self.close()

        self.isConnected = False

        self._connection = Domoticz.Connection(
            Name=self.address,
            Transport="TCP/IP",
            Protocol="MQTTS" if self.port == "8883" else "MQTT",
            Address=self.address,
            Port=self.port
        )

        self._connection.Connect()

    def ping(self):
        Domoticz.Debug("MqttClient::ping")
        if (self._connection == None or not self.isConnected):
            self._open()
        else:
            self._connection.Send({'Verb': 'PING'})

    def publish(self, topic, payload, retain=0):
        Domoticz.Debug("MqttClient::publish " + topic + " (" + payload + ")")

        if (self._connection == None or not self.isConnected):
            self._open()
        else:
            self._connection.Send({
                'Verb': 'PUBLISH',
                'Topic': topic,
                'Payload': bytearray(payload, 'utf-8'),
                'Retain': retain
            })

    def subscribe(self, topics):
        Domoticz.Debug("MqttClient::subscribe")
        subscriptionlist = []
        for topic in topics:
            subscriptionlist.append({'Topic': topic, 'QoS': 0})
			
        if (self._connection == None or not self.isConnected):
            self._open()
        else:
            self._connection.Send({'Verb': 'SUBSCRIBE', 'Topics': subscriptionlist})

    def close(self):
        Domoticz.Debug("MqttClient::close")

        if self._connection != None and self._connection.Connected():
            self._connection.Send({ 'Verb' : 'DISCONNECT' })
            self._connection.Disconnect()

        self._connection = None
        self.isConnected = False

    def onConnect(self, Connection, Status, Description):
        if (self._connection != Connection):
            return

        if (Status == 0):
            Domoticz.Log("Connected to MQTT Server: {}:{}".format(
                Connection.Address, Connection.Port)
            )
            Domoticz.Debug("MQTT CLIENT ID: '" + self.client_id + "'")
            self._connection.Send({'Verb': 'CONNECT', 'ID': self.client_id})
        else:
            Domoticz.Error("Failed to connect to: {}:{}, Description: {}".format(
                Connection.Address, Connection.Port, Description)
            )

    def onDisconnect(self, Connection):
        if (self._connection != Connection):
            return

        Domoticz.Debug("MqttClient::onDisonnect")
        Domoticz.Error("Disconnected from MQTT Server: {}:{}".format(
            Connection.Address, Connection.Port)
        )

        self.close()

        if self.on_mqtt_disconnected_cb != None:
            self.on_mqtt_disconnected_cb()

    def onHeartbeat(self):
        if self._connection is None or (not self._connection.Connecting() and not self._connection.Connected() or not self.isConnected):
            Domoticz.Debug("MqttClient::Reconnecting")
            self._open()
        else:
            self.ping()

    def onMessage(self, Connection, Data):
        if (self._connection != Connection):
            return

        topic = Data['Topic'] if 'Topic' in Data else ''
        payload =  Data['Payload'].decode('utf8') if 'Payload' in Data else ''

        if Data['Verb'] == "CONNACK":
            self.isConnected = True
            if self.on_mqtt_connected_cb != None:
                self.on_mqtt_connected_cb()

        if Data['Verb'] == "SUBACK":
            if self.on_mqtt_subscribed_cb != None:
                self.on_mqtt_subscribed_cb()

        if Data['Verb'] == "PUBLISH":
            if self.on_mqtt_message_cb != None:
                message = ""

                try:
                    message = json.loads(payload)
                except ValueError:
                    message = payload

                self.on_mqtt_message_cb(topic, message)
