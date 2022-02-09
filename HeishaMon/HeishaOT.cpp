#include "OpenTherm.h"
#include "HeishaOT.h"
#include "decode.h"

OpenTherm ot(inOTPin, outOTPin, true);

unsigned long otResponse = 0;

heishaOTDataStruct heishaOTData;

void log_message(char* string);

void processOTRequest(unsigned long request, OpenThermResponseStatus status) {
  char log_msg[512];
  switch (ot.getDataID(request)) {
    case OpenThermMessageID::Status: {
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
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Status, responsedata);
      } break;
    case OpenThermMessageID::MaxTSetUBMaxTSetLB: {
        log_message((char *)"OpenTherm: Received read Ta-set bounds");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::MaxTSetUBMaxTSetLB, 0x5028);

      } break;
    case OpenThermMessageID::TdhwSetUBTdhwSetLB: {
        log_message((char *)"OpenTherm: Received read DHW-set bounds");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::TdhwSetUBTdhwSetLB, 0x5028);

      } break;
    case OpenThermMessageID::Tr: {
        heishaOTData.roomTemp = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.roomTemp);
        sprintf(log_msg, "OpenTherm: Room temp: %s", str);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::Tr, heishaOTData.roomTemp);

      } break;
    case OpenThermMessageID::MaxRelModLevelSetting: {
        float data = ot.getFloat(request);
        sprintf(log_msg, "OpenTherm: Max relative modulation level: %f", data);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MaxRelModLevelSetting, data);

      } break;
    case OpenThermMessageID::TrSet: {
        heishaOTData.roomTempSet = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.roomTempSet);
        sprintf(log_msg, "OpenTherm: Room setpoint: %s", str);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TrSet, heishaOTData.roomTempSet);

      } break;


    case OpenThermMessageID::SConfigSMemberIDcode: {
        log_message((char *)"OpenTherm: Received read slave config");
        unsigned int DHW = true;
        unsigned int Modulation = true;
        unsigned int Cool = true;
        unsigned int DHWConf = false;
        unsigned int Pump = true;
        unsigned int CH2 = false;

        unsigned int data = DHW | (Modulation << 1) | (Cool << 2) | (DHWConf << 3) | (Pump << 4) | (CH2 << 5);
        data <<= 8;
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::SConfigSMemberIDcode, data);

      } break;
    case OpenThermMessageID::NominalVentilationValue: {
        log_message((char *)"OpenTherm: Received read nominal ventilation value");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::NominalVentilationValue, 0);

      } break;
    case OpenThermMessageID::RemoteParameterSettingsVH: {
        log_message((char *)"OpenTherm: Received read remote parameters settings");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RemoteParameterSettingsVH, 0);

      } break;
    case OpenThermMessageID::TdhwSet: {
        float data = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, data);
        sprintf(log_msg, "OpenTherm: DHW setpoint: %s", str);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TdhwSet, data);

      } break;
    case OpenThermMessageID::MaxTSet: {
        float data = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, data);
        sprintf(log_msg, "OpenTherm: Max Ta-set setpoint: %s", str);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::MaxTSet, 0);

      } break;
    case OpenThermMessageID::RBPflags: {
        Serial1.println("OpenTherm: Received read RBP flags");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RBPflags, 0);

      } break;
    case OpenThermMessageID::ASFflags: {
        log_message((char *)"OpenTherm: Received read ASF flags");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::ASFflags, 0);

      } break;
    case OpenThermMessageID::TSet: {
        heishaOTData.chSetpoint = ot.getFloat(request);
        char str[200];
        sprintf((char *)&str, "%.*f", 4, heishaOTData.chSetpoint);
        sprintf(log_msg, "OpenTherm: Ta-set: %s", str);
        log_message(log_msg);
        otResponse = ot.buildResponse(OpenThermMessageType::WRITE_ACK, OpenThermMessageID::TSet, request & 0xffff);

      } break;
    case OpenThermMessageID::Tboiler: {
        log_message((char *)"OpenTherm: Received read boiler flow temp (outlet)");
        if (heishaOTData.outletTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.outletTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tboiler, data);

        }
      } break;
    case OpenThermMessageID::RelModLevel: {
        Serial1.println("OpenTherm: Received read relative modulation level");
        otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::RelModLevel, 0);

      } break;
    case OpenThermMessageID::Tret: {
        log_message((char *)"OpenTherm: Received read Tret");
        if (heishaOTData.inletTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.inletTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tret, data);

        }
      } break;
    case OpenThermMessageID::Tdhw: {
        log_message((char *)"OpenTherm: Received read DWH temp");
        int dhwTemp = 45; //temp
        if (dhwTemp > -99) {
          unsigned long data = ot.temperatureToData(dhwTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Tdhw, data);

        }
      } break;
    case OpenThermMessageID::Toutside: {
        log_message((char *)"OpenTherm: Received read outside temp");
        if (heishaOTData.outsideTemp > -99) {
          unsigned long data = ot.temperatureToData(heishaOTData.outsideTemp);
          otResponse = ot.buildResponse(OpenThermMessageType::READ_ACK, OpenThermMessageID::Toutside, data);

        }
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

        ot.setSmartPower((bool)SmartPower);
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
    default: {
        sprintf(log_msg, "OpenTherm: Unknown data ID: %d (%#010X)", ot.getDataID(request), request);
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
  heishaOTData.outsideTemp = actData[0] == '\0' ? 0 : getDataValue(actData, 14).toFloat();
  heishaOTData.inletTemp =  actData[0] == '\0' ? 0 : getDataValue(actData, 5).toFloat();
  heishaOTData.outletTemp =  actData[0] == '\0' ? 0 : getDataValue(actData, 6).toFloat();
  heishaOTData.flameState = actData[0] == '\0' ? 0 : ((getDataValue(actData, 8).toInt() > 0 ) ? true : false); //compressor freq as flame on state
  heishaOTData.chState = actData[0] == '\0' ? 0 : (((getDataValue(actData, 8).toInt() > 0 ) && (getDataValue(actData, 20).toInt() == 0 )) ? true : false); // 3-way valve on room
  heishaOTData.dhwState = actData[0] == '\0' ? 0 : (((getDataValue(actData, 8).toInt() > 0 ) && (getDataValue(actData, 20).toInt() == 1 )) ? true : false); /// 3-way valve on dhw

  // opentherm loop
  if (otResponse && ot.isReady()) {
    ot.sendResponse(otResponse);
    otResponse = 0;
  }
  ot.process();
}
