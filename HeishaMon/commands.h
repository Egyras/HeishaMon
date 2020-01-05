#include <ESP8266WiFi.h>


#define PANASONICQUERYSIZE 110
extern byte panasonicQuery[PANASONICQUERYSIZE];

extern const char* mqtt_topic_base;
extern const char* mqtt_logtopic;
extern const char* mqtt_willtopic;
extern const char* mqtt_set_quiet_mode_topic;
extern const char* mqtt_set_shift_temperature_topic;
extern const char* mqtt_set_mode_topic;
extern const char* mqtt_set_force_DHW_topic;
extern const char* mqtt_set_force_defrost_topic;
extern const char* mqtt_set_force_sterilization_topic;
extern const char* mqtt_set_holiday_topic;
extern const char* mqtt_set_powerfull_topic;
extern const char* mqtt_set_tank_temp_topic;
extern const char* mqtt_set_cool_temp_topic;

void send_heatpump_command(char* topic, char msg[],bool (*send_command)(byte*, int),void (*log_message)(char*));
