#include "decode.h"
#include "./private/decoder_private.h"
#include "commands.h"
#include "rules.h"
#include "math.h"

unsigned long lastalldatatime = 0;
unsigned long lastalloptdatatime = 0;

static void getBit1(uint8_t *input, uint8_t offset, decode_result_t *result);

static void getBit1and2(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getBit3and4(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getBit5and6(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getBit7and8(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getBit3and4and5(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getLeft5bits(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getRight3bits(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getIntMinus1(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getIntMinus128(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getIntMinus1Div5(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getIntMinus1Times10(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getIntMinus1Times50(uint8_t *input, uint8_t offset, decode_result_t *result);
static void unknown(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getOpMode(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getEnergy(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getPumpFlow(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getFirstByte(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getSecondByte(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getModel(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getErrorInfo(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getFractionalValue(uint8_t *input, uint8_t offset, decode_result_t *result);
static void getUint16Minus1(uint8_t *input, uint8_t offset, decode_result_t *result);

typedef void (*decoder_func_ptr_t)(uint8_t *data, uint8_t offset, decode_result_t *result);

typedef struct
{
  const char *const name;
  topic_description_t *description;
  uint8_t byte_offset;
  decoder_func_ptr_t decoder_function;
  filter_context_t filter_context;
} topic_t;

// clang-format off
static topic_t topic_configurations[] = {
  {
    .name = "Heatpump_State",                  // TOP0
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_OFF_ON],            // TOP0
    .byte_offset =   4,      //TOP0
    .decoder_function = getBit7and8,         //TOP0
    .filter_context = { 0 }
  },
  {
    .name = "Pump_Flow",                       // TOP1
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_LITERS_PER_MIN],     // TOP1
    .byte_offset =   169,      //TOP1
    .decoder_function = getPumpFlow,             //TOP1
    .filter_context = { 0 }
  },
  {
    .name = "Force_DHW_State",                 // TOP2
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP2
    .byte_offset =   4,      //TOP2
    .decoder_function = getBit1and2,         //TOP2
    .filter_context = { 0 }
  },
  {
    .name = "Quiet_Mode_Schedule",             // TOP3
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP3
    .byte_offset =   7,      //TOP3
    .decoder_function = getBit1and2,         //TOP3
    .filter_context = { 0 }
  },
  {
    .name = "Operating_Mode_State",            // TOP4
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_OP_MODE_DESC],       // TOP4
    .byte_offset =   6,      //TOP4
    .decoder_function = getOpMode,           //TOP4
    .filter_context = { 0 }
  },
  {
    .name = "Main_Inlet_Temp",                 // TOP5
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP5
    .byte_offset =   143,    //TOP5
    .decoder_function = getFractionalValue,      //TOP5
    .filter_context = { 0 }
  },
  {
    .name = "Main_Outlet_Temp",                // TOP6
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP6
    .byte_offset =   144,    //TOP6
    .decoder_function = getFractionalValue,      //TOP6
    .filter_context = { 0 }
  },
  {
    .name = "Main_Target_Temp",                // TOP7
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP7
    .byte_offset =   153,    //TOP7
    .decoder_function = getIntMinus128,      //TOP7
    .filter_context = { 0 }
  },
  {
    .name = "Compressor_Freq",                 // TOP8
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HERTZ],            // TOP8
    .byte_offset =   166,    //TOP8
    .decoder_function = getIntMinus1,        //TOP8
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Target_Temp",                 // TOP9
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP9
    .byte_offset =   42,     //TOP9
    .decoder_function = getIntMinus128,      //TOP9
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Temp",                        // TOP10
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP10
    .byte_offset =   141,    //TOP10
    .decoder_function = getIntMinus128,      //TOP10
    .filter_context = { 0 }
  },
  {
    .name = "Operations_Hours",                // TOP11
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HOURS],            // TOP11
    .byte_offset =   182,      //TOP11
    .decoder_function = unknown,             //TOP11
    .filter_context = { 0 }
  },
  {
    .name = "Operations_Counter",              // TOP12
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_COUNTER],          // TOP12
    .byte_offset =   179,      //TOP12
    .decoder_function = unknown,             //TOP12
    .filter_context = { 0 }
  },
  {
    .name = "Main_Schedule_State",             // TOP13
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP13
    .byte_offset =   5,      //TOP13
    .decoder_function = getBit1and2,         //TOP13
    .filter_context = { 0 }
  },
  {
    .name = "Outside_Temp",                    // TOP14
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP14
    .byte_offset =   142,    //TOP14
    .decoder_function = getIntMinus128,      //TOP14
    .filter_context = { 0 }
  },
  {
    .name = "Heat_Energy_Production",          // TOP15
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP15
    .byte_offset =   194,    //TOP15
    .decoder_function = getEnergy,           //TOP15
    .filter_context = { 0 }
  },
  {
    .name = "Heat_Energy_Consumption",         // TOP16
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP16
    .byte_offset =   193,    //TOP16
    .decoder_function = getEnergy,           //TOP16
    .filter_context = { 0 }
  },
  {
    .name = "Powerful_Mode_Time",              // TOP17
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_POWERFUL_MODE],     // TOP17
    .byte_offset =   7,      //TOP17
    .decoder_function = getRight3bits,       //TOP17
    .filter_context = { 0 }
  },
  {
    .name = "Quiet_Mode_Level",                // TOP18
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_QUIET_MODE],        // TOP18
    .byte_offset =   7,      //TOP18
    .decoder_function = getBit3and4and5,     //TOP18
    .filter_context = { 0 }
  },
  {
    .name = "Holiday_Mode_State",              // TOP19
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HOLIDAY_STATE],     // TOP19
    .byte_offset =   5,      //TOP19
    .decoder_function = getBit3and4,         //TOP19
    .filter_context = { 0 }
  },
  {
    .name = "ThreeWay_Valve_State",            // TOP20
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_VALVE],            // TOP20
    .byte_offset =   111,    //TOP20
    .decoder_function = getBit7and8,         //TOP20
    .filter_context = { 0 }
  },
  {
    .name = "Outside_Pipe_Temp",               // TOP21
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP21
    .byte_offset =   158,    //TOP21
    .decoder_function = getIntMinus128,      //TOP21
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Heat_Delta",                  // TOP22
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP22
    .byte_offset =   99,     //TOP22
    .decoder_function = getIntMinus128,      //TOP22
    .filter_context = { 0 }
  },
  {
    .name = "Heat_Delta",                      // TOP23
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP23
    .byte_offset =   84,     //TOP23
    .decoder_function = getIntMinus128,      //TOP23
    .filter_context = { 0 }
  },
  {
    .name = "Cool_Delta",                      // TOP24
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP24
    .byte_offset =   94,     //TOP24
    .decoder_function = getIntMinus128,      //TOP24
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Holiday_Shift_Temp",          // TOP25
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP25
    .byte_offset =   44,     //TOP25
    .decoder_function = getIntMinus128,      //TOP25
    .filter_context = { 0 }
  },
  {
    .name = "Defrosting_State",                // TOP26
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP26
    .byte_offset =   111,    //TOP26
    .decoder_function = getBit5and6,         //TOP26
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Heat_Request_Temp",            // TOP27
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP27
    .byte_offset =   38,     //TOP27
    .decoder_function = getIntMinus128,      //TOP27
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Cool_Request_Temp",            // TOP28
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP28
    .byte_offset =   39,     //TOP28
    .decoder_function = getIntMinus128,      //TOP28
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Heat_Curve_Target_High_Temp",  // TOP29
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP29
    .byte_offset =   75,     //TOP29
    .decoder_function = getIntMinus128,      //TOP29
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Heat_Curve_Target_Low_Temp",   // TOP30
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP30
    .byte_offset =   76,     //TOP30
    .decoder_function = getIntMinus128,      //TOP30
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Heat_Curve_Outside_High_Temp", // TOP31
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP31
    .byte_offset =   78,     //TOP31
    .decoder_function = getIntMinus128,      //TOP31
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Heat_Curve_Outside_Low_Temp",  // TOP32
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP32
    .byte_offset =   77,     //TOP32
    .decoder_function = getIntMinus128,      //TOP32
    .filter_context = { 0 }
  },
  {
    .name = "Room_Thermostat_Temp",            // TOP33
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP33
    .byte_offset =   156,    //TOP33
    .decoder_function = getIntMinus128,      //TOP33
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Heat_Request_Temp",            // TOP34
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP34
    .byte_offset =   40,     //TOP34
    .decoder_function = getIntMinus128,      //TOP34
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Cool_Request_Temp",            // TOP35
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP35
    .byte_offset =   41,     //TOP35
    .decoder_function = getIntMinus128,      //TOP35
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Water_Temp",                   // TOP36
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP36
    .byte_offset =   145,    //TOP36
    .decoder_function = getIntMinus128,      //TOP36
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Water_Temp",                   // TOP37
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP37
    .byte_offset =   146,    //TOP37
    .decoder_function = getIntMinus128,      //TOP37
    .filter_context = { 0 }
  },
  {
    .name = "Cool_Energy_Production",          // TOP38
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP38
    .byte_offset =   196,    //TOP38
    .decoder_function = getEnergy,           //TOP38
    .filter_context = { 0 }
  },
  {
    .name = "Cool_Energy_Consumption",         // TOP39
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP39
    .byte_offset =   195,    //TOP39
    .decoder_function = getEnergy,           //TOP39
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Energy_Production",           // TOP40
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP40
    .byte_offset =   198,    //TOP40
    .decoder_function = getEnergy,           //TOP40
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Energy_Consumption",          // TOP41
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_WATT],             // TOP41
    .byte_offset =   197,    //TOP41
    .decoder_function = getEnergy,           //TOP41
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Water_Target_Temp",            // TOP42
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP42
    .byte_offset =   147,    //TOP42
    .decoder_function = getIntMinus128,      //TOP42
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Water_Target_Temp",            // TOP43
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP43
    .byte_offset =   148,    //TOP43
    .decoder_function = getIntMinus128,      //TOP43
    .filter_context = { 0 }
  },
  {
    .name = "Error",                           // TOP44
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ERROR_STATE],       // TOP44
    .byte_offset =   113,      //TOP44
    .decoder_function = getErrorInfo,             //TOP44
    .filter_context = { 0 }
  },
  {
    .name = "Room_Holiday_Shift_Temp",         // TOP45
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP45
    .byte_offset =   43,     //TOP45
    .decoder_function = getIntMinus128,      //TOP45
    .filter_context = { 0 }
  },
  {
    .name = "Buffer_Temp",                     // TOP46
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP46
    .byte_offset =   149,    //TOP46
    .decoder_function = getIntMinus128,      //TOP46
    .filter_context = { 0 }
  },
  {
    .name = "Solar_Temp",                      // TOP47
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP47
    .byte_offset =   150,    //TOP47
    .decoder_function = getIntMinus128,      //TOP47
    .filter_context = { 0 }
  },
  {
    .name = "Pool_Temp",                       // TOP48
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP48
    .byte_offset =   151,    //TOP48
    .decoder_function = getIntMinus128,      //TOP48
    .filter_context = { 0 }
  },
  {
    .name = "Main_Hex_Outlet_Temp",            // TOP49
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP49
    .byte_offset =   154,    //TOP49
    .decoder_function = getIntMinus128,      //TOP49
    .filter_context = { 0 }
  },
  {
    .name = "Discharge_Temp",                  // TOP50
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP50
    .byte_offset =   155,    //TOP50
    .decoder_function = getIntMinus128,      //TOP50
    .filter_context = { 0 }
  },
  {
    .name = "Inside_Pipe_Temp",                // TOP51
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP51
    .byte_offset =   157,    //TOP51
    .decoder_function = getIntMinus128,      //TOP51
    .filter_context = { 0 }
  },
  {
    .name = "Defrost_Temp",                    // TOP52
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP52
    .byte_offset =   159,    //TOP52
    .decoder_function = getIntMinus128,      //TOP52
    .filter_context = { 0 }
  },
  {
    .name = "Eva_Outlet_Temp",                 // TOP53
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP53
    .byte_offset =   160,    //TOP53
    .decoder_function = getIntMinus128,      //TOP53
    .filter_context = { 0 }
  },
  {
    .name = "Bypass_Outlet_Temp",              // TOP54
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP54
    .byte_offset =   161,    //TOP54
    .decoder_function = getIntMinus128,      //TOP54
    .filter_context = { 0 }
  },
  {
    .name = "Ipm_Temp",                        // TOP55
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP55
    .byte_offset =   162,    //TOP55
    .decoder_function = getIntMinus128,      //TOP55
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Temp",                         // TOP56
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP56
    .byte_offset =   139,    //TOP56
    .decoder_function = getIntMinus128,      //TOP56
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Temp",                         // TOP57
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP57
    .byte_offset =   140,    //TOP57
    .decoder_function = getIntMinus128,      //TOP57
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Heater_State",                // TOP58
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_BLOCKED_FREE],      // TOP58
    .byte_offset =   9,      //TOP58
    .decoder_function = getBit5and6,         //TOP58
    .filter_context = { 0 }
  },
  {
    .name = "Room_Heater_State",               // TOP59
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_BLOCKED_FREE],      // TOP59
    .byte_offset =   9,      //TOP59
    .decoder_function = getBit7and8,         //TOP59
    .filter_context = { 0 }
  },
  {
    .name = "Internal_Heater_State",           // TOP60
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_INACTIVE_ACTIVE],   // TOP60
    .byte_offset =   112,    //TOP60
    .decoder_function = getBit7and8,         //TOP60
    .filter_context = { 0 }
  },
  {
    .name = "External_Heater_State",           // TOP61
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_INACTIVE_ACTIVE],   // TOP61
    .byte_offset =   112,    //TOP61
    .decoder_function = getBit5and6,         //TOP61
    .filter_context = { 0 }
  },
  {
    .name = "Fan1_Motor_Speed",                // TOP62
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ROTATIONS_PER_MIN],  // TOP62
    .byte_offset =   173,    //TOP62
    .decoder_function = getIntMinus1Times10, //TOP62
    .filter_context = { 0 }
  },
  {
    .name = "Fan2_Motor_Speed",                // TOP63
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ROTATIONS_PER_MIN],  // TOP63
    .byte_offset =   174,    //TOP63
    .decoder_function = getIntMinus1Times10, //TOP63
    .filter_context = { 0 }
  },
  {
    .name = "High_Pressure",                   // TOP64
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_PRESSURE],         // TOP64
    .byte_offset =   163,    //TOP64
    .decoder_function = getIntMinus1Div5,    //TOP64
    .filter_context = { 0 }
  },
  {
    .name = "Pump_Speed",                      // TOP65
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ROTATIONS_PER_MIN],  // TOP65
    .byte_offset =   171,    //TOP65
    .decoder_function = getIntMinus1Times50, //TOP65
    .filter_context = { 0 }
  },
  {
    .name = "Low_Pressure",                    // TOP66
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_PRESSURE],         // TOP66
    .byte_offset =   164,    //TOP66
    .decoder_function = getIntMinus1,        //TOP66
    .filter_context = { 0 }
  },
  {
    .name = "Compressor_Current",              // TOP67
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_AMPERE],           // TOP67
    .byte_offset =   165,    //TOP67
    .decoder_function = getIntMinus1Div5,    //TOP67
    .filter_context = { 0 }
  },
  {
    .name = "Force_Heater_State",              // TOP68
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_INACTIVE_ACTIVE],   // TOP68
    .byte_offset =   5,      //TOP68
    .decoder_function = getBit5and6,         //TOP68
    .filter_context = { 0 }
  },
  {
    .name = "Sterilization_State",             // TOP69
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_INACTIVE_ACTIVE],   // TOP69
    .byte_offset =   117,    //TOP69
    .decoder_function = getBit5and6,         //TOP69
    .filter_context = { 0 }
  },
  {
    .name = "Sterilization_Temp",              // TOP70
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP70
    .byte_offset =   100,    //TOP70
    .decoder_function = getIntMinus128,      //TOP70
    .filter_context = { 0 }
  },
  {
    .name = "Sterilization_Max_Time",          // TOP71
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_MINUTES],          // TOP71
    .byte_offset =   101,    //TOP71
    .decoder_function = getIntMinus1,        //TOP71
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Cool_Curve_Target_High_Temp",  // TOP72
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP72
    .byte_offset =   86,     //TOP72
    .decoder_function = getIntMinus128,      //TOP72
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Cool_Curve_Target_Low_Temp",   // TOP73
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP73
    .byte_offset =   87,     //TOP73
    .decoder_function = getIntMinus128,      //TOP73
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Cool_Curve_Outside_High_Temp", // TOP74
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP74
    .byte_offset =   89,     //TOP74
    .decoder_function = getIntMinus128,      //TOP74
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Cool_Curve_Outside_Low_Temp",  // TOP75
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP75
    .byte_offset =   88,     //TOP75
    .decoder_function = getIntMinus128,      //TOP75
    .filter_context = { 0 }
  },
  {
    .name = "Heating_Mode",                    // TOP76
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HEAT_COOL_MODE_DESC], // TOP76
    .byte_offset =   28,     //TOP76
    .decoder_function = getBit7and8,         //TOP76
    .filter_context = { 0 }
  },
  {
    .name = "Heating_Off_Outdoor_Temp",        // TOP77
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP77
    .byte_offset =   83,     //TOP77
    .decoder_function = getIntMinus128,      //TOP77
    .filter_context = { 0 }
  },
  {
    .name = "Heater_On_Outdoor_Temp",          // TOP78
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP78
    .byte_offset =   85,     //TOP78
    .decoder_function = getIntMinus128,      //TOP78
    .filter_context = { 0 }
  },
  {
    .name = "Heat_To_Cool_Temp",               // TOP79
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP79
    .byte_offset =   95,     //TOP79
    .decoder_function = getIntMinus128,      //TOP79
    .filter_context = { 0 }
  },
  {
    .name = "Cool_To_Heat_Temp",               // TOP80
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP80
    .byte_offset =   96,     //TOP80
    .decoder_function = getIntMinus128,      //TOP80
    .filter_context = { 0 }
  },
  {
    .name = "Cooling_Mode",                    // TOP81
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HEAT_COOL_MODE_DESC], // TOP81
    .byte_offset =   28,     //TOP81
    .decoder_function = getBit5and6,         //TOP81
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Heat_Curve_Target_High_Temp",  // TOP82
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP82
    .byte_offset =   79,     //TOP82
    .decoder_function = getIntMinus128,      //TOP82
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Heat_Curve_Target_Low_Temp",   // TOP83
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP83
    .byte_offset =   80,     //TOP83
    .decoder_function = getIntMinus128,      //TOP83
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Heat_Curve_Outside_High_Temp", // TOP84
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP84
    .byte_offset =   82,     //TOP84
    .decoder_function = getIntMinus128,      //TOP84
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Heat_Curve_Outside_Low_Temp",  // TOP85
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP85
    .byte_offset =   81,     //TOP85
    .decoder_function = getIntMinus128,      //TOP85
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Cool_Curve_Target_High_Temp",  // TOP86
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP86
    .byte_offset =   90,     //TOP86
    .decoder_function = getIntMinus128,      //TOP86
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Cool_Curve_Target_Low_Temp",   // TOP87
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP87
    .byte_offset =   91,     //TOP87
    .decoder_function = getIntMinus128,      //TOP87
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Cool_Curve_Outside_High_Temp", // TOP88
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP88
    .byte_offset =   93,     //TOP88
    .decoder_function = getIntMinus128,      //TOP88
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Cool_Curve_Outside_Low_Temp",  // TOP89
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP89
    .byte_offset =   92,     //TOP89
    .decoder_function = getIntMinus128,      //TOP89
    .filter_context = { 0 }
  },
  {
    .name = "Room_Heater_Operations_Hours",    // TOP90
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HOURS],            // TOP90
    .byte_offset =   185,      //TOP90
    .decoder_function = unknown,             //TOP90
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Heater_Operations_Hours",     // TOP91
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_HOURS],            // TOP91
    .byte_offset =   188,      //TOP91
    .decoder_function = unknown,             //TOP91
    .filter_context = { 0 }
  },
  {
    .name = "Heat_Pump_Model",                 // TOP92
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_MODEL],             // TOP92
    .byte_offset =   129,      //TOP92
    .decoder_function = getModel,			       //TOP92
    .filter_context = { 0 }
  },
  {
    .name = "Pump_Duty",                       // TOP93
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DUTY],             // TOP93
    .byte_offset =   172,    //TOP93
    .decoder_function = getIntMinus1,        //TOP93
    .filter_context = { 0 }
  },
  {
    .name = "Zones_State",                     // TOP94
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ZONE_STATUS],       // TOP94
    .byte_offset =   6,      //TOP94
    .decoder_function = getBit1and2,         //TOP94
    .filter_context = { 0 }
  },
  {
    .name = "Max_Pump_Duty",                   // TOP95
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DUTY],             // TOP95
    .byte_offset =   45,     //TOP95
    .decoder_function = getIntMinus1,        //TOP95
    .filter_context = { 0 }
  },
  {
    .name = "Heater_Delay_Time",               // TOP96
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_MINUTES],          // TOP96
    .byte_offset =   104,    //TOP96
    .decoder_function = getIntMinus1,        //TOP96
    .filter_context = { 0 }
  },
  {
    .name = "Heater_Start_Delta",              // TOP97
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP97
    .byte_offset =   105,    //TOP97
    .decoder_function = getIntMinus128,      //TOP97
    .filter_context = { 0 }
  },
  {
    .name = "Heater_Stop_Delta",               // TOP98
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP98
    .byte_offset =   106,    //TOP98
    .decoder_function = getIntMinus128,      //TOP98
    .filter_context = { 0 }
  },
  {
    .name = "Buffer_Installed",                // TOP99
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP99
    .byte_offset =   24,     //TOP99
    .decoder_function = getBit5and6,         //TOP99
    .filter_context = { 0 }
  },
  {
    .name = "DHW_Installed",                   // TOP100
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP100
    .byte_offset =   24,     //TOP100
    .decoder_function = getBit7and8,         //TOP100
    .filter_context = { 0 }
  },
  {
    .name = "Solar_Mode",                      // TOP101
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_SOLAR_MODE_DESC],    // TOP101
    .byte_offset =   24,     //TOP101
    .decoder_function = getBit3and4,         //TOP101
    .filter_context = { 0 }
  },
  {
    .name = "Solar_On_Delta",                  // TOP102
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP102
    .byte_offset =   61,     //TOP102
    .decoder_function = getIntMinus128,      //TOP102
    .filter_context = { 0 }
  },
  {
    .name = "Solar_Off_Delta",                 // TOP103
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP103
    .byte_offset =   62,     //TOP103
    .decoder_function = getIntMinus128,      //TOP103
    .filter_context = { 0 }
  },
  {
    .name = "Solar_Frost_Protection",          // TOP104
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP104
    .byte_offset =   63,     //TOP104
    .decoder_function = getIntMinus128,      //TOP104
    .filter_context = { 0 }
  },
  {
    .name = "Solar_High_Limit",                // TOP105
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_CELSIUS],          // TOP105
    .byte_offset =   64,     //TOP105
    .decoder_function = getIntMinus128,      //TOP105
    .filter_context = { 0 }
  },
  {
    .name = "Pump_Flowrate_Mode",              // TOP106
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_PUMP_FLOW_RATE_MODE], // TOP106
    .byte_offset =   29,     //TOP106
    .decoder_function = getBit3and4,         //TOP106
    .filter_context = { 0 }
  },
  {
    .name = "Liquid_Type",                     // TOP107
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_LIQUID_TYPE],       // TOP107
    .byte_offset =   20,     //TOP107
    .decoder_function = getBit1,             //TOP107
    .filter_context = { 0 }
  },
  {
    .name = "Alt_External_Sensor",             // TOP108
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP108
    .byte_offset =   20,     //TOP108
    .decoder_function = getBit3and4,         //TOP108
    .filter_context = { 0 }
  },
  {
    .name = "Anti_Freeze_Mode",                // TOP109
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP109
    .byte_offset =   20,     //TOP109
    .decoder_function = getBit5and6,         //TOP109
    .filter_context = { 0 }
  },
  {
    .name = "Optional_PCB",                    // TOP110
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE],  // TOP110
    .byte_offset =   20,     //TOP110
    .decoder_function = getBit7and8,         //TOP110  
    .filter_context = { 0 }
  },
  {
    .name = "Z1_Sensor_Settings",              // TOP111
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ZONES_SENSOR_TYPE],  // TOP111
    .byte_offset =   22,     //TOP111
    .decoder_function = getSecondByte,       //TOP111
    .filter_context = { 0 }
  },
  {
    .name = "Z2_Sensor_Settings",              // TOP112
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_ZONES_SENSOR_TYPE],  // TOP112
    .byte_offset =   22,     //TOP112
    .decoder_function = getFirstByte,        //TOP112 
    .filter_context = { 0 }
  },
  {
    .name = "Buffer_Tank_Delta",               // TOP113
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_KELVIN],           // TOP113
    .byte_offset =   59,     //TOP113
    .decoder_function = getIntMinus128,      //TOP113
    .filter_context = { 0 }
  },
  {
    .name = "External_Pad_Heater",             // TOP114
    .description = &topic_descriptors[TOPIC_DESCRIPTION_ENUM_EXT_PAD_HEATER_TYPE], // TOP114
    .byte_offset =   25,     //TOP114
    .decoder_function = getBit3and4,         //TOP114
    .filter_context = { 0 }
  },
};
// clang-format on

static void getBit1(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = input[offset] >> 7;
}

static void getBit1and2(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (input[offset] >> 6) - 1;
}

static void getBit3and4(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = ((input[offset] >> 4) & 0b11) - 1;
}

static void getBit5and6(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = ((input[offset] >> 2) & 0b11) - 1;
}

static void getBit7and8(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (input[offset] & 0b11) - 1;
}

static void getBit3and4and5(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = ((input[offset] >> 3) & 0b111) - 1;
}

static void getLeft5bits(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (input[offset] >> 3) - 1;
}

static void getRight3bits(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (input[offset] & 0b111) - 1;
}

static void getIntMinus1(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (int)input[offset] - 1;
}

static void getIntMinus128(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (int)input[offset] - 128;
}

static void getIntMinus1Div5(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_FLOAT;
  result->int_value = (((float)input[offset] - 1.0f) / 5.0f);
}

static void getIntMinus1Times10(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (int)input[offset] - 1;
}

static void getIntMinus1Times50(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = ((int)input[offset] - 1) * 50;
}

static void unknown(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = -1;
}

static void getOpMode(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  uint8_t val = 0;
  result->result_type = DECODE_RESULT_INT;
  switch ((int)(input[offset] & 0b111111))
  {
  case 18:
    val = 0;
  case 19:
    val = 1;
  case 25:
    val = 2;
  case 33:
    val = 3;
  case 34:
    val = 4;
  case 35:
    val = 5;
  case 41:
    val = 6;
  case 26:
    val = 7;
  case 42:
    val = 8;
  default:
    val = -1;
  }

  result->int_value = val;
}

static void getEnergy(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = ((int)*input - 1) * 200;
}

static void getPumpFlow(uint8_t *input, uint8_t offset, decode_result_t *result)
{ // TOP1 //
  float PumpFlow1 = (int)input[offset + 1];
  float PumpFlow2 = (((float)input[offset] - 1.0f) / 256.0f);
  float PumpFlow = PumpFlow1 + PumpFlow2;

  result->result_type = DECODE_RESULT_FLOAT;
  result->float_value = PumpFlow;
}

static void getFirstByte(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (*input >> 4) - 1;
}

static void getSecondByte(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = (*input & 0b1111) - 1;
}

static void getModel(uint8_t *input, uint8_t offset, decode_result_t *result)
{ // TOP92 //
  result->result_type = DECODE_RESULT_INT;
  result->int_value = -1;

  for (unsigned int i = 0; i < sizeof(knownModels) / sizeof(knownModels[0]); i++)
  {
    if (memcmp_P(&input[offset], knownModels[i], 10) == 0)
    {
      result->int_value = i;
    }
  }
}

static void getErrorInfo(uint8_t *input, uint8_t offset, decode_result_t *result)
{ // TOP44 //
  int Error_type = (int)(input[offset]);
  int Error_number = ((int)(input[offset + 1])) - 17;
  result->result_type = DECODE_RESULT_STRING;

  switch (Error_type)
  {
  case 177: // B1=F type error
    snprintf(result->string_value, sizeof(result->string_value), "F%02X", Error_number);
    break;
  case 161: // A1=H type error
    snprintf(result->string_value, sizeof(result->string_value), "H%02X", Error_number);
    break;
  default:
    snprintf(result->string_value, sizeof(result->string_value), "No error");
    break;
  }
}

static void getFractionalValue(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  getIntMinus128(input, offset, result);
  result->result_type = DECODE_RESULT_FLOAT;
  result->float_value = result->int_value;

  int fractional = (int)(input[118] & 0b111);
  switch (fractional)
  {
  case 1: // fractional .00
    break;
  case 2: // fractional .25
    result->float_value += 0.25f;
    break;
  case 3: // fractional .50
    result->float_value += 0.5f;
    break;
  case 4: // fractional .75
    result->float_value += 0.75f;
    break;
  default:
    break;
  }
}

static void getUint16Minus1(uint8_t *input, uint8_t offset, decode_result_t *result)
{
  result->result_type = DECODE_RESULT_INT;
  result->int_value = word(input[offset + 1], input[offset]) - 1;
}

void decode_get_topic_value(heatpump_topic_t topic, uint8_t *data, decode_result_t *result, bool get_unfiltered_value)
{
  topic_t *topic_config = &topic_configurations[topic];
  topic_config->decoder_function(data, topic_config->byte_offset, result);

  if (get_unfiltered_value == false && topic_config->description->filter_type != FILTER_TYPE_NONE)
  {
    const float filtered_value = filter_get_value(&(topic_config->filter_context), topic_config->description->filter_type);
    switch (result->result_type)
    {
    case DECODE_RESULT_INT:
      result->int_value = (int)filtered_value;
      break;
    case DECODE_RESULT_FLOAT:
      result->float_value = filtered_value;
      break;
    }
  }
}

void decode_heatpump_data(uint8_t data[255])
{
  decode_result_t result;
  float result_value;
  for (size_t idx = 0; idx < HEATPUMP_TOPIC_Last; idx++)
  {
    topic_configurations[idx].decoder_function(data, topic_configurations[idx].byte_offset, &result);
    switch (result.result_type)
    {
    case DECODE_RESULT_INT:
      result_value = (float)result.int_value;
      break;
    case DECODE_RESULT_FLOAT:
      result_value = result.float_value;
      break;
    }

    filter_update(&(topic_configurations[idx].filter_context), topic_configurations[idx].description->filter_type, result_value);
  }
}

const char *get_topic_name(heatpump_topic_t topic)
{
  return topic_configurations[topic].name;
}

const char *get_description_text(heatpump_topic_t topic, uint8_t description_idx)
{
  return topic_configurations[topic].description->descriptions_strs[description_idx];
}

uint8_t get_description_cnt(heatpump_topic_t topic)
{
  return topic_configurations[topic].description->number_of_descriptions;
}

void clear_filters()
{
  for (size_t idx = 0; idx < HEATPUMP_TOPIC_Last; idx++)
  {
    filter_clear(&(topic_configurations[idx].filter_context));
  }
}

void result_to_string(decode_result_t *result, char *buffer, uint16_t buffer_size)
{
  switch (result->result_type)
  {
  case DECODE_RESULT_INT:
    snprintf(buffer, buffer_size, "%d", result->int_value);
    break;
  case DECODE_RESULT_FLOAT:
    snprintf(buffer, buffer_size, "%f", result->float_value);
    break;
  case DECODE_RESULT_STRING:
    snprintf(buffer, buffer_size, "%s", result->string_value);
    break;
  }
}

uint16_t get_max_filter_depth()
{
  uint16_t max_depth = 0;
  for (size_t idx = 0; idx < HEATPUMP_TOPIC_Last; idx++)
  {
    if (topic_configurations[idx].description->filter_type != FILTER_TYPE_NONE)
    {
      max_depth = _max(max_depth, topic_configurations[idx].filter_context.filter_count);
    }
  }
  return max_depth;
}
