#include "OpenTherm.h"
#include "HeishaOT.h"
#include "decode.h"
#include "src/common/stricmp.h"

OpenTherm ot(inOTPin, outOTPin, true);

const char* mqtt_topic_opentherm PROGMEM = "opentherm";

unsigned long otResponse = 0;

heishaOTDataStruct heishaOTData;

void log_message(char* string);
void mqttPublish(char* topic, char* subtopic, char* value);

void processOTRequest(unsigned long request, OpenThermResponseStatus status) {
  char log_msg[512];
  {
    char str[200];
    sprintf(str, "%#010x", request);
    mqttPublish((char*)mqtt_topic_opentherm, (char*)"raw", str);
  }
  switch (ot.getDataID(request)) {
    case OpenThermMessageID::Status: { //mandatory
        unsigned long data = ot.getUInt(request);
        unsigned int CHEnable = (data >> 8) & (1 << 0);
        heishaOTData.chEnable = (bool)CHEnable;
        unsigned int DHWEnable = ((data >> 8) & (1 << 1)) >> 1;
        unsigned int Cooling = ((data >> 8) & (1 << 2)) >> 2;
        unsigned int OTCEnable = ((data >> 8) & (1 << 3)) >> 3;
        unsigned int CH2Enable = ((data >> 8) & (1 << 4)) >> 4;
        unsigned int SWMode = ((data >> 8) & (1 << 5)) >> 5;
        unsigned int DHWBlock = ((data >> 8) & (1 << 6)) >> 6;
        sprintf(log_msg,
                "OpenTherm: Received status check: %d, CH: %d, DHW: %d, Cooling, %d, OTC: %d, CH2: %d, SWMode: %d, DHWBlock: %d",
                data >> 8, CHEnable, DHWEnable, Cooling, OTCEnable, CH2Enable, SWMode, DHWBlock
               );
        log_message(log_msg);
        //clean slave bits from 2-byte data
        data = ((data >> 8) << 8);

        unsigned int FaultInd = false;
        unsigned int CHMode = (unsigned int)heishaOTData.chState;
        unsigned int FlameStatus = (unsigned int)heishaOTData.flameState;
        unsigned int DHWMode = (unsigned int)heishaOTData.dhwState;
        unsigned int CoolingStatus = false;
        unsigned int CH2 = false;
        unsigned int DiagInd = false;
        sprintf(log_msg,
                "OpenTherm: Send status: CH: %d, Flame:%d, DHW:%d",
                CHMode, FlameStatus, DHWMode
               );
        log_message(log_msg);
        unsigned int responsedata = FaultInd | (CHMode << 1) | (DHWMode << 2) | (FlameStatus << 3) | (CoolingStatus << 4) | (CH2 << 5) | (DiagInd << 6);
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Status, (data |= responsedata));
      } break;
    case OpenThermMessageID::TSet: { //mandatory
        heishaOTData.chSetpoint = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.chSetpoint);
        sprintf(log_msg, "OpenTherm: control setpoint TSet: %s", str);
        log_message(log_msg);
        mqttPublish((char*)mqtt_topic_opentherm, (char*)"chSetpoint", str);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TSet, request & 0xffff);

      } break;
    case OpenThermMessageID::SConfigSMemberIDcode: { //mandatory
        log_message((char *)"OpenTherm: Received read slave config");
        unsigned int DHW = true;
        unsigned int Modulation = false;
        unsigned int Cool = false;
        unsigned int DHWConf = false;
        unsigned int Pump = false;
        unsigned int CH2 = false; // no 2nd zone yet

        unsigned int data = DHW | (Modulation << 1) | (Cool << 2) | (DHWConf << 3) | (Pump << 4) | (CH2 << 5);
        data <<= 8;
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::SConfigSMemberIDcode, data);

      } break;
    case OpenThermMessageID::MaxRelModLevelSetting: { //mandatory
        float data = ot.getFloat(request);
        sprintf(log_msg, "OpenTherm: Max relative modulation level: %f", data);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MaxRelModLevelSetting, request & 0xffff); //ACK for mandatory fields

      } break;
    case OpenThermMessageID::RelModLevel: { //mandatory
        log_message((char *)"OpenTherm: Received read relative modulation level");
        otResponse = ot.buildResponse(OpenThermMessageType::DATA_INVALID, OpenThermMessageID::RelModLevel, request & 0xffff); //invalid for now to fill mandatory fields
      } break;
    case OpenThermMessageID::Tboiler: { //mandatory
        log_message((char *)"OpenTherm: Received read boiler flow temp (outlet)");
        if (heishaOTData.outletTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.outletTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tboiler, data);

        } else {
          otResponse = ot.buildResponse(OpenThermMessageType::DATA_INVALID, OpenThermMessageID::Tboiler, request & 0xffff);
        }
      } break;
    // now adding some more useful, not mandatory, types
    case OpenThermMessageID::RBPflags: { //Pre-Defined Remote Boiler Parameters
        log_message((char *)"OpenTherm: Received Remote Boiler parameters request");
        //fixed settings for now
        const unsigned int DHWsetTransfer = true;
        const unsigned int maxCHsetTransfer = true;
        const unsigned int DHWsetReadWrite = true;
        const unsigned int maxCHsetReadWrite = true;
        const unsigned int responsedata = DHWsetReadWrite | (maxCHsetReadWrite << 1) | (DHWsetTransfer << 8) | (maxCHsetTransfer << 9);
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RBPflags, responsedata);
      } break;
    case OpenThermMessageID::TdhwSetUBTdhwSetLB : { //DHW boundaries
        log_message((char *)"OpenTherm: Received DHW set boundaries request");
        //fixed settings for now
        const unsigned int DHWsetUppBound = 75;
        const unsigned int DHWsetLowBound = 40;
        const unsigned int responsedata = DHWsetLowBound | (DHWsetUppBound << 8);
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TdhwSetUBTdhwSetLB, responsedata);
      } break;
    case OpenThermMessageID::MaxTSetUBMaxTSetLB  : { //CHset boundaries
        log_message((char *)"OpenTherm: Received CH set boundaries request");
        //fixed settings for now, seems valid for most heatpump types
        const unsigned int CHsetUppBound = 65;
        const unsigned int CHsetLowBound = 20;
        const unsigned int responsedata = CHsetLowBound | (CHsetUppBound << 8);
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::MaxTSetUBMaxTSetLB, responsedata);
      } break;
    case OpenThermMessageID::Tr: {
        heishaOTData.roomTemp = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.roomTemp);
        sprintf(log_msg, "OpenTherm: Room temp: %s", str);
        log_message(log_msg);
        mqttPublish((char*)mqtt_topic_opentherm, (char*)"roomTemp", str);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::Tr, request & 0xffff);

      } break;
    case OpenThermMessageID::TrSet: {
        heishaOTData.roomTempSet = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.roomTempSet);
        sprintf(log_msg, "OpenTherm: Room setpoint: %s", str);
        log_message(log_msg);
        mqttPublish((char*)mqtt_topic_opentherm, (char*)"roomTempSet", str);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TrSet, request & 0xffff);
      } break;
    case OpenThermMessageID::TdhwSet: {
        if (ot.getMessageType(request) == OpenThermMessageType::WRITE_DATA) {
          heishaOTData.dhwSetpoint = ot.getFloat(request);
          char str[200];
          sprintf((char *)&str, "%.*f", 4, heishaOTData.dhwSetpoint);
          sprintf(log_msg, "OpenTherm: Write request DHW setpoint: %s", str);
          log_message(log_msg);
          mqttPublish((char*)mqtt_topic_opentherm, (char*)"dhwSetpoint", str);
          otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TdhwSet, ot.temperatureToData(heishaOTData.dhwSetpoint));
        } else { //READ_DATA
          sprintf(log_msg, "OpenTherm: Read request DHW setpoint");
          log_message(log_msg);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TdhwSet, ot.temperatureToData(heishaOTData.dhwSetpoint));
        }
      } break;
    case OpenThermMessageID::MaxTSet: {
        if (ot.getMessageType(request) == OpenThermMessageType::WRITE_DATA) {
          heishaOTData.maxTSet = ot.getFloat(request);
          char str[200];
          sprintf((char *)&str, "%.*f", 4, heishaOTData.maxTSet);
          sprintf(log_msg, "OpenTherm: Write request Max Ta-set setpoint: %s", str);
          log_message(log_msg);
          mqttPublish((char*)mqtt_topic_opentherm, (char*)"maxTSet", str);
          otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MaxTSet, ot.temperatureToData(heishaOTData.maxTSet));
        } else { //READ_DATA
          sprintf(log_msg, "OpenTherm: Read request Max Ta-set setpoint");
          log_message(log_msg);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::MaxTSet, ot.temperatureToData(heishaOTData.maxTSet));
        }
      } break;
    case OpenThermMessageID::Tret: {
        log_message((char *)"OpenTherm: Received read Tret");
        if (heishaOTData.inletTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.inletTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tret, data);
        } else {
          otResponse = ot.buildResponse(OpenThermMessageType::DATA_INVALID, OpenThermMessageID::Tboiler, request & 0xffff);
        }
      } break;
    case OpenThermMessageID::Tdhw: {
        log_message((char *)"OpenTherm: Received read DWH temp");
        int dhwTemp = 45; //temp
        if (dhwTemp > -99) {
          unsigned long data = ot.temperatureToData(dhwTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tdhw, data);
        } else {
          otResponse = ot.buildResponse(OpenThermMessageType::DATA_INVALID, OpenThermMessageID::Tboiler, request & 0xffff);
        }
      } break;
    case OpenThermMessageID::Toutside: {
        log_message((char *)"OpenTherm: Received read outside temp");
        if (heishaOTData.outsideTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.outsideTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Toutside, data);

        } else {
          otResponse = ot.buildResponse(OpenThermMessageType::DATA_INVALID, OpenThermMessageID::Tboiler, request & 0xffff);
        }
      } break;
    /*
      case OpenThermMessageID::ASFflags: {
        log_message((char *)"OpenTherm: Received read ASF flags");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::ASFflags, 0);

      } break;
      case OpenThermMessageID::MaxTSetUBMaxTSetLB: {
      log_message((char *)"OpenTherm: Received read Ta-set bounds");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::MaxTSetUBMaxTSetLB, 0x5028);

      } break;
      case OpenThermMessageID::TdhwSetUBTdhwSetLB: {
      log_message((char *)"OpenTherm: Received read DHW-set bounds");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TdhwSetUBTdhwSetLB, 0x5028);

      } break;
      case OpenThermMessageID::NominalVentilationValue: {
      log_message((char *)"OpenTherm: Received read nominal ventilation value");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::NominalVentilationValue, 0);

      } break;
      case OpenThermMessageID::RemoteParameterSettingsVH: {
      log_message((char *)"OpenTherm: Received read remote parameters settings");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RemoteParameterSettingsVH, 0);

      } break;

      case OpenThermMessageID::RBPflags: {
      Serial1.println("OpenTherm: Received read RBP flags");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RBPflags, 0);

      } break;

      case OpenThermMessageID::TrOverride: {
      log_message((char *)"OpenTherm: Received read remote override setpoint");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TrOverride, 0);

      } break;
      case OpenThermMessageID::CHPressure: {
      log_message((char *)"OpenTherm: Received read CH pressure");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::CHPressure, 0);

      } break;
      case OpenThermMessageID::OpenThermVersionMaster: {
      float data = ot.getFloat(request);
      char str[200];
      sprintf((char *)&str, "%.*f", 4, data);
      sprintf(log_msg, "OpenTherm: OT Master version: %s", str);
      log_message(log_msg);
      otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::OpenThermVersionMaster, request & 0xffff);

      } break;
      case OpenThermMessageID::OpenThermVersionSlave: {
      log_message((char *)"OpenTherm: Received read OT slave version");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::OpenThermVersionSlave, 0);

      } break;
      case OpenThermMessageID::MasterVersion: {
      float data = ot.getFloat(request);
      char str[200];
      sprintf((char *)&str, "%.*f", 4, data);
      sprintf(log_msg, "OpenTherm: Master device version: %s", str);
      log_message(log_msg);
      otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MasterVersion, request & 0xffff);


      } break;
      case OpenThermMessageID::SlaveVersion: {
      log_message((char *)"OpenTherm: Received read slave device version");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::SlaveVersion, 0);

      } break;
      case OpenThermMessageID::MConfigMMemberIDcode: {
      unsigned long data = ot.getUInt(request);
      unsigned int SmartPower = (data >> 8) & (1 << 0);
      sprintf(log_msg,
              "OpenTherm: Received master config: %d, Smartpower: %d",
              data >> 8, SmartPower
             );
      log_message(log_msg);
      otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MConfigMMemberIDcode, data);

      //ot.setSmartPower((bool)SmartPower); not working correctly yet
      } break;

      case OpenThermMessageID::TSP: {
      log_message((char *)"OpenTherm: Received read TSP");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TSP, 0);

      } break;
      case OpenThermMessageID::FHBsize: {
      log_message((char *)"OpenTherm: Received read fault buffer size");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::FHBsize, 0);

      } break;

      case OpenThermMessageID::RemoteOverrideFunction: {
      log_message((char *)"OpenTherm: Received read remote override function");
      otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RemoteOverrideFunction, 0);

      } break;
    */
    default: {
        sprintf(log_msg, "OpenTherm: Unknown data ID: %d (%#010x)", ot.getDataID(request), request);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::UNKNOWN_DATA_ID, ot.getDataID(request), 0);
      } break;

  }
}

void IRAM_ATTR handleOTInterrupt() {
  ot.handleInterrupt();
}

void HeishaOTSetup() {
  ot.begin(handleOTInterrupt, processOTRequest);
}

void HeishaOTLoop(char * actData, PubSubClient &mqtt_client, char* mqtt_topic_base) {
  //heishaOTData.outsideTemp = actData[0] == '\0' ? 0 : getDataValue(actData, 14).toFloat();
  //heishaOTData.inletTemp =  actData[0] == '\0' ? 0 : getDataValue(actData, 5).toFloat();
  //heishaOTData.outletTemp =  actData[0] == '\0' ? 0 : getDataValue(actData, 6).toFloat();
  //heishaOTData.flameState = actData[0] == '\0' ? 0 : ((getDataValue(actData, 8).toInt() > 0 ) ? true : false); //compressor freq as flame on state
  //heishaOTData.chState = actData[0] == '\0' ? 0 : (((getDataValue(actData, 8).toInt() > 0 ) && (getDataValue(actData, 20).toInt() == 0 )) ? true : false); // 3-way valve on room
  //heishaOTData.dhwState = actData[0] == '\0' ? 0 : (((getDataValue(actData, 8).toInt() > 0 ) && (getDataValue(actData, 20).toInt() == 1 )) ? true : false); /// 3-way valve on dhw

  // opentherm loop
  if (otResponse && ot.isReady()) {
    ot.sendResponse(otResponse);
    otResponse = 0;
  }
  ot.process();
}

void mqttOTCallback(char* topic, char* value) {
  //only READ values can be overwritten using received mqtt messages
  //log_message((char *)"OpenTherm: MQTT message received");
  if (strcmp((char*)"outsideTemp", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'outsideTemp'");
    heishaOTData.outsideTemp = String(value).toFloat();
  }
  else if (strcmp((char*)"inletTemp", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'inletTemp'");
    heishaOTData.inletTemp = String(value).toFloat();
  }
  else if (strcmp((char*)"outletTemp", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'outletTemp'");
    heishaOTData.outletTemp = String(value).toFloat();
  }
  else if (strcmp((char*)"dhwSetpoint", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'dhwSetpoint'");
    heishaOTData.dhwSetpoint = String(value).toFloat();
  }
  else if (strcmp((char*)"maxTSet", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'maxTSet'");
    heishaOTData.maxTSet = String(value).toFloat();
  }
  else if (strcmp((char*)"flameState", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'flameState'");
    heishaOTData.flameState = ((stricmp((char*)"true", value) == 0) || (String(value).toInt() == 1 ));
  }
  else if (strcmp((char*)"chState", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'chState'");
    heishaOTData.chState = ((stricmp((char*)"true", value) == 0) || (String(value).toInt() == 1 ));
  }
  else if (strcmp((char*)"dhwState", topic) == 0) {
    log_message((char *)"OpenTherm: MQTT message received 'dhwState'");
    heishaOTData.dhwState = ((stricmp((char*)"true", value) == 0) || (String(value).toInt() == 1 ));
  }

}

void openthermTableOutput(struct webserver_t *client) {
  char str[64];
  //roomtemp
  webserver_send_content_P(client, PSTR("<tr><td>roomTemp</td><td>R</td><td>"), 35);
  dtostrf( heishaOTData.roomTemp, 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //roomtempset
  webserver_send_content_P(client, PSTR("<tr><td>roomTempSet</td><td>R</td><td>"), 38);
  dtostrf( heishaOTData.roomTempSet , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //chSetpoint
  webserver_send_content_P(client, PSTR("<tr><td>chSetpoint</td><td>R</td><td>"), 37);
  dtostrf( heishaOTData.chSetpoint , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //dhwSetpoint
  webserver_send_content_P(client, PSTR("<tr><td>dhwSetpoint</td><td>RW</td><td>"), 40);
  dtostrf( heishaOTData.dhwSetpoint , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //maxTSet
  webserver_send_content_P(client, PSTR("<tr><td>maxTSet</td><td>RW</td><td>"), 35);
  dtostrf( heishaOTData.maxTSet , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //outsideTemp
  webserver_send_content_P(client, PSTR("<tr><td>outsideTemp</td><td>W</td><td>"), 38);
  dtostrf( heishaOTData.outsideTemp , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //inletTemp
  webserver_send_content_P(client, PSTR("<tr><td>inletTemp</td><td>W</td><td>"), 36);
  dtostrf( heishaOTData.inletTemp , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //outletTemp
  webserver_send_content_P(client, PSTR("<tr><td>outletTemp</td><td>W</td><td>"), 37);
  dtostrf( heishaOTData.outletTemp , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //boilerTemp
  webserver_send_content_P(client, PSTR("<tr><td>boilerTemp</td><td>W</td><td>"), 37);
  dtostrf( heishaOTData.boilerTemp , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //DHWTemp
  webserver_send_content_P(client, PSTR("<tr><td>DHWTemp</td><td>W</td><td>"), 34);
  dtostrf( heishaOTData.DHWTemp , 0, 2, str);
  webserver_send_content(client, str, strlen(str));
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //flameState
  webserver_send_content_P(client, PSTR("<tr><td>flameState</td><td>W</td><td>"), 37);
  heishaOTData.flameState ? webserver_send_content_P(client, PSTR("on"), 2) : webserver_send_content_P(client, PSTR("off"), 3);
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //chState
  webserver_send_content_P(client, PSTR("<tr><td>chState</td><td>W</td><td>"), 34);
  heishaOTData.chState ? webserver_send_content_P(client, PSTR("on"), 2) : webserver_send_content_P(client, PSTR("off"), 3);
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
  //dhwState
  webserver_send_content_P(client, PSTR("<tr><td>dhwState</td><td>W</td><td>"), 35);
  heishaOTData.dhwState ? webserver_send_content_P(client, PSTR("on"), 2) : webserver_send_content_P(client, PSTR("off"), 3);
  webserver_send_content_P(client, PSTR("</td></tr>"), 10);
}
