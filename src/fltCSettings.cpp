/****************************************************************************
** Module...: fltCSettings.cpp                                             **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include <EEPROM.h>
#include "fltCSettings.h"

// Local definitions
const uint32_t MAGIC_NUMBER = 0xE9BD13A5;

typedef struct {
    uint32_t magic;
    char ssid[31+1];
    char password[31+1];
    uint16_t portNumber;
    char ntpServer[23+1];
    int16_t timeZoneOffsetMins;
    bool displayHighLow;
    char deviceLocation[23+1];
    float sensor1Offset;
    float sensor2Offset;
    int16_t altitudeMeters;
    int32_t pressureOffset;
    int16_t humidityOffset;
} FirmwareSettings_t;

static FirmwareSettings_t config;

// CSettings class
void CSettings::init()
{
    memset(&config, 0, sizeof(FirmwareSettings_t));
    EEPROM.begin(sizeof(FirmwareSettings_t));
    loadSettings();
}

const char ntpServer[] PROGMEM = FMOC_NTP_SERVER;

void CSettings::setDefaults()
{
    memset(&config, 0, sizeof(FirmwareSettings_t));
    config.magic = MAGIC_NUMBER;
    config.portNumber = FMOC_HTTPS_SERVER_PORT;
    strncpy_P(config.ntpServer, ntpServer, sizeof(config.ntpServer));
    config.timeZoneOffsetMins = FMOC_TIME_ZONE_OFFSET_MINS;
    config.displayHighLow = true;
    strncpy(config.deviceLocation,
        FMOC_DEFAULT_DEVICE_LOCATION, sizeof(config.deviceLocation) - 1);
    config.deviceLocation[sizeof(config.deviceLocation) - 1] = '\0';
    config.altitudeMeters = FMOC_DEFAULT_ALTITUDE_METERS;
}

void CSettings::loadSettings()
{
    EEPROM.get(0, config);
    if (config.magic != MAGIC_NUMBER) {
        config.magic = MAGIC_NUMBER;
        setDefaults();
        saveSettings();
    }
}

void CSettings::saveSettings()
{
    EEPROM.put(0, config);
    EEPROM.commit();
}

/****************************************************************************
** Purpose:                                                                **
** Param's:                                                                **
** Returns:                                                                **
****************************************************************************/
char* CSettings::getWifiSsid()
{
    return config.ssid;
}

void CSettings::setWifiSsid(const char* ssid)
{
    //TODO: Check for invalid characters, and length
    strncpy(config.ssid, ssid, sizeof(config.ssid));
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    saveSettings();
}

char* CSettings::getWifiPassword()
{
    return config.password;
}

void CSettings::setWifiPassword(const char* password)
{
    //TODO: Check for invalid characters, and length
    strncpy(config.password, password, sizeof(config.password));
    config.password[sizeof(config.password) - 1] = '\0';
    saveSettings();
}

uint16_t CSettings::getWebServerPort()
{
    return config.portNumber;
}

void CSettings::setWebServerPort(uint16_t portNumber)
{
    config.portNumber = portNumber;
    saveSettings();
}

char* CSettings::getFirstNtpServer()
{
    return config.ntpServer;
}

void CSettings::setFirstNtpServer(const char* ntpServer)
{
    strncpy(config.ntpServer, ntpServer, sizeof(config.ntpServer));
    config.ntpServer[sizeof(config.ntpServer) - 1] = '\0';
    saveSettings();
}

int16_t CSettings::getTimeZoneOffsetMins()
{
    return config.timeZoneOffsetMins;
}

void CSettings::setTimeZoneOffsetMins(int16_t timeZoneOffsetMins)
{
    config.timeZoneOffsetMins = timeZoneOffsetMins;
    saveSettings();
}

bool CSettings::getDisplayHighLow()
{
    return config.displayHighLow;
}

void CSettings::setDisplayHighLow(bool highLow)
{
    config.displayHighLow = highLow;
    saveSettings();
}

char* CSettings::getDeviceLocation()
{
    return config.deviceLocation;
}

void CSettings::setDeviceLocation(const char* deviceLocation)
{
    strncpy(config.deviceLocation, deviceLocation, sizeof(config.deviceLocation));
    config.deviceLocation[sizeof(config.deviceLocation) - 1] = '\0';
    saveSettings();
}

float CSettings::getSensor1Offset()
{
    return config.sensor1Offset;
}

void CSettings::setSensor1Offset(float offset)
{
    config.sensor1Offset = offset;
    saveSettings();
}

float CSettings::getSensor2Offset()
{
    return config.sensor2Offset;
}

void CSettings::setSensor2Offset(float offset)
{
    config.sensor2Offset = offset;
    saveSettings();
}

int16_t CSettings::getAltitudeMeters()
{
    return config.altitudeMeters;
}

void CSettings::setAltitudeMeters(int16_t meters)
{
    config.altitudeMeters = meters;
    saveSettings();
}

int32_t CSettings::getPressureOffset()
{
    return config.pressureOffset;
}

void CSettings::setPressureOffset(int32_t offset)
{
    config.pressureOffset = offset;
    saveSettings();
}

int16_t CSettings::getHumidityOffset()
{
    return config.humidityOffset;
}

void CSettings::setHumidityOffset(int16_t percent)
{
    config.humidityOffset = percent;
    saveSettings();
}
