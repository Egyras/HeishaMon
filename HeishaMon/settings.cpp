#include "settings.h"

const char* Settings::hotspot_mode = "hotspot_mode";
const char* Settings::wifi_ssid = "wifi_ssid";
const char* Settings::wifi_password = "wifi_password";
const char* Settings::wifi_hostname = "wifi_hostname";
const char* Settings::ota_password = "ota_password";
const char* Settings::mqtt_topic_base = "mqtt_topic_base";
const char* Settings::mqtt_server = "mqtt_server";
const char* Settings::mqtt_port = "mqtt_port";
const char* Settings::mqtt_username = "mqtt_username";
const char* Settings::mqtt_password = "mqtt_password";
const char* Settings::use_1wire = "use_1wire";
const char* Settings::use_s0 = "use_s0";
const char* Settings::listenonly = "listenonly";
const char* Settings::logMqtt = "logMqtt";
const char* Settings::logHexdump = "logHexdump";
const char* Settings::logSerial1 = "logSerial1";
const char* Settings::optionalPCB = "optionalPCB";
const char* Settings::waitTime = "waitTime";
const char* Settings::waitDallasTime = "waitDallasTime";
const char* Settings::updateAllTime = "updateAllTime";
const char* Settings::updataAllDallasTime = "updataAllDallasTime";
const char* Settings::s0_1_gpio = "s0_1_gpio";
const char* Settings::s0_1_ppkwh = "s0_1_ppkwh";
const char* Settings::s0_1_interval = "s0_1_interval";
const char* Settings::s0_2_gpio = "s0_2_gpio";
const char* Settings::s0_2_ppkwh = "s0_2_ppkwh";
const char* Settings::s0_2_interval = "s0_2_interval";

const char* enabled = "enabled";
const char* disabled = "disabled";

const char* toString(bool value)
{
    return value ? enabled : disabled;
}

bool toBool(const char* value)
{
    return value == enabled;
}

void SettingsStruct::fromJson(DynamicJsonDocument& jsonDoc)
{
    if (jsonDoc[Settings::hotspot_mode]) hotspot_mode = (HotspotMode)atoi(jsonDoc[Settings::hotspot_mode]);
    if (jsonDoc[Settings::wifi_ssid]) strncpy(wifi_ssid, jsonDoc[Settings::wifi_ssid], sizeof(wifi_ssid));
    if (jsonDoc[Settings::wifi_password]) strncpy(wifi_password, jsonDoc[Settings::wifi_password], sizeof(wifi_password));
    if (jsonDoc[Settings::wifi_hostname]) strncpy(wifi_hostname, jsonDoc[Settings::wifi_hostname], sizeof(wifi_hostname));
    if (jsonDoc[Settings::ota_password]) strncpy(ota_password, jsonDoc[Settings::ota_password], sizeof(ota_password));
    if (jsonDoc[Settings::mqtt_topic_base]) strncpy(mqtt_topic_base, jsonDoc[Settings::mqtt_topic_base], sizeof(mqtt_topic_base));
    if (jsonDoc[Settings::mqtt_server]) strncpy(mqtt_server, jsonDoc[Settings::mqtt_server], sizeof(mqtt_server));
    if (jsonDoc[Settings::mqtt_port]) strncpy(mqtt_port, jsonDoc[Settings::mqtt_port], sizeof(mqtt_port));
    if (jsonDoc[Settings::mqtt_username]) strncpy(mqtt_username, jsonDoc[Settings::mqtt_username], sizeof(mqtt_username));
    if (jsonDoc[Settings::mqtt_password]) strncpy(mqtt_password, jsonDoc[Settings::mqtt_password], sizeof(mqtt_password));
    use_1wire = toBool(jsonDoc[Settings::use_1wire]);
    use_s0 = toBool(jsonDoc[Settings::use_s0]);
    listenonly = toBool(jsonDoc[Settings::listenonly]);
    logMqtt = toBool(jsonDoc[Settings::logMqtt]);
    logHexdump = toBool(jsonDoc[Settings::logHexdump]);
    logSerial1 = toBool(jsonDoc[Settings::logSerial1]);
    optionalPCB = toBool(jsonDoc[Settings::optionalPCB]);
    if (jsonDoc[Settings::waitTime]) waitTime = jsonDoc[Settings::waitTime];
    if (waitTime < 5) waitTime = 5;
    if (jsonDoc[Settings::waitDallasTime]) waitDallasTime = jsonDoc[Settings::waitDallasTime];
    if (waitDallasTime < 5) waitDallasTime = 5;
    if (jsonDoc[Settings::updateAllTime]) updateAllTime = jsonDoc[Settings::updateAllTime];
    if (updateAllTime < waitTime) updateAllTime = waitTime;
    if (jsonDoc[Settings::updataAllDallasTime]) updataAllDallasTime = jsonDoc[Settings::updataAllDallasTime];
    if (updataAllDallasTime < waitDallasTime) updataAllDallasTime = waitDallasTime;
    if (jsonDoc[Settings::s0_1_gpio]) s0Settings[0].gpiopin = jsonDoc[Settings::s0_1_gpio];
    if (jsonDoc[Settings::s0_1_ppkwh]) s0Settings[0].ppkwh = jsonDoc[Settings::s0_1_ppkwh];
    if (jsonDoc[Settings::s0_1_interval]) s0Settings[0].lowerPowerInterval = jsonDoc[Settings::s0_1_interval];
    if (jsonDoc[Settings::s0_2_gpio]) s0Settings[1].gpiopin = jsonDoc[Settings::s0_2_gpio];
    if (jsonDoc[Settings::s0_2_ppkwh]) s0Settings[1].ppkwh = jsonDoc[Settings::s0_2_ppkwh];
    if (jsonDoc[Settings::s0_2_interval] ) s0Settings[1].lowerPowerInterval = jsonDoc[Settings::s0_2_interval];
}

void SettingsStruct::toJson(DynamicJsonDocument& jsonDoc)
{
    char hotspotModeBuf[4];
    jsonDoc[Settings::hotspot_mode] = itoa(hotspot_mode, hotspotModeBuf, 10);
    jsonDoc[Settings::wifi_hostname] = wifi_hostname;
    jsonDoc[Settings::wifi_password] = wifi_password;
    jsonDoc[Settings::wifi_ssid] = wifi_ssid;
    jsonDoc[Settings::ota_password] = ota_password;
    jsonDoc[Settings::mqtt_topic_base] = mqtt_topic_base;
    jsonDoc[Settings::mqtt_server] = mqtt_server;
    jsonDoc[Settings::mqtt_port] = mqtt_port;
    jsonDoc[Settings::mqtt_username] = mqtt_username;
    jsonDoc[Settings::mqtt_password] = mqtt_password;
    jsonDoc[Settings::use_1wire] = toString(use_1wire);
    jsonDoc[Settings::use_s0] = toString(use_s0);
    jsonDoc[Settings::listenonly] = toString(listenonly);
    jsonDoc[Settings::logMqtt] = toString(logMqtt);
    jsonDoc[Settings::logHexdump] = toString(logHexdump);
    jsonDoc[Settings::logSerial1] = toString(logSerial1);
    jsonDoc[Settings::optionalPCB] = toString(optionalPCB);
    jsonDoc[Settings::waitTime] = waitTime;
    jsonDoc[Settings::waitDallasTime] = waitDallasTime;
    jsonDoc[Settings::updateAllTime] = updateAllTime;
    jsonDoc[Settings::updataAllDallasTime] = updataAllDallasTime;
}
