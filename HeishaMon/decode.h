#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define UPDATEALLTIME 300000 // how often all data is cleared and so resend to mqtt
#define MQTT_RETAIN_VALUES 1

void decode_heatpump_data(char* data, DynamicJsonDocument &actData, PubSubClient &mqtt_client, void (*log_message)(char*));

String unknown(byte input);
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
String getIntMinus1Times10(byte input);
String getIntMinus1Times50(byte input);
String getOpMode(byte input);
String getEnergy(byte input);

static const String topics[] = {
            "Power_State",             //TOP0
            "Pump_Flow",               //TOP1
            "ForceDHW_State",          //TOP2
            "Quietmode_Schedule",      //TOP3
            "OpMode_State",            //TOP4
            "Water_Inlet_Temp",        //TOP5
            "Water_Outlet_Temp",       //TOP6
            "Water_Target_Temp",       //TOP7
            "Compressor_Freq",         //TOP8
            "Tank_Target_Temp",        //TOP9
            "Tank_Temp",               //TOP10
            "Operations_Hours",        //TOP11
            "Operations_Counter",      //TOP12
            "MainSchedule_State",      //TOP13
            "Outside_Temp",            //TOP14
            "Heat_Energy_Production",  //TOP15
            "Heat_Energy_Consumption", //TOP16
            "Powerfullmode_Time",      //TOP17
            "Quietmode_Level",         //TOP18
            "Holidaymode_State",       //TOP19
            "Valve_State",             //TOP20
            "Outside_Pipe_Temp",       //TOP21
            "Tank_Heat_Delta",         //TOP22
            "Heat_Delta",              //TOP23
            "Cool_Delta",              //TOP24
            "ShiftTank_Temp",          //TOP25
            "Defrosting_State",        //TOP26
            "Z1_HeatShift_Temp",       //TOP27
            "Z1_CoolShift_Temp",       //TOP28
            "HCurve_OutHighTemp",      //TOP29
            "HCurve_OutLowTemp",       //TOP30
            "HCurve_OutsHighTemp",     //TOP31
            "HCurve_OutsLowTemp",      //TOP32
            "Roomthermostat_Temp",     //TOP33
            "Z2_HeatShift_Temp",       //TOP34
            "Z2_CoolShift_Temp",       //TOP35
            "Z1_Water_Temp",           //TOP36
            "Z2_Water_Temp",           //TOP37
            "Cool_Energy_Production",  //TOP38
            "Cool_Energy_Consumption", //TOP39
            "DHW_Energy_Production",   //TOP40
            "DHW_Energy_Consumption",  //TOP41
            "Z1_Water_Target_Temp",    //TOP42
            "Z2_Water_Target_Temp",    //TOP43
            "Error",                   //TOP44
            "ShiftHoliday_Temp",       //TOP45
            "Buffer_Temp",             //TOP46
            "Solar_Temp",              //TOP47
            "Pool_Temp",               //TOP48
            "Water_Hex_Outlet_Temp",   //TOP49
            "Discharge_Temp",          //TOP50
            "Inside_Pipe_Temp",        //TOP51
            "Defrost_Temp",            //TOP52
            "EvaOutlet_Temp",          //TOP53
            "BypassOutlet_Temp",       //TOP54
            "Ipm_Temp",                //TOP55
            "Z1_Temp",                 //TOP56
            "Z2_Temp",                 //TOP57
            "TankHeater_State",        //TOP58
            "WaterHeater_State",       //TOP59
            "InternalHeater_State",    //TOP60
            "ExternalHeater_State",    //TOP61
            "Fan1Motor_Speed",         //TOP62
            "Fan2Motor_Speed",         //TOP63
            "High_Pressure",           //TOP64
            "Pump_Speed",              //TOP65
            "Low_Pressure",            //TOP66
            "Outdoor_Current",         //TOP67
            "ForceHeater_State",       //TOP68
};

static const unsigned int topicBytes[] = {
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
};        

typedef String (*topicFP)(byte);

static const topicFP topicFunctions[] = {
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
    getEnergy,           //TOP15
    getEnergy,           //TOP16
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
    getEnergy,           //TOP38
    getEnergy,           //TOP39
    getEnergy,           //TOP40
    getEnergy,           //TOP41
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
    getIntMinus1,        //TOP66
    getIntMinus1Div5,    //TOP67
    getBit5and6,         //TOP68
};

static const char *DisabledEnabled[] = {"Disabled","Enabled"};
static const char *OffOn[] = {"Off","On"};
static const char *InactiveActive[] = {"Inactive","Active"};
static const char *OpModeDesc[] = {"Heat","Cool","Auto","Tank","Heat+Tank","Cool+Tank","Auto+Tank"};
static const char *Powerfullmode[] = {"Off","30min","60min","90min"};
static const char *Quietmode[] = {"Off","Level 1","Level 2","Level 3"};
static const char *Valve[] = {"Room","DHW"};
static const char *LitersPerMin[] = {"value","l/min"};
static const char *RotationsPerMin[] = {"value","r/min"};
static const char *Pressure[] = {"value","Kgf/cm2"};
static const char *Celsius[] = {"value","&deg;C"};
static const char *Kelvin[] = {"value","&deg;K"};
static const char *Hertz[] = {"value","Hz"};
static const char *Counter[] = {"value","count"};
static const char *Hours[] = {"value","hours"};
static const char *Watt[] = {"value","Watt"};
static const char *ErrorState[] = {"value","Error"};
static const char *Ampere[] = {"value","Ampere"};
static const char **topicDescription[] = {
    OffOn,                 //TOP0
    LitersPerMin,          //TOP1
    DisabledEnabled,       //TOP2
    DisabledEnabled,       //TOP3
    OpModeDesc,            //TOP4
    Celsius,               //TOP5
    Celsius,               //TOP6
    Celsius,               //TOP7
    Hertz,                 //TOP8
    Celsius,               //TOP9
    Celsius,               //TOP10
    Hours,                 //TOP11    
    Counter,               //TOP12    
    DisabledEnabled,       //TOP13    
    Celsius,               //TOP14    
    Watt,                  //TOP15    
    Watt,                  //TOP16    
    Powerfullmode,         //TOP17    
    Quietmode,             //TOP18    
    DisabledEnabled,       //TOP19    
    Valve,                 //TOP20
    Celsius,               //TOP21    
    Kelvin,                //TOP22    
    Kelvin,                //TOP23    
    Kelvin,                //TOP24    
    Kelvin,                //TOP25    
    DisabledEnabled,       //TOP26    
    Celsius,               //TOP27    
    Celsius,               //TOP28    
    Celsius,               //TOP29    
    Celsius,               //TOP30
    Celsius,               //TOP31    
    Celsius,               //TOP32    
    Celsius,               //TOP33    
    Celsius,               //TOP34    
    Celsius,               //TOP35    
    Celsius,               //TOP36    
    Celsius,               //TOP37    
    Watt,                  //TOP38    
    Watt,                  //TOP39    
    Watt,                  //TOP40
    Watt,                  //TOP41    
    Celsius,               //TOP42    
    Celsius,               //TOP43    
    ErrorState,            //TOP44    
    Celsius,               //TOP45    
    Celsius,               //TOP46    
    Celsius,               //TOP47    
    Celsius,               //TOP48    
    Celsius,               //TOP49    
    Celsius,               //TOP50
    Celsius,               //TOP51    
    Celsius,               //TOP52    
    Celsius,               //TOP53    
    Celsius,               //TOP54    
    Celsius,               //TOP55    
    Celsius,               //TOP56    
    Celsius,               //TOP57    
    DisabledEnabled,       //TOP58    
    DisabledEnabled,       //TOP59    
    InactiveActive,        //TOP60
    InactiveActive,        //TOP61    
    RotationsPerMin,       //TOP62    
    RotationsPerMin,       //TOP63    
    Pressure,              //TOP64    
    RotationsPerMin,       //TOP65    
    Pressure,              //TOP66
    Ampere,                //TOP67
    InactiveActive,                //TOP68
};

 
