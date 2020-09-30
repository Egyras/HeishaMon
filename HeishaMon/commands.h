#include <ESP8266WiFi.h>


#define PANASONICQUERYSIZE 110
extern byte panasonicQuery[PANASONICQUERYSIZE];

extern char mqtt_topic_base[40];
extern const char* mqtt_topic_values;
extern const char* mqtt_topic_1wire;
extern const char* mqtt_topic_s0;
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

void send_heatpump_command(char* topic, char *msg,bool (*send_command)(byte*, int),void (*log_message)(char*));
