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

String getIntMinus1Div5(byte input){
  return String((((float)input - 1) / 5),1);
  
}
String getIntMinus1Times10(byte input){
  int value = (int)input - 1;
  return (String)(value*10);
  
}
String getIntMinus1Times50(byte input){
  int value = (int)input - 1;
  return (String)(value*50);
}

String unknown(byte input) {
  return "-1";
}

String getOpMode(byte input) {
  switch ((int)input) {
    case 82:
      return "0";
    case 83:
      return "1";
    case 89:
      return "2";
    case 97:
      return "3";
    case 98:
      return "4";
    case 99:
      return "5";
    case 105:
      return "6";
    case 90:
      return "7";
    case 106:
      return "8";
    default:
      return "-1";
  }
}

String getModel(byte input) {
  switch ((int)input) {
    case 19:
      return "0";
    case 20:
      return "1";
    case 119:
      return "2";
    case 136:
      return "3";
    case 133:
      return "4";
    case 134:
      return "5";
    case 135:
      return "6";
    case 113:
      return "7";
    case 67:
      return "8";
    case 51:
      return "9";
    default:
      return "-1";
  }
}

String getEnergy(byte input)
    {
      int value = ((int)input - 1) * 200;
      return (String)value;
}

String getPumpFlow(char* data){   // TOP1 //
  int PumpFlow1 = (int)data[170];
  float PumpFlow2 = (((float)data[169]-1) / 256);
  float PumpFlow = PumpFlow1 + PumpFlow2;
  return String(PumpFlow,2);
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
void decode_heatpump_data(char* data, String actData[], PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, unsigned int updateAllTime) {
  char log_msg[256];
  char mqtt_topic[256];
  bool updatenow = false;

  if (millis() > nextalldatatime) {
    updatenow = true;
    nextalldatatime = millis() + (1000 * updateAllTime);
  }

  for (unsigned int Topic_Number = 0 ; Topic_Number < NUMBER_OF_TOPICS ; Topic_Number++) {
    byte Input_Byte;
    String Topic_Value;
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
      case 90:
        Topic_Value = String(word(data[186], data[185]) - 1);
        break;
      case 91:
        Topic_Value = String(word(data[189], data[188]) - 1);
        break;
      case 44:
        Topic_Value = getErrorInfo(data);
        break;
      default:
        Input_Byte = data[topicBytes[Topic_Number]];
        Topic_Value = topicFunctions[Topic_Number](Input_Byte);
        break;
    }
    if ((updatenow) || ( actData[Topic_Number] != Topic_Value )) {
      actData[Topic_Number] = Topic_Value;
      sprintf(log_msg, "received TOP%d %s: %s", Topic_Number, topics[Topic_Number], Topic_Value.c_str()); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s/%s", mqtt_topic_base, mqtt_topic_values, topics[Topic_Number]); mqtt_client.publish(mqtt_topic, Topic_Value.c_str(), MQTT_RETAIN_VALUES);
    }
  }

}
