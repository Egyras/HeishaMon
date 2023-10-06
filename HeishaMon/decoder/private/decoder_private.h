#pragma once
#include "stdint.h"
#include "string.h"
#include "filter/filter.h"

#define SIZE_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

typedef enum
{
  TOPIC_DESCRIPTION_ENUM_DISABLE_ENABLE,
  TOPIC_DESCRIPTION_ENUM_BLOCKED_FREE,
  TOPIC_DESCRIPTION_ENUM_OFF_ON,
  TOPIC_DESCRIPTION_ENUM_INACTIVE_ACTIVE,
  TOPIC_DESCRIPTION_ENUM_PUMP_FLOW_RATE_MODE,
  TOPIC_DESCRIPTION_ENUM_HOLIDAY_STATE,
  TOPIC_DESCRIPTION_ENUM_OP_MODE_DESC,
  TOPIC_DESCRIPTION_ENUM_POWERFUL_MODE,
  TOPIC_DESCRIPTION_ENUM_QUIET_MODE,
  TOPIC_DESCRIPTION_ENUM_VALVE,
  TOPIC_DESCRIPTION_ENUM_LITERS_PER_MIN,
  TOPIC_DESCRIPTION_ENUM_ROTATIONS_PER_MIN,
  TOPIC_DESCRIPTION_ENUM_PRESSURE,
  TOPIC_DESCRIPTION_ENUM_CELSIUS,
  TOPIC_DESCRIPTION_ENUM_KELVIN,
  TOPIC_DESCRIPTION_ENUM_HERTZ,
  TOPIC_DESCRIPTION_ENUM_COUNTER,
  TOPIC_DESCRIPTION_ENUM_HOURS,
  TOPIC_DESCRIPTION_ENUM_WATT,
  TOPIC_DESCRIPTION_ENUM_ERROR_STATE,
  TOPIC_DESCRIPTION_ENUM_AMPERE,
  TOPIC_DESCRIPTION_ENUM_MINUTES,
  TOPIC_DESCRIPTION_ENUM_DUTY,
  TOPIC_DESCRIPTION_ENUM_ZONE_STATUS,
  TOPIC_DESCRIPTION_ENUM_HEAT_COOL_MODE_DESC,
  TOPIC_DESCRIPTION_ENUM_SOLAR_MODE_DESC,
  TOPIC_DESCRIPTION_ENUM_ZONES_SENSOR_TYPE,
  TOPIC_DESCRIPTION_ENUM_LIQUID_TYPE,
  TOPIC_DESCRIPTION_ENUM_EXT_PAD_HEATER_TYPE,
  TOPIC_DESCRIPTION_ENUM_MODEL,
  TOPIC_DESCRIPTION_ENUM_Last
} topic_description_emum_t;

typedef struct
{
  const char **descriptions_strs;
  uint8_t number_of_descriptions;
  filter_type_t filter_type;
} topic_description_t;

// Array of strings for topic description
static const char *TOPIC_DESCRIPTION_TEXTS_DISABLE_ENABLE[] PROGMEM = {"Disabled", "Enabled"};
static const char *TOPIC_DESCRIPTION_TEXTS_BLOCKED_FREE[] PROGMEM = {"Blocked", "Free"};
static const char *TOPIC_DESCRIPTION_TEXTS_OFF_ON[] PROGMEM = {"Off", "On"};
static const char *TOPIC_DESCRIPTION_TEXTS_INACTIVE_ACTIVE[] PROGMEM = {"Inactive", "Active"};
static const char *TOPIC_DESCRIPTION_TEXTS_PUMP_FLOW_RATE_MODE[] PROGMEM = {"DeltaT", "Max flow"};
static const char *TOPIC_DESCRIPTION_TEXTS_HOLIDAY_STATE[] PROGMEM = {"Off", "Scheduled", "Active"};
static const char *TOPIC_DESCRIPTION_TEXTS_OP_MODE_DESC[] PROGMEM = {"Heat", "Cool", "Auto(heat)", "DHW", "Heat+DHW", "Cool+DHW", "Auto(heat)+DHW", "Auto(cool)", "Auto(cool)+DHW"};
static const char *TOPIC_DESCRIPTION_TEXTS_POWERFUL_MODE[] PROGMEM = {"Off", "30min", "60min", "90min"};
static const char *TOPIC_DESCRIPTION_TEXTS_QUIET_MODE[] PROGMEM = {"Off", "Level 1", "Level 2", "Level 3"};
static const char *TOPIC_DESCRIPTION_TEXTS_VALVE[] PROGMEM = {"Room", "DHW"};
static const char *TOPIC_DESCRIPTION_TEXTS_LITERS_PER_MIN[] PROGMEM = {"l/min"};
static const char *TOPIC_DESCRIPTION_TEXTS_ROTATIONS_PER_MIN[] PROGMEM = {"r/min"};
static const char *TOPIC_DESCRIPTION_TEXTS_PRESSURE[] PROGMEM = {"Kgf/cm2"};
static const char *TOPIC_DESCRIPTION_TEXTS_CELSIUS[] PROGMEM = {"&deg;C"};
static const char *TOPIC_DESCRIPTION_TEXTS_KELVIN[] PROGMEM = {"K"};
static const char *TOPIC_DESCRIPTION_TEXTS_HERTZ[] PROGMEM = {"Hz"};
static const char *TOPIC_DESCRIPTION_TEXTS_COUNTER[] PROGMEM = {"count"};
static const char *TOPIC_DESCRIPTION_TEXTS_HOURS[] PROGMEM = {"hours"};
static const char *TOPIC_DESCRIPTION_TEXTS_WATT[] PROGMEM = {"Watt"};
static const char *TOPIC_DESCRIPTION_TEXTS_ERROR_STATE[] PROGMEM = {"Error"};
static const char *TOPIC_DESCRIPTION_TEXTS_AMPERE[] PROGMEM = {"Ampere"};
static const char *TOPIC_DESCRIPTION_TEXTS_MINUTES[] PROGMEM = {"Minutes"};
static const char *TOPIC_DESCRIPTION_TEXTS_DUTY[] PROGMEM = {"Duty"};
static const char *TOPIC_DESCRIPTION_TEXTS_ZONE_STATUS[] PROGMEM = {"Zone1 active", "Zone2 active", "Zone1 and zone2 active"};
static const char *TOPIC_DESCRIPTION_TEXTS_HEAT_COOL_MODE_DESC[] PROGMEM = {"Comp. Curve", "Direct"};
static const char *TOPIC_DESCRIPTION_TEXTS_SOLAR_MODE_DESC[] PROGMEM = {"Disabled", "Buffer", "DHW"};
static const char *TOPIC_DESCRIPTION_TEXTS_ZONES_SENSOR_TYPE[] = {"Water Temperature", "External Thermostat", "Internal Thermostat", "Thermistor"};
static const char *TOPIC_DESCRIPTION_TEXTS_LIQUID_TYPE[] PROGMEM = {"Water", "Glycol"};
static const char *TOPIC_DESCRIPTION_TEXTS_EXT_PAD_HEATER_TYPE[] PROGMEM = {"Disabled", "Type-A", "Type-B"};
static const char *TOPIC_DESCRIPTION_TEXTS_MODEL[] PROGMEM = {
    "WH-MDC05H3E5",                          // 0
    "WH-MDC07H3E5",                          // 1
    "IDU:WH-SXC09H3E5, ODU:WH-UX09HE5",      // 2
    "IDU:WH-SDC09H3E8, ODU:WH-UD09HE8",      // 3
    "IDU:WH-SXC09H3E8, ODU:WH-UX09HE8",      // 4
    "IDU:WH-SXC12H9E8, ODU:WH-UX12HE8",      // 5
    "IDU:WH-SXC16H9E8, ODU:WH-UX16HE8",      // 6
    "IDU:WH-SDC05H3E5, ODU:WH-UD05HE5",      // 7
    "IDU:WH-SDC0709J3E5, ODU:WH-UD09JE5",    // 8
    "WH-MDC05J3E5",                          // 9
    "WH-MDC09H3E5",                          // 10
    "WH-MXC09H3E5",                          // 11
    "IDU:WH-ADC0309J3E5, ODU:WH-UD09JE5",    // 12
    "IDU:WH-ADC0916H9E8, ODU:WH-UX12HE8",    // 13
    "IDU:WH-SQC09H3E8, ODU:WH-UQ09HE8",      // 14
    "IDU:WH-SDC09H3E5, ODU:WH-UD09HE5",      // 15
    "IDU:WH-ADC0309H3E5, ODU:WH-UD09HE5",    // 16
    "IDU:WH-ADC0309J3E5, ODU: WH-UD05JE5",   // 17
    "IDU: WH-SDC0709J3E5, ODU: WH-UD07JE5",  // 18
    "IDU: WH-SDC07H3E5-1 ODU: WH-UD07HE5-1", // 19
    "WH-MDC07J3E5",                          // 20
    "WH-MDC09J3E5",                          // 21
    "IDU: WH-SDC0305J3E5 ODU: WH-UD05JE5",   // 22
    "WH-MXC09J3E8",                          // 23
    "WH-MXC12J9E8",                          // 24
    "IDU: WH-ADC1216H6E5 ODU: WH-UD12HE5",   // 25
    "IDU: WH-ADC0309J3E5C ODU: WH-UD07JE5",  // 26
    "WH-MDC07J3E5",                          // 27
    "WH-MDC05J3E5",                          // 28
};

static const uint8_t knownModels[SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_MODEL)][10] PROGMEM = {
    // stores the bytes #129 to #138 of known models in the same order as the const above
    0xE2, 0xCF, 0x0B, 0x13, 0x33, 0x32, 0xD1, 0x0C, 0x16, 0x33, // 0
    0xE2, 0xCF, 0x0B, 0x14, 0x33, 0x42, 0xD1, 0x0B, 0x17, 0x33, // 1
    0xE2, 0xCF, 0x0D, 0x77, 0x09, 0x12, 0xD0, 0x0B, 0x05, 0x11, // 2
    0xE2, 0xCF, 0x0C, 0x88, 0x05, 0x12, 0xD0, 0x0B, 0x97, 0x05, // 3
    0xE2, 0xCF, 0x0D, 0x85, 0x05, 0x12, 0xD0, 0x0C, 0x94, 0x05, // 4
    0xE2, 0xCF, 0x0D, 0x86, 0x05, 0x12, 0xD0, 0x0C, 0x95, 0x05, // 5
    0xE2, 0xCF, 0x0D, 0x87, 0x05, 0x12, 0xD0, 0x0C, 0x96, 0x05, // 6
    0xE2, 0xCE, 0x0D, 0x71, 0x81, 0x72, 0xCE, 0x0C, 0x92, 0x81, // 7
    0x62, 0xD2, 0x0B, 0x43, 0x54, 0x42, 0xD2, 0x0B, 0x72, 0x66, // 8
    0xC2, 0xD3, 0x0B, 0x33, 0x65, 0xB2, 0xD3, 0x0B, 0x94, 0x65, // 9
    0xE2, 0xCF, 0x0B, 0x15, 0x33, 0x42, 0xD1, 0x0B, 0x18, 0x33, // 10
    0xE2, 0xCF, 0x0B, 0x41, 0x34, 0x82, 0xD1, 0x0B, 0x31, 0x35, // 11
    0x62, 0xD2, 0x0B, 0x45, 0x54, 0x42, 0xD2, 0x0B, 0x47, 0x55, // 12
    0xE2, 0xCF, 0x0C, 0x74, 0x09, 0x12, 0xD0, 0x0D, 0x95, 0x05, // 13
    0xE2, 0xCF, 0x0B, 0x82, 0x05, 0x12, 0xD0, 0x0C, 0x91, 0x05, // 14
    0xE2, 0xCF, 0x0C, 0x55, 0x14, 0x12, 0xD0, 0x0B, 0x15, 0x08, // 15
    0xE2, 0xCF, 0x0C, 0x43, 0x00, 0x12, 0xD0, 0x0B, 0x15, 0x08, // 16
    0x62, 0xD2, 0x0B, 0x45, 0x54, 0x32, 0xD2, 0x0C, 0x45, 0x55, // 17
    0x62, 0xD2, 0x0B, 0x43, 0x54, 0x42, 0xD2, 0x0C, 0x46, 0x55, // 18
    0xE2, 0xCF, 0x0C, 0x54, 0x14, 0x12, 0xD0, 0x0B, 0x14, 0x08, // 19
    0xC2, 0xD3, 0x0B, 0x34, 0x65, 0xB2, 0xD3, 0x0B, 0x95, 0x65, // 20
    0xC2, 0xD3, 0x0B, 0x35, 0x65, 0xB2, 0xD3, 0x0B, 0x96, 0x65, // 21
    0x62, 0xD2, 0x0B, 0x41, 0x54, 0x32, 0xD2, 0x0C, 0x45, 0x55, // 22
    0x32, 0xD4, 0x0B, 0x87, 0x84, 0x73, 0x90, 0x0C, 0x84, 0x84, // 23
    0x32, 0xD4, 0x0B, 0x88, 0x84, 0x73, 0x90, 0x0C, 0x85, 0x84, // 24
    0xE2, 0xCF, 0x0B, 0x75, 0x09, 0x12, 0xD0, 0x0C, 0x06, 0x11, // 25
    0x42, 0xD4, 0x0B, 0x83, 0x71, 0x42, 0xD2, 0x0C, 0x46, 0x55, // 26
    0xC2, 0xD3, 0x0C, 0x34, 0x65, 0xB2, 0xD3, 0x0B, 0x95, 0x65, // 27
    0xC2, 0xD3, 0x0C, 0x33, 0x65, 0xB2, 0xD3, 0x0B, 0x94, 0x65, // 28
};

// clang-format off
static topic_description_t topic_descriptors[] = {
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_DISABLE_ENABLE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_DISABLE_ENABLE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_BLOCKED_FREE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_BLOCKED_FREE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_OFF_ON,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_OFF_ON),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_INACTIVE_ACTIVE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_INACTIVE_ACTIVE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_PUMP_FLOW_RATE_MODE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_PUMP_FLOW_RATE_MODE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_HOLIDAY_STATE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_HOLIDAY_STATE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_OP_MODE_DESC,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_OP_MODE_DESC),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_POWERFUL_MODE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_POWERFUL_MODE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_QUIET_MODE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_QUIET_MODE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_VALVE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_VALVE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_LITERS_PER_MIN,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_LITERS_PER_MIN),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_ROTATIONS_PER_MIN,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_ROTATIONS_PER_MIN),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_PRESSURE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_PRESSURE),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_CELSIUS,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_CELSIUS),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_KELVIN,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_KELVIN),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_HERTZ,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_HERTZ),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_COUNTER,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_COUNTER),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_HOURS,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_HOURS),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_WATT,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_WATT),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_ERROR_STATE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_ERROR_STATE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_AMPERE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_AMPERE),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_MINUTES,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_MINUTES),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_DUTY,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_DUTY),
      .filter_type = FILTER_TYPE_AVG
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_ZONE_STATUS,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_ZONE_STATUS),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_HEAT_COOL_MODE_DESC,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_HEAT_COOL_MODE_DESC),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_SOLAR_MODE_DESC,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_SOLAR_MODE_DESC),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_ZONES_SENSOR_TYPE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_ZONES_SENSOR_TYPE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_LIQUID_TYPE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_LIQUID_TYPE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_EXT_PAD_HEATER_TYPE,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_EXT_PAD_HEATER_TYPE),
      .filter_type = FILTER_TYPE_NONE
    },
    {
      .descriptions_strs = TOPIC_DESCRIPTION_TEXTS_MODEL,
      .number_of_descriptions = SIZE_OF_ARRAY(TOPIC_DESCRIPTION_TEXTS_MODEL),
      .filter_type = FILTER_TYPE_NONE
    },
};
