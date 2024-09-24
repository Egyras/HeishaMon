#include <ArduinoJson.h>
#include <PubSubClient.h>

#define MQTT_RETAIN_VALUES 1

void resetlastalldatatime();

String getDataValue(char* data, unsigned int Topic_Number);
String getDataValueExtra(char* data, unsigned int Topic_Number);
String getOptDataValue(char* data, unsigned int Topic_Number);
void decode_heatpump_data(char* data, char* actData, PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, unsigned int updateAllTime);
void decode_heatpump_data_extra(char* data, char* actDataExtra, PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, unsigned int updateAllTime);
void decode_optional_heatpump_data(char* data, char* actOptDat, PubSubClient &mqtt_client, void (*log_message)(char*), char* mqtt_topic_base, unsigned int updateAllTime);

String unknown(byte input);
String getBit1(byte input);
String getBit1and2(byte input);
String getBit3and4(byte input);
String getBit5and6(byte input);
String getBit7and8(byte input);
String getBit3and4and5(byte input);
String getLeft5bits(byte input);
String getRight3bits(byte input);
String getIntMinus1(byte input);
String getIntMinus128(byte input);
String getIntMinus1Div5(byte input);
String getIntMinus1Div50(byte input);
String getIntMinus1Times10(byte input);
String getIntMinus1Times50(byte input);
String getOpMode(byte input);
String getPower(byte input);
String getHeatMode(byte input);
String getModel(byte input);
String getFirstByte(byte input);
String getSecondByte(byte input);
String getUintt16(char * data, byte input);

static const char _unknown[] PROGMEM = "unknown";

static const char *Model[] PROGMEM = {
  "51", //string representation of number of known models (last model number + 1)
  "WH-MDC05H3E5", //0
  "WH-MDC07H3E5", //1
  "IDU:WH-SXC09H3E5, ODU:WH-UX09HE5", //2
  "IDU:WH-SDC09H3E8, ODU:WH-UD09HE8", //3
  "IDU:WH-SXC09H3E8, ODU:WH-UX09HE8", //4
  "IDU:WH-SXC12H9E8, ODU:WH-UX12HE8", //5
  "IDU:WH-SXC16H9E8, ODU:WH-UX16HE8", //6
  "IDU:WH-SDC05H3E5, ODU:WH-UD05HE5", //7
  "IDU:WH-SDC0709J3E5, ODU:WH-UD09JE5",  //8
  "WH-MDC05J3E5", //9
  "WH-MDC09H3E5", //10
  "WH-MXC09H3E5", //11
  "IDU:WH-ADC0309J3E5, ODU:WH-UD09JE5", //12
  "IDU:WH-ADC0916H9E8, ODU:WH-UX12HE8", //13
  "IDU:WH-SQC09H3E8, ODU:WH-UQ09HE8", //14
  "IDU:WH-SDC09H3E5, ODU:WH-UD09HE5", //15
  "IDU:WH-ADC0309H3E5, ODU:WH-UD09HE5", //16
  "IDU:WH-ADC0309J3E5, ODU:WH-UD05JE5", //17
  "IDU:WH-SDC0709J3E5, ODU:WH-UD07JE5", //18
  "IDU:WH-SDC07H3E5-1, ODU:WH-UD07HE5-1", //19
  "WH-MDC07J3E5", //20
  "WH-MDC09J3E5", //21
  "IDU:WH-SDC0305J3E5, ODU:WH-UD05JE5", //22
  "WH-MXC09J3E8", //23
  "WH-MXC12J9E8", //24
  "IDU:WH-ADC1216H6E5, ODU:WH-UD12HE5", //25
  "IDU:WH-ADC0309J3E5C, ODU:WH-UD07JE5", //26
  "WH-MDC07J3E5", //27
  "WH-MDC05J3E5", //28
  "IDU:WH-UQ12HE8, ODU:WH-SQC12H9E8", //29
  "IDU:WH-SXC12H6E5, ODU:WH-UX12HE5", //30
  "WH-MDC09J3E5", //31
  "WH-MXC09J3E5", //32
  "IDU:WH-ADC1216H6E5C ODU:WH-UD12HE5", //33
  "IDU:WH-ADC0509L3E5 ODU:WH-WDG07LE5", //34
  "IDU:WH-SXC09H3E8 ODU:WH-UX09HE8", //35
  "IDU:WH-ADC0309K3E5AN ODU:WH-UDZ07KE5", //36
  "IDU:WH-SDC0309K3E5 ODU:WH-UDZ05KE5", //37
  "IDU:WH-SDC0509L3E5 ODU:WH-WDG09LE5", //38
  "IDU:WH-SDC12H9E8 ODU:WH-UD12HE8", //39
  "IDU:WH-SDC0309K3E5, ODU:WH-UDZ07KE5", //40
  "IDU:WH-ADC0916H9E8, ODU:WH-UX16HE8", //41
  "IDU:WH-ADC0912H9E8, ODU:WH-UX12HE8", //42
  "WH-MXC16J9E8", //43
  "WH-MXC12J6E5", //44
  "IDU:WH-SQC09H3E8, ODU:WH-UQ09HE8", //45
  "IDU:WH-ADC0309K3E5 ODU:WH-UDZ09KE5", //46
  "IDU:WH-ADC0916H9E8, ODU:WH-UX12HE8", //47
  "IDU:WH-SDC0509L3E5 ODU:WH-WDG07LE5", //48
  "IDU:WH-SXC09H3E5, ODU:WH-UX09HE5", //49
  "IDU:WH-SXC12H9E8, ODU:WH-UX12HE8", //50
};

static const byte knownModels[sizeof(Model) / sizeof(Model[0])][10] PROGMEM = { //stores the bytes #129 to #138 of known models in the same order as the const above
  0xE2, 0xCF, 0x0B, 0x13, 0x33, 0x32, 0xD1, 0x0C, 0x16, 0x33, //0
  0xE2, 0xCF, 0x0B, 0x14, 0x33, 0x42, 0xD1, 0x0B, 0x17, 0x33, //1
  0xE2, 0xCF, 0x0D, 0x77, 0x09, 0x12, 0xD0, 0x0B, 0x05, 0x11, //2
  0xE2, 0xCF, 0x0C, 0x88, 0x05, 0x12, 0xD0, 0x0B, 0x97, 0x05, //3
  0xE2, 0xCF, 0x0D, 0x85, 0x05, 0x12, 0xD0, 0x0C, 0x94, 0x05, //4
  0xE2, 0xCF, 0x0D, 0x86, 0x05, 0x12, 0xD0, 0x0C, 0x95, 0x05, //5
  0xE2, 0xCF, 0x0D, 0x87, 0x05, 0x12, 0xD0, 0x0C, 0x96, 0x05, //6
  0xE2, 0xCE, 0x0D, 0x71, 0x81, 0x72, 0xCE, 0x0C, 0x92, 0x81, //7
  0x62, 0xD2, 0x0B, 0x43, 0x54, 0x42, 0xD2, 0x0B, 0x72, 0x66, //8
  0xC2, 0xD3, 0x0B, 0x33, 0x65, 0xB2, 0xD3, 0x0B, 0x94, 0x65, //9
  0xE2, 0xCF, 0x0B, 0x15, 0x33, 0x42, 0xD1, 0x0B, 0x18, 0x33, //10
  0xE2, 0xCF, 0x0B, 0x41, 0x34, 0x82, 0xD1, 0x0B, 0x31, 0x35, //11
  0x62, 0xD2, 0x0B, 0x45, 0x54, 0x42, 0xD2, 0x0B, 0x47, 0x55, //12
  0xE2, 0xCF, 0x0C, 0x74, 0x09, 0x12, 0xD0, 0x0D, 0x95, 0x05, //13
  0xE2, 0xCF, 0x0B, 0x82, 0x05, 0x12, 0xD0, 0x0C, 0x91, 0x05, //14
  0xE2, 0xCF, 0x0C, 0x55, 0x14, 0x12, 0xD0, 0x0B, 0x15, 0x08, //15
  0xE2, 0xCF, 0x0C, 0x43, 0x00, 0x12, 0xD0, 0x0B, 0x15, 0x08, //16
  0x62, 0xD2, 0x0B, 0x45, 0x54, 0x32, 0xD2, 0x0C, 0x45, 0x55, //17
  0x62, 0xD2, 0x0B, 0x43, 0x54, 0x42, 0xD2, 0x0C, 0x46, 0x55, //18
  0xE2, 0xCF, 0x0C, 0x54, 0x14, 0x12, 0xD0, 0x0B, 0x14, 0x08, //19
  0xC2, 0xD3, 0x0B, 0x34, 0x65, 0xB2, 0xD3, 0x0B, 0x95, 0x65, //20
  0xC2, 0xD3, 0x0B, 0x35, 0x65, 0xB2, 0xD3, 0x0B, 0x96, 0x65, //21
  0x62, 0xD2, 0x0B, 0x41, 0x54, 0x32, 0xD2, 0x0C, 0x45, 0x55, //22
  0x32, 0xD4, 0x0B, 0x87, 0x84, 0x73, 0x90, 0x0C, 0x84, 0x84, //23
  0x32, 0xD4, 0x0B, 0x88, 0x84, 0x73, 0x90, 0x0C, 0x85, 0x84, //24
  0xE2, 0xCF, 0x0B, 0x75, 0x09, 0x12, 0xD0, 0x0C, 0x06, 0x11, //25
  0x42, 0xD4, 0x0B, 0x83, 0x71, 0x42, 0xD2, 0x0C, 0x46, 0x55, //26
  0xC2, 0xD3, 0x0C, 0x34, 0x65, 0xB2, 0xD3, 0x0B, 0x95, 0x65, //27
  0xC2, 0xD3, 0x0C, 0x33, 0x65, 0xB2, 0xD3, 0x0B, 0x94, 0x65, //28
  0xE2, 0xCF, 0x0B, 0x83, 0x05, 0x12, 0xD0, 0x0D, 0x92, 0x05, //29
  0xE2, 0xCF, 0x0C, 0x78, 0x09, 0x12, 0xD0, 0x0B, 0x06, 0x11, //30
  0xC2, 0xD3, 0x0C, 0x35, 0x65, 0xB2, 0xD3, 0x0B, 0x96, 0x65, //31
  0x32, 0xD4, 0x0B, 0x99, 0x77, 0x62, 0x90, 0x0B, 0x01, 0x78, //32
  0x42, 0xD4, 0x0B, 0x15, 0x76, 0x12, 0xD0, 0x0B, 0x10, 0x11, //33
  0xE2, 0xD5, 0x0C, 0x29, 0x99, 0x83, 0x92, 0x0C, 0x28, 0x98, //34
  0xE2, 0xCF, 0x0D, 0x85, 0x05, 0x12, 0xD0, 0x0E, 0x94, 0x05, //35
  0xE2, 0xD5, 0x0D, 0x36, 0x99, 0x02, 0xD6, 0x0F, 0x67, 0x95, //36
  0xE2, 0xD5, 0x0B, 0x08, 0x95, 0x02, 0xD6, 0x0E, 0x66, 0x95, //37
  0xE2, 0xD5, 0x0B, 0x34, 0x99, 0x83, 0x92, 0x0C, 0x29, 0x98, //38
  0xE2, 0xCF, 0x0C, 0x89, 0x05, 0x12, 0xD0, 0x0C, 0x98, 0x05, //39
  0xE2, 0xD5, 0x0B, 0x08, 0x95, 0x02, 0xD6, 0x0E, 0x67, 0x95, //40
  0xE2, 0xCF, 0x0C, 0x74, 0x09, 0x12, 0xD0, 0x0C, 0x96, 0x05, //41
  0xE2, 0xCF, 0x0C, 0x74, 0x09, 0x12, 0xD0, 0x0E, 0x95, 0x05, //42
  0x32, 0xD4, 0x0B, 0x89, 0x84, 0x73, 0x90, 0x0C, 0x86, 0x84, //43
  0x32, 0xD4, 0x0B, 0x00, 0x78, 0x62, 0x90, 0x0B, 0x02, 0x78, //44
  0xE2, 0xCF, 0x0B, 0x82, 0x05, 0x12, 0xD0, 0x0D, 0x91, 0x05, //45  
  0xE2, 0xD5, 0x0D, 0x99, 0x94, 0x02, 0xD6, 0x0D, 0x68, 0x95, //46
  0xE2, 0xCF, 0x0C, 0x74, 0x09, 0x12, 0xD0, 0x0C, 0x95, 0x05, //47
  0xE2, 0xD5, 0x0B, 0x34, 0x99, 0x83, 0x92, 0x0C, 0x28, 0x98, //48
  0xE2, 0xCF, 0x0D, 0x77, 0x09, 0x12, 0xD0, 0x0C, 0x05, 0x11, //49
  0xE2, 0xCF, 0x0D, 0x86, 0x05, 0x12, 0xD0, 0x0E, 0x95, 0x05, //50
};

#define NUMBER_OF_TOPICS 127 //last topic number + 1
#define NUMBER_OF_TOPICS_EXTRA 6 //last topic number + 1
#define NUMBER_OF_OPT_TOPICS 7 //last topic number + 1
#define MAX_TOPIC_LEN 42 // max length + 1

static const char optTopics[][20] PROGMEM = {
  "Z1_Water_Pump", // OPT0
  "Z1_Mixing_Valve", // OPT1
  "Z2_Water_Pump", // OPT2
  "Z2_Mixing_Valve", // OPT3
  "Pool_Water_Pump", // OPT4
  "Solar_Water_Pump", // OPT5
  "Alarm_State", // OPT6
};

static const char xtopics[][MAX_TOPIC_LEN] PROGMEM = {
  "Heat_Power_Consumption_Extra", //XTOP0
  "Cool_Power_Consumption_Extra", //XTOP1
  "DHW_Power_Consumption_Extra", //XTOP2
  "Heat_Power_Production_Extra",  //XTOP3
  "Cool_Power_Production_Extra",  //XTOP4
  "DHW_Power_Production_Extra",  //XTOP5
};

static const byte xtopicBytes[] PROGMEM = { //can store the index as byte (8-bit unsigned humber) as there aren't more then 255 bytes (actually only 203 bytes) to decode
  14,      //XTOP0
  16,      //XTOP1
  18,      //XTOP2
  20,      //XTOP3
  22,      //XTOP4
  24,      //XTOP5
};

static const char topics[][MAX_TOPIC_LEN] PROGMEM = {
  "Heatpump_State",          //TOP0
  "Pump_Flow",               //TOP1
  "Force_DHW_State",         //TOP2
  "Quiet_Mode_Schedule",     //TOP3
  "Operating_Mode_State",    //TOP4
  "Main_Inlet_Temp",         //TOP5
  "Main_Outlet_Temp",        //TOP6
  "Main_Target_Temp",        //TOP7
  "Compressor_Freq",         //TOP8
  "DHW_Target_Temp",         //TOP9
  "DHW_Temp",                //TOP10
  "Operations_Hours",        //TOP11
  "Operations_Counter",      //TOP12
  "Main_Schedule_State",     //TOP13
  "Outside_Temp",            //TOP14
  "Heat_Power_Production",  //TOP15
  "Heat_Power_Consumption", //TOP16
  "Powerful_Mode_Time",      //TOP17
  "Quiet_Mode_Level",        //TOP18
  "Holiday_Mode_State",      //TOP19
  "ThreeWay_Valve_State",    //TOP20
  "Outside_Pipe_Temp",       //TOP21
  "DHW_Heat_Delta",          //TOP22
  "Heat_Delta",              //TOP23
  "Cool_Delta",              //TOP24
  "DHW_Holiday_Shift_Temp",  //TOP25
  "Defrosting_State",        //TOP26
  "Z1_Heat_Request_Temp",    //TOP27
  "Z1_Cool_Request_Temp",    //TOP28
  "Z1_Heat_Curve_Target_High_Temp",      //TOP29
  "Z1_Heat_Curve_Target_Low_Temp",       //TOP30
  "Z1_Heat_Curve_Outside_High_Temp",     //TOP31
  "Z1_Heat_Curve_Outside_Low_Temp",      //TOP32
  "Room_Thermostat_Temp",    //TOP33
  "Z2_Heat_Request_Temp",    //TOP34
  "Z2_Cool_Request_Temp",    //TOP35
  "Z1_Water_Temp",           //TOP36
  "Z2_Water_Temp",           //TOP37
  "Cool_Power_Production",  //TOP38
  "Cool_Power_Consumption", //TOP39
  "DHW_Power_Production",   //TOP40
  "DHW_Power_Consumption",  //TOP41
  "Z1_Water_Target_Temp",    //TOP42
  "Z2_Water_Target_Temp",    //TOP43
  "Error",                   //TOP44
  "Room_Holiday_Shift_Temp", //TOP45
  "Buffer_Temp",             //TOP46
  "Solar_Temp",              //TOP47
  "Pool_Temp",               //TOP48
  "Main_Hex_Outlet_Temp",    //TOP49
  "Discharge_Temp",          //TOP50
  "Inside_Pipe_Temp",        //TOP51
  "Defrost_Temp",            //TOP52
  "Eva_Outlet_Temp",         //TOP53
  "Bypass_Outlet_Temp",      //TOP54
  "Ipm_Temp",                //TOP55
  "Z1_Temp",                 //TOP56
  "Z2_Temp",                 //TOP57
  "DHW_Heater_State",        //TOP58
  "Room_Heater_State",       //TOP59
  "Internal_Heater_State",   //TOP60
  "External_Heater_State",   //TOP61
  "Fan1_Motor_Speed",        //TOP62
  "Fan2_Motor_Speed",        //TOP63
  "High_Pressure",           //TOP64
  "Pump_Speed",              //TOP65
  "Low_Pressure",            //TOP66
  "Compressor_Current",      //TOP67
  "Force_Heater_State",      //TOP68
  "Sterilization_State",     //TOP69
  "Sterilization_Temp",      //TOP70
  "Sterilization_Max_Time",  //TOP71
  "Z1_Cool_Curve_Target_High_Temp",      //TOP72
  "Z1_Cool_Curve_Target_Low_Temp",       //TOP73
  "Z1_Cool_Curve_Outside_High_Temp",     //TOP74
  "Z1_Cool_Curve_Outside_Low_Temp",      //TOP75
  "Heating_Mode",            //TOP76
  "Heating_Off_Outdoor_Temp",//TOP77
  "Heater_On_Outdoor_Temp",  //TOP78
  "Heat_To_Cool_Temp",       //TOP79
  "Cool_To_Heat_Temp",       //TOP80
  "Cooling_Mode",            //TOP81
  "Z2_Heat_Curve_Target_High_Temp",      //TOP82
  "Z2_Heat_Curve_Target_Low_Temp",       //TOP83
  "Z2_Heat_Curve_Outside_High_Temp",     //TOP84
  "Z2_Heat_Curve_Outside_Low_Temp",      //TOP85
  "Z2_Cool_Curve_Target_High_Temp",      //TOP86
  "Z2_Cool_Curve_Target_Low_Temp",       //TOP87
  "Z2_Cool_Curve_Outside_High_Temp",     //TOP88
  "Z2_Cool_Curve_Outside_Low_Temp",      //TOP89
  "Room_Heater_Operations_Hours",        //TOP90
  "DHW_Heater_Operations_Hours",         //TOP91
  "Heat_Pump_Model",         //TOP92
  "Pump_Duty",               //TOP93
  "Zones_State",             //TOP94
  "Max_Pump_Duty",           //TOP95
  "Heater_Delay_Time",       //TOP96
  "Heater_Start_Delta",      //TOP97
  "Heater_Stop_Delta",       //TOP98
  "Buffer_Installed",        //TOP99
  "DHW_Installed",           //TOP100
  "Solar_Mode",              //TOP101
  "Solar_On_Delta",          //TOP102
  "Solar_Off_Delta",         //TOP103
  "Solar_Frost_Protection",  //TOP104
  "Solar_High_Limit",        //TOP105
  "Pump_Flowrate_Mode",      //TOP106
  "Liquid_Type",             //TOP107
  "Alt_External_Sensor",     //TOP108
  "Anti_Freeze_Mode",        //TOP109
  "Optional_PCB",            //TOP110
  "Z1_Sensor_Settings",      //TOP111
  "Z2_Sensor_Settings",      //TOP112
  "Buffer_Tank_Delta",       //TOP113
  "External_Pad_Heater",     //TOP114
  "Water_Pressure",          //TOP115
  "Second_Inlet_Temp",       //TOP116
  "Economizer_Outlet_Temp",  //TOP117
  "Second_Room_Thermostat_Temp",//TOP118
  "Bivalent_Heating_Start_Temperature",//TOP119
  "Bivalent_Heating_Parallel_Adv_Starttemp",//TOP120
  "Bivalent_Heating_Parallel_Adv_Stoptemp",//TOP121
  "Bivalent_Heating_Parallel_Adv_Start_Delay",//TOP122
  "Bivalent_Heating_Parallel_Adv_Stop_Delay",//TOP123
  "Bivalent_Heating_Setting",//TOP124
  "2_Zone_mixing_valve_1_opening",//TOP125
  "2_Zone_mixing_valve_2_opening",//TOP126
};

static const byte topicBytes[] PROGMEM = { //can store the index as byte (8-bit unsigned humber) as there aren't more then 255 bytes (actually only 203 bytes) to decode
  4,      //TOP0
  0,      //TOP1
  4,      //TOP2
  7,      //TOP3
  6,      //TOP4
  143,    //TOP5
  144,    //TOP6
  153,    //TOP7
  166,    //TOP8
  42,     //TOP9
  141,    //TOP10
  0,      //TOP11
  0,      //TOP12
  5,      //TOP13
  142,    //TOP14
  194,    //TOP15
  193,    //TOP16
  7,      //TOP17
  7,      //TOP18
  5,      //TOP19
  111,    //TOP20
  158,    //TOP21
  99,     //TOP22
  84,     //TOP23
  94,     //TOP24
  44,     //TOP25
  111,    //TOP26
  38,     //TOP27
  39,     //TOP28
  75,     //TOP29
  76,     //TOP30
  78,     //TOP31
  77,     //TOP32
  156,    //TOP33
  40,     //TOP34
  41,     //TOP35
  145,    //TOP36
  146,    //TOP37
  196,    //TOP38
  195,    //TOP39
  198,    //TOP40
  197,    //TOP41
  147,    //TOP42
  148,    //TOP43
  0,      //TOP44
  43,     //TOP45
  149,    //TOP46
  150,    //TOP47
  151,    //TOP48
  154,    //TOP49
  155,    //TOP50
  157,    //TOP51
  159,    //TOP52
  160,    //TOP53
  161,    //TOP54
  162,    //TOP55
  139,    //TOP56
  140,    //TOP57
  9,      //TOP58
  9,      //TOP59
  112,    //TOP60
  112,    //TOP61
  173,    //TOP62
  174,    //TOP63
  163,    //TOP64
  171,    //TOP65
  164,    //TOP66
  165,    //TOP67
  5,      //TOP68
  117,    //TOP69
  100,    //TOP70
  101,    //TOP71
  86,     //TOP72
  87,     //TOP73
  89,     //TOP74
  88,     //TOP75
  28,     //TOP76
  83,     //TOP77
  85,     //TOP78
  95,     //TOP79
  96,     //TOP80
  28,     //TOP81
  79,     //TOP82
  80,     //TOP83
  82,     //TOP84
  81,     //TOP85
  90,     //TOP86
  91,     //TOP87
  93,     //TOP88
  92,     //TOP89
  0,      //TOP90
  0,      //TOP91
  0,      //TOP92
  172,    //TOP93
  6,      //TOP94
  45,     //TOP95
  104,    //TOP96
  105,    //TOP97
  106,    //TOP98
  24,     //TOP99
  24,     //TOP100
  24,     //TOP101
  61,     //TOP102
  62,     //TOP103
  63,     //TOP104
  64,     //TOP105
  29,     //TOP106
  20,     //TOP107
  20,     //TOP108
  20,     //TOP109
  20,     //TOP110
  22,     //TOP111
  22,     //TOP112
  59,     //TOP113
  25,     //TOP114
  125,    //TOP115
  126,    //TOP116
  127,    //TOP117
  128,    //TOP118
  65,    //TOP119
  66,    //TOP120
  68,    //TOP121
  67,    //TOP122
  69,    //TOP123
  26,    //TOP124
  177,    //TOP125
  178,    //TOP126
};


typedef String (*xtopicFP)(char*, byte);
static const xtopicFP xtopicFunctions[] PROGMEM = {
  getUintt16,         //XTOP0
  getUintt16,         //XTOP1
  getUintt16,         //XTOP2
  getUintt16,         //XTOP3
  getUintt16,         //XTOP4
  getUintt16,         //XTOP5
};

typedef String (*topicFP)(byte);
static const topicFP topicFunctions[] PROGMEM = {
  getBit7and8,         //TOP0
  unknown,             //TOP1
  getBit1and2,         //TOP2
  getBit1and2,         //TOP3
  getOpMode,           //TOP4
  getIntMinus128,      //TOP5
  getIntMinus128,      //TOP6
  getIntMinus128,      //TOP7
  getIntMinus1,        //TOP8
  getIntMinus128,      //TOP9
  getIntMinus128,      //TOP10
  unknown,             //TOP11
  unknown,             //TOP12
  getBit1and2,         //TOP13
  getIntMinus128,      //TOP14
  getPower,            //TOP15
  getPower,            //TOP16
  getRight3bits,       //TOP17
  getBit3and4and5,     //TOP18
  getBit3and4,         //TOP19
  getBit7and8,         //TOP20
  getIntMinus128,      //TOP21
  getIntMinus128,      //TOP22
  getIntMinus128,      //TOP23
  getIntMinus128,      //TOP24
  getIntMinus128,      //TOP25
  getBit5and6,         //TOP26
  getIntMinus128,      //TOP27
  getIntMinus128,      //TOP28
  getIntMinus128,      //TOP29
  getIntMinus128,      //TOP30
  getIntMinus128,      //TOP31
  getIntMinus128,      //TOP32
  getIntMinus128,      //TOP33
  getIntMinus128,      //TOP34
  getIntMinus128,      //TOP35
  getIntMinus128,      //TOP36
  getIntMinus128,      //TOP37
  getPower,            //TOP38
  getPower,            //TOP39
  getPower,            //TOP40
  getPower,            //TOP41
  getIntMinus128,      //TOP42
  getIntMinus128,      //TOP43
  unknown,             //TOP44
  getIntMinus128,      //TOP45
  getIntMinus128,      //TOP46
  getIntMinus128,      //TOP47
  getIntMinus128,      //TOP48
  getIntMinus128,      //TOP49
  getIntMinus128,      //TOP50
  getIntMinus128,      //TOP51
  getIntMinus128,      //TOP52
  getIntMinus128,      //TOP53
  getIntMinus128,      //TOP54
  getIntMinus128,      //TOP55
  getIntMinus128,      //TOP56
  getIntMinus128,      //TOP57
  getBit5and6,         //TOP58
  getBit7and8,         //TOP59
  getBit7and8,         //TOP60
  getBit5and6,         //TOP61
  getIntMinus1Times10, //TOP62
  getIntMinus1Times10, //TOP63
  getIntMinus1Div5,    //TOP64
  getIntMinus1Times50, //TOP65
  getIntMinus1Times50, //TOP66
  getIntMinus1Div5,    //TOP67
  getBit5and6,         //TOP68
  getBit5and6,         //TOP69
  getIntMinus128,      //TOP70
  getIntMinus1,        //TOP71
  getIntMinus128,      //TOP72
  getIntMinus128,      //TOP73
  getIntMinus128,      //TOP74
  getIntMinus128,      //TOP75
  getBit7and8,         //TOP76
  getIntMinus128,      //TOP77
  getIntMinus128,      //TOP78
  getIntMinus128,      //TOP79
  getIntMinus128,      //TOP80
  getBit5and6,         //TOP81
  getIntMinus128,      //TOP82
  getIntMinus128,      //TOP83
  getIntMinus128,      //TOP84
  getIntMinus128,      //TOP85
  getIntMinus128,      //TOP86
  getIntMinus128,      //TOP87
  getIntMinus128,      //TOP88
  getIntMinus128,      //TOP89
  unknown,             //TOP90
  unknown,             //TOP91
  unknown,			       //TOP92
  getIntMinus1,        //TOP93
  getBit1and2,         //TOP94
  getIntMinus1,        //TOP95
  getIntMinus1,        //TOP96
  getIntMinus128,      //TOP97
  getIntMinus128,      //TOP98
  getBit5and6,         //TOP99
  getBit7and8,         //TOP100
  getBit3and4,         //TOP101
  getIntMinus128,      //TOP102
  getIntMinus128,      //TOP103
  getIntMinus128,      //TOP104
  getIntMinus128,      //TOP105
  getBit3and4,         //TOP106
  getBit1,             //TOP107
  getBit3and4,         //TOP108
  getBit5and6,         //TOP109
  getBit7and8,         //TOP110  
  getSecondByte,       //TOP111
  getFirstByte,        //TOP112 
  getIntMinus128,      //TOP113
  getBit3and4,         //TOP114
  getIntMinus1Div50,   //TOP115
  getIntMinus128,      //TOP116
  getIntMinus128,      //TOP117
  getIntMinus128,      //TOP118
  getIntMinus128,      //TOP119
  getIntMinus128,      //TOP120
  getIntMinus128,      //TOP121
  getIntMinus1,      //TOP122
  getIntMinus1,      //TOP123
  getIntMinus1,      //TOP124
  getIntMinus1,      //TOP125
  getIntMinus1,      //TOP126
};

static const char *DisabledEnabled[] PROGMEM = {"2", "Disabled", "Enabled"};
static const char *BlockedFree[] PROGMEM = {"2", "Blocked", "Free"};
static const char *OffOn[] PROGMEM = {"2", "Off", "On"};
static const char *InactiveActive[] PROGMEM = {"2", "Inactive", "Active"};
static const char *PumpFlowRateMode[] PROGMEM = {"2", "DeltaT", "Max flow"};
static const char *HolidayState[] PROGMEM = {"3", "Off", "Scheduled", "Active"};
static const char *OpModeDesc[] PROGMEM = {"9", "Heat", "Cool", "Auto(heat)", "DHW", "Heat+DHW", "Cool+DHW", "Auto(heat)+DHW", "Auto(cool)", "Auto(cool)+DHW"};
static const char *Powerfulmode[] PROGMEM = {"4", "Off", "30min", "60min", "90min"};
static const char *Quietmode[] PROGMEM = {"4", "Off", "Level 1", "Level 2", "Level 3"};
static const char *Valve[] PROGMEM = {"2", "Room", "DHW"};
static const char *MixingValve[] PROGMEM = {"4", "Off", "Increase","Nothing","Decrease"};
static const char *LitersPerMin[] PROGMEM = {"0", "l/min"};
static const char *RotationsPerMin[] PROGMEM = {"0", "r/min"};
static const char *Bar[] PROGMEM = {"0", "Bar"};
static const char *Pressure[] PROGMEM = {"0", "Kgf/cm2"};
static const char *Celsius[] PROGMEM = {"0", "°C"};
static const char *Kelvin[] PROGMEM = {"0", "K"};
static const char *Hertz[] PROGMEM = {"0", "Hz"};
static const char *Counter[] PROGMEM = {"0", "count"};
static const char *Hours[] PROGMEM = {"0", "hours"};
static const char *Watt[] PROGMEM = {"0", "Watt"};
static const char *ErrorState[] PROGMEM = {"0", "Error"};
static const char *Ampere[] PROGMEM = {"0", "Ampere"};
static const char *Minutes[] PROGMEM = {"0", "Minutes"};
static const char *Duty[] PROGMEM = {"0", "Duty"};
static const char *ZonesState[] PROGMEM = {"3", "Zone1 active", "Zone2 active", "Zone1 and zone2 active"};
static const char *HeatCoolModeDesc[] PROGMEM = {"2", "Comp. Curve", "Direct"};
static const char *SolarModeDesc[] PROGMEM = {"3", "Disabled", "Buffer", "DHW"};
static const char *ZonesSensorType[] PROGMEM = {"4", "Water Temperature", "External Thermostat", "Internal Thermostat", "Thermistor"};
static const char *LiquidType[] PROGMEM = {"2", "Water", "Glycol"};
static const char *ExtPadHeaterType[] PROGMEM = {"3", "Disabled", "Type-A","Type-B"};
static const char *Bivalent[] PROGMEM = {"6", "Off", "Alternativ", "A-Off", "Parallel", "P-Off", "Parallel Advanced"};


static const char **opttopicDescription[] PROGMEM = {
  OffOn,          //OPT0
  MixingValve,    //OPT1
  OffOn,          //OPT2
  MixingValve,    //OPT3
  OffOn,          //OPT4
  OffOn,          //OPT5
  OffOn,          //OPT6
};

static const char **xtopicDescription[] PROGMEM = {
  Watt,           //XTOP0
  Watt,           //XTOP1
  Watt,           //XTOP2
  Watt,           //XTOP3
  Watt,           //XTOP4
  Watt,           //XTOP5
};

static const char **topicDescription[] PROGMEM = {
  OffOn,           //TOP0
  LitersPerMin,    //TOP1
  DisabledEnabled, //TOP2
  DisabledEnabled, //TOP3
  OpModeDesc,      //TOP4
  Celsius,         //TOP5
  Celsius,         //TOP6
  Celsius,         //TOP7
  Hertz,           //TOP8
  Celsius,         //TOP9
  Celsius,         //TOP10
  Hours,           //TOP11
  Counter,         //TOP12
  DisabledEnabled, //TOP13
  Celsius,         //TOP14
  Watt,            //TOP15
  Watt,            //TOP16
  Powerfulmode,    //TOP17
  Quietmode,       //TOP18
  HolidayState,    //TOP19
  Valve,           //TOP20
  Celsius,         //TOP21
  Kelvin,          //TOP22
  Kelvin,          //TOP23
  Kelvin,          //TOP24
  Kelvin,          //TOP25
  DisabledEnabled, //TOP26
  Celsius,         //TOP27
  Celsius,         //TOP28
  Celsius,         //TOP29
  Celsius,         //TOP30
  Celsius,         //TOP31
  Celsius,         //TOP32
  Celsius,         //TOP33
  Celsius,         //TOP34
  Celsius,         //TOP35
  Celsius,         //TOP36
  Celsius,         //TOP37
  Watt,            //TOP38
  Watt,            //TOP39
  Watt,            //TOP40
  Watt,            //TOP41
  Celsius,         //TOP42
  Celsius,         //TOP43
  ErrorState,      //TOP44
  Kelvin,          //TOP45
  Celsius,         //TOP46
  Celsius,         //TOP47
  Celsius,         //TOP48
  Celsius,         //TOP49
  Celsius,         //TOP50
  Celsius,         //TOP51
  Celsius,         //TOP52
  Celsius,         //TOP53
  Celsius,         //TOP54
  Celsius,         //TOP55
  Celsius,         //TOP56
  Celsius,         //TOP57
  BlockedFree,     //TOP58
  BlockedFree,     //TOP59
  InactiveActive,  //TOP60
  InactiveActive,  //TOP61
  RotationsPerMin, //TOP62
  RotationsPerMin, //TOP63
  Pressure,        //TOP64
  RotationsPerMin, //TOP65
  Pressure,        //TOP66
  Ampere,          //TOP67
  InactiveActive,  //TOP68
  InactiveActive,  //TOP69
  Celsius,         //TOP70
  Minutes,         //TOP71
  Celsius,         //TOP72
  Celsius,         //TOP73
  Celsius,         //TOP74
  Celsius,         //TOP75
  HeatCoolModeDesc,//TOP76
  Celsius,         //TOP77
  Celsius,         //TOP78
  Celsius,         //TOP79
  Celsius,         //TOP80
  HeatCoolModeDesc,//TOP81
  Celsius,         //TOP82
  Celsius,         //TOP83
  Celsius,         //TOP84
  Celsius,         //TOP85
  Celsius,         //TOP86
  Celsius,         //TOP87
  Celsius,         //TOP88
  Celsius,         //TOP89
  Hours,           //TOP90
  Hours,           //TOP91
  Model,           //TOP92
  Duty,            //TOP93
  ZonesState,      //TOP94
  Duty,            //TOP95
  Minutes,         //TOP96
  Kelvin,          //TOP97
  Kelvin,          //TOP98
  DisabledEnabled, //TOP99
  DisabledEnabled, //TOP100
  SolarModeDesc,   //TOP101
  Kelvin,          //TOP102
  Kelvin,          //TOP103
  Celsius,         //TOP104
  Celsius,         //TOP105
  PumpFlowRateMode,//TOP106
  LiquidType,      //TOP107
  DisabledEnabled, //TOP108
  DisabledEnabled, //TOP109
  DisabledEnabled, //TOP110
  ZonesSensorType, //TOP111
  ZonesSensorType, //TOP112
  Kelvin,          //TOP113
  ExtPadHeaterType,//TOP114
  Bar,             //TOP115
  Celsius,         //TOP116
  Celsius,         //TOP117
  Celsius,         //TOP118
  Celsius,         //TOP119
  Celsius,         //TOP120
  Celsius,         //TOP121
  Minutes,         //TOP122
  Minutes,         //TOP123
  Bivalent,        //TOP124
  Counter,         //TOP125
  Counter,         //TOP126
};
