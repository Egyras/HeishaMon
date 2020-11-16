#include <ESP8266WiFi.h>


#define PANASONICQUERYSIZE 110
extern byte panasonicQuery[PANASONICQUERYSIZE];

#define OPTIONALPCBQUERYSIZE 19
extern byte optionalPCBQuery[OPTIONALPCBQUERYSIZE];

extern const char* mqtt_topic_values;
extern const char* mqtt_topic_1wire;
extern const char* mqtt_topic_s0;
extern const char* mqtt_topic_pcb;
extern const char* mqtt_logtopic;
extern const char* mqtt_willtopic;
extern const char* mqtt_iptopic;
extern const char* mqtt_send_raw_value_topic;

unsigned int set_heatpump_state(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_pump(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_pump_speed(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_quiet_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z1_heat_request_temperature(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z1_cool_request_temperature(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z2_heat_request_temperature(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z2_cool_request_temperature(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_force_DHW(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_force_defrost(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_force_sterilization(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_holiday_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_powerful_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_operation_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_DHW_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_heat_cool_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_compressor_state(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_smart_grid_mode(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_external_thermostat_1_state(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_external_thermostat_2_state(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_demand_control(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_pool_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_buffer_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z1_room_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z1_water_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z2_room_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_z2_water_temp(char *msg, unsigned char **cmd, char **log_msg);
unsigned int set_solar_temp(char *msg, unsigned char **cmd, char **log_msg);

struct {
  const char *name;
  unsigned int (*func)(char *msg, unsigned char **cmd, char **log_msg);
} commands[] = {
  // set heatpump state to on by sending 1
  { "SetHeatpump", set_heatpump_state },
  // set pump state to on by sending 1
  { "SetPump", set_pump },
  // set pump speed
  { "SetPumpSpeed", set_pump_speed },
  // set 0 for Off mode, set 1 for Quiet mode 1, set 2 for Quiet mode 2, set 3 for Quiet mode 3
  { "SetQuietMode", set_quiet_mode },
  // z1 heat request temp -  set from -5 to 5 to get same temperature shift point or set direct temp
  { "SetZ1HeatRequestTemperature", set_z1_heat_request_temperature },
  // z1 cool request temp -  set from -5 to 5 to get same temperature shift point or set direct temp
  { "SetZ1CoolRequestTemperature", set_z1_cool_request_temperature },
  // z2 heat request temp -  set from -5 to 5 to get same temperature shift point or set direct temp
  { "SetZ2HeatRequestTemperature", set_z2_heat_request_temperature },
  // z2 cool request temp -  set from -5 to 5 to get same temperature shift point or set direct temp
  { "SetZ2CoolRequestTemperature", set_z2_cool_request_temperature },
  // set mode to force DHW by sending 1
  { "SetForceDHW", set_force_DHW },
  // set mode to force defrost  by sending 1
  { "SetForceDefrost", set_force_defrost },
  // set mode to force sterilization by sending 1
  { "SetForceSterilization", set_force_sterilization },
  // set Holiday mode by sending 1, off will be 0
  { "SetHolidayMode", set_holiday_mode },
  // set Powerful mode by sending 0 = off, 1 for 30min, 2 for 60min, 3 for 90 min
  { "SetPowerfulMode", set_powerful_mode },
  // set Heat pump operation mode  3 = DHW only, 0 = heat only, 1 = cool only, 2 = Auto, 4 = Heat+DHW, 5 = Cool+DHW, 6 = Auto + DHW
  { "SetOperationMode", set_operation_mode },
  // set DHW temperature by sending desired temperature between 40C-75C
  { "SetDHWTemp", set_DHW_temp },
  // optional PCB
  { "SetHeatCoolMode", set_heat_cool_mode },
  { "SetCompressorState", set_compressor_state },
  { "SetSmartGridMode", set_smart_grid_mode },
  { "SetExternalThermostat1State", set_external_thermostat_1_state },
  { "SetExternalThermostat2State", set_external_thermostat_2_state },
  { "SetDemandControl", set_demand_control },
  { "SetPoolTemp", set_pool_temp },
  { "SetBufferTemp", set_buffer_temp },
  { "SetZ1RoomTemp", set_z1_room_temp },
  { "SetZ1WaterTemp", set_z1_water_temp },
  { "SetZ2RoomTemp", set_z2_room_temp },
  { "SetZ2WaterTemp", set_z2_water_temp },
  { "SetSolarTemp", set_solar_temp }
};

void send_heatpump_command(char* topic, char *msg,bool (*send_command)(byte*, int),void (*log_message)(char*));
void set_optionalpcb(char* topic, char *msg,void (*log_message)(char*));
