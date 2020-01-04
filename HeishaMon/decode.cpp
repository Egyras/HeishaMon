#include "decode.h"
#include "commands.h"

unsigned long nextalldatatime = 0;


// Decode ////////////////////////////////////////////////////////////////////////////
void decode_heatpump_data(char* data, DynamicJsonDocument &actData, PubSubClient &mqtt_client, void (*log_message)(char*)) {
  char log_msg[256];
  char mqtt_topic[256];
  
  if (millis() > nextalldatatime) {
    actData.clear(); // clearing all actual data so everything will be updated and sent to mqtt
    nextalldatatime = millis() + UPDATEALLTIME;
  }

  // TOP3 //  
  int Power_State = (int)(data[4]);
  String Power_State_string;
  switch (Power_State & 0b11) { //probably only last two bits for Power dhw state
    case 0b01:
      Power_State_string = "0";
      break;
    case 0b10:
      Power_State_string = "1";
      break;
    default:
      Power_State_string = "-1";
      break;
  }
  if ( actData["Power_State"] != Power_State_string ) {
    actData["Power_State"] = Power_State_string;
    sprintf(log_msg, "received Power state : %d (%s)", Power_State, Power_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Power_State"); mqtt_client.publish(mqtt_topic, Power_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP4 //
  int Mode_State = (int)(data[6]);
  String Mode_State_string;
  switch (Mode_State) {
    case 82:
      Mode_State_string = "0";
      break;
    case 83:
      Mode_State_string = "1";
      break;
    case 89:
      Mode_State_string = "2";
      break;
    case 97:
      Mode_State_string = "3";
      break;
    case 98:
      Mode_State_string = "4";
      break;
    case 99:
      Mode_State_string = "5";
      break;
    case 105:
      Mode_State_string = "6";
      break;
    default:
      Mode_State_string = "-1";
      break;
  }
  if ( actData["OpMode_State"] != Mode_State_string ) {
    actData["OpMode_State"] = Mode_State_string;
    sprintf(log_msg, "received OpMode state : %d (%s)", Mode_State, Mode_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "OpMode_State"); mqtt_client.publish(mqtt_topic, Mode_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP18 //
  int quietpower_Mode_State = (int)(data[7]);
  String Powerfull_Mode_State_string = "-1";
  String Quiet_Mode_State_string = "-1";
  switch (quietpower_Mode_State & 0b11111000) { // only interested in left most 5 bits for quiet state
    case 0b10001000:
      Quiet_Mode_State_string = "4";
      break;
    case 0b01001000:
      Quiet_Mode_State_string = "0";
      break;
    case 0b01010000:
      Quiet_Mode_State_string = "1";
      break;
    case 0b01011000:
      Quiet_Mode_State_string = "2";
      break;
    case 0b01100000:
      Quiet_Mode_State_string = "3";
      break;
    default:
      break;
  }
  switch (quietpower_Mode_State & 0b111) { // only interested in last 3 bits for powerfull state
    case 0b001:
      Powerfull_Mode_State_string = "0";
      break;
    case 0b010:
      Powerfull_Mode_State_string = "1";
      break;
    case 0b011:
      Powerfull_Mode_State_string = "2";
      break;
    case 0b100:
      Powerfull_Mode_State_string = "3";
      break;
    default:
      break;
  }
  if ( actData["Quietmode_Level"] != Quiet_Mode_State_string ) {
    actData["Quietmode_Level"] = Quiet_Mode_State_string;
    sprintf(log_msg, "received quietmode level : %d (%s)", quietpower_Mode_State, Quiet_Mode_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Quietmode_Level"); mqtt_client.publish(mqtt_topic, Quiet_Mode_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP17 //
  if ( actData["Powerfullmode_State"] != Powerfull_Mode_State_string ) {
    actData["Powerfullmode_State"] = Powerfull_Mode_State_string;
    sprintf(log_msg, "received powerfullmode state : %d (%s)", quietpower_Mode_State, Powerfull_Mode_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Powerfullmode_State"); mqtt_client.publish(mqtt_topic, Powerfull_Mode_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP20//
  int valve_defrost_State = (int)(data[111]);
  String Valve_State_string;
  switch (valve_defrost_State & 0b11) { //bitwise AND with 0b11 because we are only interested in last 2 bits of the byte.
    case 0b01:
      Valve_State_string = "0";
      break;
    case 0b10:
      Valve_State_string = "1";
      break;
    default:
      Valve_State_string = "-1";
      break;
  }
  if ( actData["Valve_State"] != Valve_State_string ) {
    actData["Valve_State"] = Valve_State_string;
    sprintf(log_msg, "received 3-way valve state : %d (%s)", valve_defrost_State, Valve_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Valve_State"); mqtt_client.publish(mqtt_topic, Valve_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP26 //  
  String Defrosting_State_string;
  switch (valve_defrost_State & 0b1100) { //bitwise AND with 0b1100 because we are only interested in these two bits
    case 0b0100:
      Defrosting_State_string = "0";
      break;
    case 0b1000:
      Defrosting_State_string = "1";
      break;
    default:
      Defrosting_State_string = "-1";
      break;
  }
  if ( actData["Defrosting_State"] != Defrosting_State_string ) {
    actData["Defrosting_State"] = Defrosting_State_string;
    sprintf(log_msg, "received defrosting state : %d (%s)", valve_defrost_State, Defrosting_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Defrosting_State"); mqtt_client.publish(mqtt_topic, Defrosting_State_string.c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP9 //
  float Tank_Target_Temp = (float)data[42] - 128;
  if ( actData["Tank_Target_Temp"] != Tank_Target_Temp ) {
    actData["Tank_Target_Temp"] = Tank_Target_Temp;
    sprintf(log_msg, "received temperature (Tank_Target_Temp): %.2f", Tank_Target_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Target_Temp"); mqtt_client.publish(mqtt_topic, String(Tank_Target_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP6 // 
  float Flow_Outlet_Temp = (float)data[139] - 128;
  if ( actData["Z1_Flow_Outlet_Temp"] != Flow_Outlet_Temp ) {
    actData["Z1_Flow_Outlet_Temp"] = Flow_Outlet_Temp;
    sprintf(log_msg, "received temperature (Z1_Flow_Outlet_Temp): %.2f", Flow_Outlet_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Z1_Flow_Outlet_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Outlet_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP10 //
  float Tank_Temp = (float)data[141] - 128;
  if ( actData["Tank_Temp"] != Tank_Temp ) {
    actData["Tank_Temp"] = Tank_Temp;
    sprintf(log_msg, "received temperature (Tank_Temp): %.2f", Tank_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Temp"); mqtt_client.publish(mqtt_topic, String(Tank_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP14 //
  float Outside_Temp = (float)data[142] - 128;
  if ( actData["Outside_Temp"] != Outside_Temp ) {
    actData["Outside_Temp"] = Outside_Temp;
    sprintf(log_msg, "received temperature (Outside_Temp): %.2f", Outside_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Outside_Temp"); mqtt_client.publish(mqtt_topic, String(Outside_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP5 //
  float Flow_Inlet_Temp = (float)data[143] - 128;
  if ( actData["Z1_Flow_Inlet_Temp"] != Flow_Inlet_Temp ) {
    actData["Z1_Flow_Inlet_Temp"] = Flow_Inlet_Temp;
    sprintf(log_msg, "received temperature (Z1_Flow_Inlet_Temp): %.2f", Flow_Inlet_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Z1_Flow_Inlet_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Inlet_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP36 //
  // *placeholder* byte 145 Z1_Water_Temp

  // TOP37 //
  // *placeholder* byte 146 Z2_Water_Temp

  // TOP42 //
  // *placeholder* byte 147 Z1_Water_Traget_Temp

  // TOP43 //
  // *placeholder* byte 148 Z2_Water_Target_Temp


  // TOP7 //
  float Flow_Target_Temp = (float)data[153] - 128;
  if ( actData["Z1_Flow_Target_Temp"] != Flow_Target_Temp ) {
    actData["Z1_Flow_Target_Temp"] = Flow_Target_Temp;
    sprintf(log_msg, "received temperature (Z1_Flow_Target_Temp): %.2f", Flow_Target_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Z1_Flow_Target_Temp"); mqtt_client.publish(mqtt_topic, String(Flow_Target_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP33 //
  float Room_Temp = (float)data[156] - 128;
  if ( actData["Room_Temp"] != Room_Temp ) {
    actData["Room_Temp"] = Room_Temp;
    sprintf(log_msg, "received temperature (Room_Temp): %.2f", Room_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Room_Temp"); mqtt_client.publish(mqtt_topic, String(Room_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP21 //
  float Outside_Pipe_Temp = (float)data[158] - 128;
  if ( actData["Outside_Pipe_Temp"] != Outside_Pipe_Temp ) {
    actData["Outside_Pipe_Temp"] = Outside_Pipe_Temp;
    sprintf(log_msg, "received temperature (Outside_Pipe_Temp): %.2f", Outside_Pipe_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Outside_Pipe_Temp"); mqtt_client.publish(mqtt_topic, String(Outside_Pipe_Temp).c_str(), MQTT_RETAIN_VALUES);
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

  // TOP8 //
  float CompFreq = (float)data[166] - 1;
  if ( actData["Compressor_Freq"] != CompFreq ) {
    actData["Compressor_Freq"] = CompFreq;
    sprintf(log_msg, "received compressor frequency (Compressor_Freq): %.2f", CompFreq); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Compressor_Freq"); mqtt_client.publish(mqtt_topic, String(CompFreq).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP27 //
  float Heat_Shift_Temp = (float)data[38] - 128;
  if ( actData["Z1_HeatShift_Temp"] != Heat_Shift_Temp ) {
    actData["Z1_HeatShift_Temp"] = Heat_Shift_Temp;
    sprintf(log_msg, "received temperature (Z1_HeatShift_Temp): %.2f", Heat_Shift_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Z1_HeatShift_Temp"); mqtt_client.publish(mqtt_topic, String(Heat_Shift_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP28 //
  float Cool_Shift_Temp = (float)data[39] - 128;
  if ( actData["Z1_CoolShift_Temp"] != Cool_Shift_Temp ) {
    actData["Z1_CoolShift_Temp"] = Cool_Shift_Temp;
    sprintf(log_msg, "received temperature (Z1_CoolShift_Temp): %.2f", Cool_Shift_Temp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Cool_Shift_Temp"); mqtt_client.publish(mqtt_topic, String(Cool_Shift_Temp).c_str(), MQTT_RETAIN_VALUES);
  }

  //TOP34 // 
  // *placeholder* byte 40 Z2_HeatShift_Temp

  //TOP35 //
  // *placeholder* byte 41 Z2_CoolShift_Temp

  // TOP29 //
  float HCurveOutHighTemp = (float)data[75] - 128;
  if ( actData["HCurve_OutHighTemp"] != HCurveOutHighTemp ) {
    actData["HCurve_OutHighTemp"] = HCurveOutHighTemp;
    sprintf(log_msg, "received temperature (HCurve_OutHighTemp): %.2f", HCurveOutHighTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutHighTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP30 //
  float HCurveOutLowTemp = (float)data[76] - 128;
  if ( actData["HCurve_OutLowTemp"] != HCurveOutLowTemp ) {
    actData["HCurve_OutLowTemp"] = HCurveOutLowTemp;
    sprintf(log_msg, "received temperature (HCurve_OutLowTemp): %.2f", HCurveOutLowTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutLowTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP32 //
  float HCurveOutsLowTemp = (float)data[77] - 128;
  if ( actData["HCurve_OutsLowTemp"] != HCurveOutsLowTemp ) {
    actData["HCurve_OutsLowTemp"] = HCurveOutsLowTemp;
    sprintf(log_msg, "received temperature (HCurve_OutsLowTemp): %.2f", HCurveOutsLowTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutsLowTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsLowTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP31 //
  float HCurveOutsHighTemp = (float)data[78] - 128;
  if ( actData["HCurve_OutsHighTemp"] != HCurveOutsHighTemp ) {
    actData["HCurve_OutsHighTemp"] = HCurveOutsHighTemp;
    sprintf(log_msg, "received temperature (HCurve_OutsHighTemp): %.2f", HCurveOutsHighTemp); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "HCurve_OutsHighTemp"); mqtt_client.publish(mqtt_topic, String(HCurveOutsHighTemp).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP2
  int ForceDHW_State = (int)(data[4]);
  String ForceDHW_State_string;
  switch (ForceDHW_State & 0b11000000) { //probably only first two bits for force dhw state
    case 0b01000000:
      ForceDHW_State_string = "0";
      break;
    case 0b10000000:
      ForceDHW_State_string = "1";
      break;
    default:
      ForceDHW_State_string = "-1";
      break;
  }
  if ( actData["ForceDHW_State"] != ForceDHW_State_string ) {
    actData["ForceDHW_State"] = ForceDHW_State_string;
    sprintf(log_msg, "received force DHW state : %d (%s)", ForceDHW_State, ForceDHW_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ForceDHW"); mqtt_client.publish(mqtt_topic, ForceDHW_State_string.c_str(), MQTT_RETAIN_VALUES);
  }
  
  // TOP19 //
  int Holiday_Mode_State = (int)(data[5]);
  String Holiday_Mode_State_string;
  switch (Holiday_Mode_State & 0b00110000) { //probably only these two bits determine holiday state
    case 0b00010000:
      Holiday_Mode_State_string = "0";
      break;
    case 0b00100000:
      Holiday_Mode_State_string = "1";
      break;
    case 0b00110000:
      Holiday_Mode_State_string = "2";
      break;      
    default:
      Holiday_Mode_State_string = "-1";
      break;
  }
  if ( actData["Holidaymode_State"] != Holiday_Mode_State_string ) {
    actData["Holidaymode_State"] = Holiday_Mode_State_string;
    sprintf(log_msg, "received Holidaymode state : %d (%s)", Holiday_Mode_State, Holiday_Mode_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Holidaymode_State"); mqtt_client.publish(mqtt_topic, Holiday_Mode_State_string.c_str(), MQTT_RETAIN_VALUES);
  }
  
  int MainSchedule_State = (int)(data[5]);
  String MainSchedule_State_string;
  switch (MainSchedule_State & 0b11000000) { //these two bits determine main schedule state
    case 0b01000000:
      MainSchedule_State_string = "0";
      break;
    case 0b10000000:
      MainSchedule_State_string = "1";
      break;
    default:
      MainSchedule_State_string = "-1";
      break;
  }

  // TOP?? //
  if ( actData["MainSchedule_State"] != MainSchedule_State_string ) {
    actData["MainSchedule_State"] = MainSchedule_State_string;
    sprintf(log_msg, "received MainSchedule_State state : %d (%s)", MainSchedule_State, MainSchedule_State_string.c_str()); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "MainSchedule_State"); mqtt_client.publish(mqtt_topic, MainSchedule_State_string.c_str(), MQTT_RETAIN_VALUES);
  }  

  // TOP23 //
  float Heat_Delta = (float)data[84] - 128;
  if ( actData["Heat_Delta"] != Heat_Delta ) {
    actData["Heat_Delta"] = Heat_Delta;
    sprintf(log_msg, "received temperature (Heat_Delta): %.2f", Heat_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Delta"); mqtt_client.publish(mqtt_topic, String(Heat_Delta).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP24 //
  float Cool_Delta = (float)data[94] - 128;
  if ( actData["Cool_Delta"] != Cool_Delta ) {
    actData["Cool_Delta"] = Cool_Delta;
    sprintf(log_msg, "received temperature (Cool_Delta): %.2f", Cool_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Cool_Delta"); mqtt_client.publish(mqtt_topic, String(Cool_Delta).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP22 //
  float Tank_Heat_Delta = (float)data[99] - 128;
  if ( actData["Tank_Heat_Delta"] != Tank_Heat_Delta ) {
    actData["Tank_Heat_Delta"] = Tank_Heat_Delta;
    sprintf(log_msg, "received temperature (Tank_Heat_Delta): %.2f", Tank_Heat_Delta); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Tank_Heat_Delta"); mqtt_client.publish(mqtt_topic, String(Tank_Heat_Delta).c_str(), MQTT_RETAIN_VALUES);
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

  // TOP16 //
  float Energy_Consumption = ((float)data[193] - 1.0) * 200;
  if ( actData["Heat_Energy_Consumption"] != Energy_Consumption ) {
    actData["Heat_Energy_Consumption"] = Energy_Consumption;
    sprintf(log_msg, "received Watt (Heat_Energy_Consumption): %.2f", Energy_Consumption); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Energy_Consumption"); mqtt_client.publish(mqtt_topic, String(Energy_Consumption).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP15 //
  float Energy_Production = ((float)data[194] - 1.0) * 200;
  if ( actData["Heat_Energy_Production"] != Energy_Production ) {
    actData["Heat_Energy_Production"] = Energy_Production;
    sprintf(log_msg, "received Watt (Heat_Energy_Production): %.2f", Energy_Production); log_message(log_msg);
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "Heat_Energy_Production"); mqtt_client.publish(mqtt_topic, String(Energy_Production).c_str(), MQTT_RETAIN_VALUES);
  }

  // TOP38
  // *placeholder* byte 195 Cool_Energy_Production

  // TOP39
  // *placeholder* byte 196 Cool_Energy_Consumtion

  // TOP40
  // *placeholder* byte 197 DHW_Energy_Production

  // TOP41
  // *placeholder* byte 198 DHW_Energy_Consumtion


}
//////////////////////////////////////////////////////////////////////////////////////////
