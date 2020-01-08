#include "decode.h"
#include "commands.h"

unsigned long nextalldatatime = 0;


String getBit1and2(byte input) {
  return String((input  >> 6)-1);
}

String getBit3and4(byte input) {
  return String(((input >> 4) & 0b11)-1);
}

String getBit5and6(byte input) {
  return String(((input >> 2) & 0b11)-1);
}

String getBit7and8(byte input) {
  return String((input & 0b11)-1);
}

String getBit3and4and5(byte input) {
  return String(((input >> 3) & 0b111)-1);
}


String getLeft5bits(byte input) {
  return String((input >> 3)-1);
}

String getRight3bits(byte input) {
  return String((input & 0b111)-1);
}

String getIntMinus1(byte input) {
  int value = (int)input - 1;
  return (String)value;
}

String getIntMinus128(byte input) {
  int value = (int)input - 128;
  return (String)value;
}


String unknown(byte input) {
  return "-1";
}

String getOpMode(byte input) {
  switch ((int)input) {
    case 82:
      return "0";      break;
    case 83:
      return "1";      break;
    case 89:
      return "2";      break;
    case 97:
      return "3";      break;
    case 98:
      return "4";      break;
    case 99:
      return "5";      break;
    case 105:
      return "6";      break;
    default:
      return "-1";      break;
  }
}

String getEnergy(byte input) {
  int value = ((int)input - 1) * 200;
  return (String)value;
}

String getPumpFlow(char* data){   // TOP1 //
  int PumpFlow1 = (int)data[170];
  float PumpFlow2 = ((((float)data[169] - 1) / 5) * 2) / 100;
  float PumpFlow = PumpFlow1 + PumpFlow2;
  return String(PumpFlow);
}

String getErrorInfo(char* data){ // TOP44 //
  int Error_type = (int)(data[113]);
  int Error_number = ((int)(data[114])) - 17;
  char Error_string[10];
  switch (Error_type) {
    case 177:                  //B1=F type error
      sprintf(Error_string, "F%02X", Error_number);
      break;
    case 161:                  //A1=H type error
      sprintf(Error_string, "H%02X", Error_number);
      break;
    default:
      sprintf(Error_string,"No error");
      break;
  }
  return String(Error_string);  
}

// Decode ////////////////////////////////////////////////////////////////////////////
void decode_heatpump_data(char* data, DynamicJsonDocument &actData, PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];

  if (millis() > nextalldatatime) {
    actData.clear(); // clearing all actual data so everything will be updated and sent to mqtt
    nextalldatatime = millis() + UPDATEALLTIME;
  }

  for (int Topic_Number = 0 ; Topic_Number < sizeof(topics) / sizeof(topics[0]) ; Topic_Number++) {
    String Topic_Name;
    byte Input_Byte;
    String Topic_Value;
    Topic_Name = topics[Topic_Number];
    switch (Topic_Number) { //switch on topic numbers, some have special needs
      case 1: 
        Topic_Value = getPumpFlow(data);
        break;
      case 11:
        Topic_Value = String(word(data[183], data[182]) - 1);
        break;
      case 12:
        Topic_Value = String(word(data[180], data[179]) - 1);
        break;
      case 44:
        Topic_Value = getErrorInfo(data);
        break;
      default:
        Input_Byte = data[topicBytes[Topic_Number]];
        Topic_Value = topicFunctions[Topic_Number](Input_Byte);
        break;
    }
    if ( actData[Topic_Name] != Topic_Value ) {
      actData[Topic_Name] = Topic_Value;
      sprintf(log_msg, "received %s: %s", Topic_Name.c_str(), Topic_Value.c_str()); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, Topic_Name.c_str()); mqtt_client.publish(mqtt_topic, Topic_Value.c_str(), MQTT_RETAIN_VALUES);
    }
  }

  //run this only to check state of json doc. memory usage (currently sized at 4096)
  //sprintf(log_msg, "JSON doc memory usage: %d", actData.memoryUsage()); log_message(log_msg);

}
