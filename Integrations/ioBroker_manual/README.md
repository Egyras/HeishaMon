## Manual integration in ioBroker

# General

With this manual, you can manually integrate the heishamon into iobroker via mqtt. 

# mqtt in iobroker

## Installation
1. Install the mqtt adapter for iobroker. Go to *Adapters* and search for `mqtt`.  
![adapters_mqtt](https://github.com/emign/HeishaMon/blob/master/Integrations/ioBroker_manual/images/adapters_mqtt.png)
2. Install the `MQTT Broker/Client` adapter with clicking the plus-symbol in the correspinding line. Wait for it to finish.

## Configuration

1. Navigate to the iobroker `Instances menu and click on the wrench symbol for the mqtt-instance. It will most likely be called `mqtt.0`

![mqtt_setup](https://github.com/emign/HeishaMon/blob/master/Integrations/ioBroker_manual/images/mqtt_setup.png)

2. Configure the Connection settings as follows:

Type: Server/Broker
Port: 1883 (default port, or chose your own)
Secure: Leave it unchecked for now until everything is set up and tested. You can activate it later.
User: Username you want your mqtt clients to authenticate with
Password: Password you want your mqtt clients to authenticate with

3. Save and close
4. At the instances menu, check if you need to activate the mqtt-instance with the play button.
5. Check if the light at the instances menu is green for mqtt. You can hover over it for more information. 

# mqtt in heishaMon

1. Log into heishamon and configure the mqtt settings at http://<<IPTOHEISHAMON>>/settings or at the setup page of the heishamon Accespoint when you install the heishamon from scratch.
2. Save and reboot

# Test
1. Go back to iobroker
2. Navigate to `objects
3. Wait. (seriously, grab a coffee or something. iobroker sometimes needs minutes to detect all incoming messages and propagate them correctly)

![iobroker_objects](https://github.com/emign/HeishaMon/blob/master/Integrations/ioBroker_manual/images/iobroker_objects.png)

4. When everything is setup correctly, a new group `panasonic_heat_pump` will be displayed. Under the subgroup `sdc` there already should be values coming in. They flash green when updated

![sdc values](https://github.com/emign/HeishaMon/blob/master/Integrations/ioBroker_manual/images/sdc_values.png)

# Setting values

There are setter-topics (direct children of the `panasonic_heat_pump`). When you change the value in the `value` column, they will instantly be send to the heatpump. 
**Important: If you want to use nodered automation using the iobroker outlets for the heatpump, you need to manually set the values you want to automate manually in iobroker once. Otherwise
they will not be transmitted**
