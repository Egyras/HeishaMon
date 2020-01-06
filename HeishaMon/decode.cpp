#include "decode.h"
#include "commands.h"

unsigned long nextalldatatime = 0;


String getBit1and2(byte input) {
  switch (input & 0b11000000) {
    case 0b01000000:
      return "0"; break;
    case 0b10000000:
      return "1"; break;
    default:
      return "-1"; break;
  }
}

String getBit3and4(byte input) {
  switch (input & 0b110000) {
    case 0b010000:
      return "0"; break;
    case 0b100000:
      return "1"; break;
    default:
      return "-1"; break;
  }
}

String getBit5and6(byte input) {
  switch (input & 0b1100) {
    case 0b0100:
      return "0"; break;
    case 0b1000:
      return "1"; break;
    default:
      return "-1"; break;
  }
}

String getBit7and8(byte input) {
  switch (input & 0b11) {
    case 0b01:
      return "0"; break;
    case 0b10:
      return "1"; break;
    default:
      return "-1"; break;
  }
}

String getLeft5bits(byte input) {
  switch (input & 0b11111000) {
    case 0b10001000:
      return "4";      break;
    case 0b01001000:
      return "0";      break;
    case 0b01010000:
      return "1";      break;
    case 0b01011000:
      return "2";      break;
    case 0b01100000:
      return "3";      break;
    default:      break;
  }
}

String getRight3bits(byte input) {
  switch (input & 0b111) {
    case 0b001:
      return "0";      break;
    case 0b010:
      return "1";      break;
    case 0b011:
      return "2";      break;
    case 0b100:
      return "3";      break;
    default:      break;
  }
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

// Decode ////////////////////////////////////////////////////////////////////////////
void decode_heatpump_data(char* data, DynamicJsonDocument &actData, PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];

  if (millis() > nextalldatatime) {
    actData.clear(); // clearing all actual data so everything will be updated and sent to mqtt
    nextalldatatime = millis() + UPDATEALLTIME;
  }



  //new style topic decoding
  int topicsrun[] = {3, 4, 18, 17, 20, 26, 9, 6, 8, 10, 13, 14, 5, 36, 37, 7, 33, 21, 27, 28, 29, 30, 31, 32, 33, 34, 2, 19, 15, 16, 22, 23, 24, 38, 39, 40, 41, 42, 43, 45}; // which topics are already working with new style
  for (int i = 0 ; i < sizeof(topicsrun) / sizeof(topicsrun[0]) ; i++) {
    int Topic_Number = topicsrun[i];
    String Topic_Name;
    byte Input_Byte;
    String Topic_Value;
    Topic_Name = topics[Topic_Number];
    Input_Byte = data[topicBytes[Topic_Number]];
    Topic_Value = topicFunctions[Topic_Number](Input_Byte);
    if ( actData[Topic_Name] != Topic_Value ) {
      actData[Topic_Name] = Topic_Value;
      sprintf(log_msg, "received %s: %d (%s)", Topic_Name.c_str(), (int)Input_Byte, Topic_Value.c_str()); log_message(log_msg);
      sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, Topic_Name.c_str()); mqtt_client.publish(mqtt_topic, Topic_Value.c_str(), MQTT_RETAIN_VALUES);
    }
  }

  // TOP1 //
  int PumpFlow1 = (int)data[170];
  float PumpFlow2 = ((((float)data[169] - 1) / 5) * 2) / 100;
  float PumpFlow = PumpFlow1 + PumpFlow2;
  if ( actData["Pump_Flow"] != PumpFlow ) {
    actData["Pump_Flow"] = PumpFlow;
    sprintf(log_msg, "received pump flow (Pump_Flow): %.2f", PumpFlow); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Pump_Flow"); mqtt_client.publish(mqtt_topic, String(PumpFlow).c_str(), MQTT_RETAIN_VALUES);
  }



  // TOP12 //
  int Operations_Counter  = word(data[180], data[179]) - 1;
  if ( actData["Operations_Counter"] != Operations_Counter ) {
    actData["Operations_Counter"] = Operations_Counter;
    sprintf(log_msg, "received (Operations_Counter): %.2f", Operations_Counter); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Operations_Counter"); mqtt_client.publish(mqtt_topic, String(Operations_Counter).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP11 //
  int Operations_Hours  = word(data[183], data[182]) - 1;
  if ( actData["Operations_Hours"] != Operations_Hours ) {
    actData["Operations_Hours"] = Operations_Hours;
    sprintf(log_msg, "received (Operations_Hours): %.2f", Operations_Hours); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Operations_Hours"); mqtt_client.publish(mqtt_topic, String(Operations_Hours).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP44 //
  int Error_type = (int)(data[113]);
  int Error_number = (int)(data[114]) - 17;
  char* Error_type_string;
  char* Error_string;
  switch (Error_type) {
    case 177:                  //B1=F type error
      Error_type_string = "F";
      sprintf(Error_string, "%s%X", Error_type_string, Error_number);
      break;
    case 161:                  //A1=H type error
      Error_type_string = "H";
      sprintf(Error_string, "%s%X", Error_type_string, Error_number);
      break;
    default:
      Error_string = "No error";
  }
  if ( actData["Error"] != Error_string ) {
    actData["Error"] = Error_string;
    sprintf(log_msg, "Last error: %d (%s)", Error_string); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Error"); mqtt_client.publish(mqtt_topic, Error_string, MQTT_RETAIN_VALUES);
  }


}

///////////////////////////////////////////////
