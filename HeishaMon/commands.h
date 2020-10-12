#include <ESP8266WiFi.h>


#define PANASONICQUERYSIZE 110
extern byte panasonicQuery[PANASONICQUERYSIZE];

#define OPTIONALPCBQUERYSIZE 19
extern byte optionalPCBQuery[OPTIONALPCBQUERYSIZE];
#define NUMBER_OF_OPTIONALPCB_TOPICS 13

static const char * optionalPcbTopics[] = {
  "Heat_Cool_Mode",
  "Compressor_State",
  "SmartGrid_Mode",
  "External_Thermostat_1_State",
  "External_Thermostat_2_State",
  "Demand_Control",
  "Pool_Temp",
  "Buffer_Temp",
  "Z1_Room_Temp",
  "Z1_Water_Temp",
  "Z2_Room_Temp",
  "Z2_Water_Temp",
  "Solar_Temp"  
};

static const byte optionalPcbBytes[] = {
  6,
  6,
  6,
  6,
  6,
  14,
  7,
  8,
  10,
  16,
  11,
  15,
  13  
};

extern const char* mqtt_topic_values;
extern const char* mqtt_topic_1wire;
extern const char* mqtt_topic_s0;
extern const char* mqtt_topic_pcb;
extern const char* mqtt_logtopic;
extern const char* mqtt_willtopic;
extern const char* mqtt_iptopic;
extern const char* mqtt_set_heatpump_state_topic;
extern const char* mqtt_set_quiet_mode_topic;
extern const char* mqtt_set_z1_heat_request_temperature_topic;
extern const char* mqtt_set_z1_cool_request_temperature_topic;
extern const char* mqtt_set_z2_heat_request_temperature_topic;
extern const char* mqtt_set_z2_cool_request_temperature_topic;
extern const char* mqtt_set_operationmode_topic;
extern const char* mqtt_set_force_DHW_topic;
extern const char* mqtt_set_force_defrost_topic;
extern const char* mqtt_set_force_sterilization_topic;
extern const char* mqtt_set_holiday_topic;
extern const char* mqtt_set_powerful_topic;
extern const char* mqtt_set_dhw_temp_topic;
extern const char* mqtt_send_raw_value_topic;
extern const char* mqtt_set_pump_topic;
extern const char* mqtt_set_pumpspeed_topic;

void send_heatpump_command(char* topic, char *msg,bool (*send_command)(byte*, int),void (*log_message)(char*));
void set_optionalpcb(char* topic, char *msg,void (*log_message)(char*));
