#include "commands.h"
#include <LittleFS.h>

//removed checksum from default query, is calculated in send_command
byte panasonicQuery[] = {0x71, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte optionalPCBQuery[] = {0xF1, 0x11, 0x01, 0x50, 0x00, 0x00, 0x40, 0xFF, 0xFF, 0xE5, 0xFF, 0xFF, 0x00, 0xFF, 0xEB, 0xFF, 0xFF, 0x00, 0x00};
byte panasonicSendQuery[] PROGMEM = {0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//timer for saving optional pcb data to flash
unsigned long nextOptionalPCBSave = 0;

const char* mqtt_topic_values = "main";
const char* mqtt_topic_commands = "commands";
const char* mqtt_topic_pcbvalues = "optional";
const char* mqtt_topic_1wire = "1wire";
const char* mqtt_topic_s0 = "s0";
const char* mqtt_logtopic = "log";

const char* mqtt_willtopic = "LWT";
const char* mqtt_iptopic = "ip";

const char* mqtt_send_raw_value_topic = "SendRawValue";

static unsigned int temp2hex(float temp) {
  int hextemp = 0;
  if (temp > 120) {
    hextemp = 0;
  } else if (temp < -78) {
    hextemp = 255;
  } else {
    byte Uref = 255;
    int constant = 3695;
    int R25 = 6340;
    byte T25 = 25;
    int Rf = 6480;
    float K = 273.15;
    float RT = R25 * exp(constant * (1 / (temp + K) - 1 / (T25 + K)));
    hextemp = Uref * (RT / (Rf + RT));
  }
  return hextemp;
}


unsigned int set_heatpump_state(char *msg, unsigned char *cmd, char *log_msg) {
  byte heatpump_state = 1;
  String set_heatpump_state_string(msg);

  if ( set_heatpump_state_string.toInt() == 1 ) {
    heatpump_state = 2;
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set heatpump state to %d"), heatpump_state);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[4] = heatpump_state;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_pump(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_pump_string(msg);

  byte pump_state = 16;
  if ( set_pump_string.toInt() == 1 ) {
    pump_state = 32;
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set pump state to %d"), pump_state);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[4] = pump_state;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_max_pump_duty(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_pumpduty_string(msg);

  byte pumpduty = set_pumpduty_string.toInt() + 1;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set max pump duty to %d"), pumpduty - 1);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[45] = pumpduty;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_quiet_mode(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_quiet_mode_string(msg);

  byte quiet_mode = (set_quiet_mode_string.toInt() + 1) * 8;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set Quiet mode to %d"), quiet_mode / 8 - 1);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[7] = quiet_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_z1_heat_request_temperature(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set z1 heat request temperature to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[38] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_z1_cool_request_temperature(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set z1 cool request temperature to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[39] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_z2_heat_request_temperature(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set z2 heat request temperature to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[40] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_z2_cool_request_temperature(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set z2 cool request temperature to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[41] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_force_DHW(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_force_DHW_string(msg);

  byte force_DHW_mode = 64; //hex 0x40
  if ( set_force_DHW_string.toInt() == 1 ) {
    force_DHW_mode = 128; //hex 0x80
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set force DHW mode to %d"), force_DHW_mode);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[4] = force_DHW_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_force_defrost(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_force_defrost_string(msg);

  byte force_defrost_mode = 0;
  if ( set_force_defrost_string.toInt() == 1 ) {
    force_defrost_mode = 2; //hex 0x02
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set force defrost mode to %d"), force_defrost_mode);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[8] = force_defrost_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_force_sterilization(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_force_sterilization_string(msg);

  byte force_sterilization_mode = 0;
  if ( set_force_sterilization_string.toInt() == 1 ) {
    force_sterilization_mode = 4; //hex 0x04
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set force sterilization mode to %d"), force_sterilization_mode);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[8] = force_sterilization_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_holiday_mode(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_holiday_string(msg);

  byte set_holiday = 16; //hex 0x10
  if ( set_holiday_string.toInt() == 1 ) {
    set_holiday = 32; //hex 0x20
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set holiday mode to %d"), set_holiday);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[5] = set_holiday;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_powerful_mode(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_powerful_string(msg);

  byte set_powerful = (set_powerful_string.toInt() ) + 73;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set powerful mode to %d"), (set_powerful - 73) );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[7] = set_powerful;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_DHW_temp(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_DHW_temp_string(msg);

  byte set_DHW_temp = set_DHW_temp_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set DHW temperature to %d"), set_DHW_temp - 128);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[42] = set_DHW_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_operation_mode(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_mode_string(msg);

  byte set_mode;
  switch (set_mode_string.toInt()) {
    case 0: set_mode = 18; break;
    case 1: set_mode = 19; break;
    case 2: set_mode = 24; break;
    case 3: set_mode = 33; break;
    case 4: set_mode = 34; break;
    case 5: set_mode = 35; break;
    case 6: set_mode = 40; break;
    default: set_mode = 0; break;
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set heat pump mode to %d"), set_mode);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[6] = set_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_curves(char *msg, unsigned char *cmd, char *log_msg) {
  memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));

  StaticJsonDocument<512> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, msg);
  if (!error) {
    char tmpmsg[256] = { 0 };
    JsonVariant jsonValue;
    snprintf(tmpmsg, 255, "SetCurves JSON received ok");
    memcpy(log_msg, tmpmsg, sizeof(tmpmsg));
    //set correct bytes according to the values in json and if not exists keep default 0x00 value which keeps current setting for this byte
    jsonValue = jsonDoc["zone1"]["heat"]["target"]["high"]; if (!jsonValue.isNull()) cmd[75] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["heat"]["target"]["low"]; if (!jsonValue.isNull()) cmd[76] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["heat"]["outside"]["low"]; if (!jsonValue.isNull()) cmd[77] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["heat"]["outside"]["high"]; if (!jsonValue.isNull()) cmd[78] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["heat"]["target"]["high"]; if (!jsonValue.isNull()) cmd[79] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["heat"]["target"]["low"]; if (!jsonValue.isNull()) cmd[80] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["heat"]["outside"]["low"]; if (!jsonValue.isNull()) cmd[81] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["heat"]["outside"]["high"]; if (!jsonValue.isNull()) cmd[82] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["cool"]["target"]["high"]; if (!jsonValue.isNull()) cmd[86] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["cool"]["target"]["low"]; if (!jsonValue.isNull()) cmd[87] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["cool"]["outside"]["low"]; if (!jsonValue.isNull()) cmd[88] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone1"]["cool"]["outside"]["high"]; if (!jsonValue.isNull()) cmd[89] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["cool"]["target"]["high"]; if (!jsonValue.isNull()) cmd[90] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["cool"]["target"]["low"]; if (!jsonValue.isNull()) cmd[91] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["cool"]["outside"]["low"]; if (!jsonValue.isNull()) cmd[92] = jsonValue.as<int>() + 128;
    jsonValue = jsonDoc["zone2"]["cool"]["outside"]["high"]; if (!jsonValue.isNull()) cmd[93] = jsonValue.as<int>() + 128;
  } else {
    char tmpmsg[256] = { 0 };
    snprintf(tmpmsg, 255, "SetCurves JSON decode failed!");
    memcpy(log_msg, tmpmsg, sizeof(tmpmsg));
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_zones(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_mode_string(msg);

  byte set_mode;
  switch (set_mode_string.toInt()) {
    case 0: set_mode = 64; break;
    case 1: set_mode = 128; break;
    case 2: set_mode = 192; break;
    default: set_mode = 0; break;
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set zones active state to %d"), set_mode);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[6] = set_mode;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_floor_heat_delta(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set floor heat delta to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[84] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_floor_cool_delta(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set floor cool delta to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[94] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_dhw_heat_delta(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String set_temperature_string(msg);

  byte request_temp = set_temperature_string.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set floor heat delta to %d"), request_temp - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[99] = request_temp;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_reset(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned  len = 0;
  byte resetRequest = 0;
  String set_reset_string(msg);

  if ( set_reset_string.toInt() == 1 ) {
    resetRequest = 1;
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set reset to %d"), resetRequest);
    memcpy(log_msg, tmp, sizeof(tmp));
  }


  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[8] = resetRequest;
  }

  return sizeof(panasonicSendQuery);
}

unsigned int set_heater_delay_time(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String stringValue(msg);

  byte byteValue = stringValue.toInt() + 1;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set heater delay time to %d"), byteValue - 1 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[104] = byteValue;
  }

  return sizeof(panasonicSendQuery);
}
unsigned int set_heater_start_delta(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String stringValue(msg);

  byte byteValue = stringValue.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set heater start delta to %d"), byteValue - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[105] = byteValue;
  }

  return sizeof(panasonicSendQuery);
}
unsigned int set_heater_stop_delta(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String stringValue(msg);

  byte byteValue = stringValue.toInt() + 128;

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set heater stop delta to %d"), byteValue - 128 );
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[106] = byteValue;
  }

  return sizeof(panasonicSendQuery);
}
unsigned int set_main_schedule(char *msg, unsigned char *cmd, char *log_msg) {
  unsigned int len = 0;
  String stringValue(msg);

  byte byteValue = 64; //hex 0x40

  if ( stringValue.toInt() == 1 ) {
    byteValue = 128; //hex 0x80
  }

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set main schedule to %d"), byteValue);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    memcpy_P(cmd, panasonicSendQuery, sizeof(panasonicSendQuery));
    cmd[5] = byteValue;
  }

  return sizeof(panasonicSendQuery);
}

//start of optional pcb commands
unsigned int set_byte_6(int val, int base, int bit, char *log_msg, const char *func) {
  unsigned char hex = (optionalPCBQuery[6] & ~(base << bit)) | (val << bit);

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set optional pcb '%s' to state %d (result byte 6: %02x)"), func, val, hex);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    optionalPCBQuery[6] = hex;
  }

  return sizeof(optionalPCBQuery);
}

unsigned int set_byte_9(char *msg, char *log_msg) {
  String set_pcb_string(msg);

  byte set_pcb_value = set_pcb_string.toInt();

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set optional pcb '%s' to %02x"), __FUNCTION__, set_pcb_value);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    optionalPCBQuery[9] = set_pcb_value;
  }
  return sizeof(optionalPCBQuery);
}

unsigned int set_heat_cool_mode(char *msg, char *log_msg) {
  String set_pcb_string(msg);
  int set_pcb_value = (set_pcb_string.toInt() == 1);

  return set_byte_6(set_pcb_value, 0b1, 7, log_msg, __FUNCTION__);
}

unsigned int set_compressor_state(char *msg, char *log_msg) {
  String set_pcb_string(msg);
  int set_pcb_value = (set_pcb_string.toInt() == 1);

  return set_byte_6(set_pcb_value, 0b1, 6, log_msg, __FUNCTION__);
}

unsigned int set_smart_grid_mode(char *msg, char *log_msg) {
  String set_pcb_string(msg);
  int set_pcb_value = set_pcb_string.toInt();

  if (set_pcb_value < 4) {
    return set_byte_6(set_pcb_value, 0b11, 4, log_msg, __FUNCTION__);
  } else {
    return 0;
  }
}

unsigned int set_external_thermostat_1_state(char *msg, char *log_msg) {
  String set_pcb_string(msg);
  int set_pcb_value = set_pcb_string.toInt();

  if (set_pcb_value < 4) {
    return set_byte_6(set_pcb_value, 0b11, 2, log_msg, __FUNCTION__);
  } else {
    return 0;
  }
}

unsigned int set_external_thermostat_2_state(char *msg, char *log_msg) {
  String set_pcb_string(msg);
  int set_pcb_value = set_pcb_string.toInt();

  if (set_pcb_value < 4) {
    return set_byte_6(set_pcb_value, 0b11, 0, log_msg, __FUNCTION__);
  } else {
    return 0;
  }
}

unsigned int set_demand_control(char *msg, char *log_msg) {
  String set_pcb_string(msg);

  byte set_pcb_value = set_pcb_string.toInt();

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set optional pcb '%s' to %02x"), __FUNCTION__, set_pcb_value);
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    optionalPCBQuery[14] = set_pcb_value;
  }

  return sizeof(optionalPCBQuery);
}

unsigned int set_xxx_temp(char *msg, char *log_msg, int byte, const char *func) {
  String set_pcb_string(msg);

  float temp = set_pcb_string.toFloat();

  {
    char tmp[256] = { 0 };
    snprintf_P(tmp, 255, PSTR("set optional pcb '%s' to temp %.2f (%02x)"), func, temp, temp2hex(temp));
    memcpy(log_msg, tmp, sizeof(tmp));
  }

  {
    optionalPCBQuery[byte] = temp2hex(temp);
  }

  return sizeof(optionalPCBQuery);
}

unsigned int set_pool_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 7, __FUNCTION__);
}

unsigned int set_buffer_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 8, __FUNCTION__);
}

unsigned int set_z1_room_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 10, __FUNCTION__);
}

unsigned int set_z1_water_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 16, __FUNCTION__);
}

unsigned int set_z2_room_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 11, __FUNCTION__);
}

unsigned int set_z2_water_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 15, __FUNCTION__);
}

unsigned int set_solar_temp(char *msg, char *log_msg) {
  return set_xxx_temp(msg, log_msg, 13, __FUNCTION__);
}




void send_heatpump_command(char* topic, char *msg, bool (*send_command)(byte*, int), void (*log_message)(char*), bool optionalPCB) {
  unsigned char cmd[256] = { 0 };
  char log_msg[256] = { 0 };
  unsigned int len = 0;

  for (unsigned int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    if (strcmp(topic, commands[i].name) == 0) {
      len = commands[i].func(msg, cmd, log_msg);
      log_message(log_msg);
      send_command(cmd, len);
    }
  }

  if (optionalPCB) {
    //run for optional pcb commands
    for (unsigned int i = 0; i < sizeof(optionalCommands) / sizeof(optionalCommands[0]); i++) {
      if (strcmp(topic, optionalCommands[i].name) == 0) {
        len = optionalCommands[i].func(msg, log_msg);
        log_message(log_msg);
        if (millis() > nextOptionalPCBSave) {  // only save each 5 minutes
          nextOptionalPCBSave = millis() + (1000 * OPTIONALPCBSAVETIME);
          if (saveOptionalPCB(optionalPCBQuery, OPTIONALPCBQUERYSIZE)) {
            log_message((char*)"Succesfully saved optional PCB data to flash!");
          } else {
            log_message((char*)"Failed to save optional PCB data to flash!");
          }
        }
      }
    }
  }

}


bool saveOptionalPCB(byte* command, int length) {
  if (LittleFS.begin()) {
    File pcbfile = LittleFS.open("/optionalpcb.raw", "w");
    if (pcbfile) {
      pcbfile.write(command, length);
      pcbfile.close();
      return true;
    }

  }
  return false;
}
bool loadOptionalPCB(byte* command, int length) {
  if (LittleFS.begin()) {
    if (LittleFS.exists("/optionalpcb.raw")) {
      File pcbfile = LittleFS.open("/optionalpcb.raw", "r");
      if (pcbfile) {
        pcbfile.read(command, length);
        pcbfile.close();
        return true;
      }
    }
  }
  return false;
}
