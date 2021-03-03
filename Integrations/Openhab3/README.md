
1. Create new mqtt generic Thing.
2. Open the Code tab and overwrite the code with the content of aquareaHeatPump.yaml, making sure you replace 'MQTTSERVICE' with yours mqtt bridge in the following lines:

```
UID: mqtt:topic:MQTTSERVICE:HPwemos1D
bridgeUID: mqtt:broker:MQTTSERVICE
```

3. For the mapping of number values into human readable strings use the *.map files. There are two strategies to do this, choose one:
  3a. If you use layouts you can specify the mapping in metadata and it will render Widgets properly. You can then just "Add from Model..." as a cell and it allows you to choose from those mapped labels. For every aquarea_ITEM.map file goto Model->ITEM->Add Metadata->State Description->Options and paste content into the field.
  OR
  3b. Copy the *.map files into openHAB-conf/transform folder and use them to map channels into string Items. Be aware that if you then want to update the Item you need to ensure that you use a number value to do that as the string mapping does not work in the opposite direction (item->channel). 



