/****************************************************************************
** Module...: fltCSettings.h                                               **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

// Global definitions

class CSettings
{
private:

public:
    CSettings() {};

    void init();

    void setDefaults();
    void loadSettings();
    void saveSettings();

    char* getWifiSsid();
    void setWifiSsid(const char* ssid);
    char* getWifiPassword();
    void setWifiPassword(const char* password);
    uint16_t getWebServerPort();
    void setWebServerPort(uint16_t portNumber);
    char* getFirstNtpServer();
    void setFirstNtpServer(const char* ntpServer);
    int16_t getTimeZoneOffsetMins();
    void setTimeZoneOffsetMins(int16_t timeZoneOffsetMins);
    bool getDisplayHighLow();
    void setDisplayHighLow(bool highLow);
    char* getDeviceLocation();
    void setDeviceLocation(const char* deviceLocation);
    float getSensor1Offset();
    void setSensor1Offset(float offset);
    float getSensor2Offset();
    void setSensor2Offset(float offset);
    int16_t getAltitudeMeters();
    void setAltitudeMeters(int16_t meters);
    int32_t getPressureOffset();
    void setPressureOffset(int32_t offset);
    int16_t getHumidityOffset();
    void setHumidityOffset(int16_t percent);
};
