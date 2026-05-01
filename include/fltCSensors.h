/****************************************************************************
** Module...: fltCSensors.h                                                **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

#define SENSORS_FORCED_UPDATES  (4)

class CSensors
{
private:
    float TemperatureSensorOffset1 = 0.0;
    float TemperatureSensorOffset2 = 0.0;
    float PressureSensorOffset = 0.0;
    uint16_t AltitudeMeters = 0;

    float LastTemperature1 = -127.0;
    float LastTemperature2 = -127.0;

    float MinTemperature1 = 127.0;
    float MaxTemperature1 = -127.0;
    float MinTemperature2 = 127.0;
    float MaxTemperature2 = -127.0;

    void setMinTemperature1(float temperature);
    void setMaxTemperature1(float temperature);
    void setMinTemperature2(float temperature);
    void setMaxTemperature2(float temperature);

    float LastPressure = 999999.0;
    float MinPressure = 999999.0;
    float MaxPressure = -999999.0;

    void setMinPressure(float temperature);
    void setMaxPressure(float temperature);

    uint8_t LastHumidity = 255;
    uint8_t MinHumidity = 100;
    uint8_t MaxHumidity = 0;

    void setMinHumidity(uint8_t humidity);
    void setMaxHumidity(uint8_t humidity);

    int ForcedCounter = SENSORS_FORCED_UPDATES;

public:
    CSensors() {};

    bool init();

    // Temperature
    bool isValidTemperature1(float temperature);
    float getTemperatureSensorOffset1();
    void setTemperatureSensorOffset1(float offset);

    float getTemperature1();
    float getLastTemperature1();
    float getMinTemperature1();
    float getMaxTemperature1();
    void resetMinMaxTemperature1();

    bool isValidTemperature2(float temperature);
    float getTemperatureSensorOffset2();
    void setTemperatureSensorOffset2(float offset);

    float requestTemperature2();
    bool isConversionComplete2();
    float getTemperature2(bool wait = false);
    float getLastTemperature2();
    float getMinTemperature2();
    float getMaxTemperature2();
    void resetMinMaxTemperature2();

    void debugTemperature();

    // Pressure
    uint16_t getAltitudeMeters();
    void setAltitudeMeters(uint16_t altitude);

    float currentLevelToSeaLevelPressure(float pressure);
    float seaLevelPressureToCurrentLevel(float seaLevelPressure);

    float getPressureSensorOffset();
    void setPressureSensorOffset(float offset);
    float calculatePressureSensorOffset(float measuredRawPa, float referenceP0Pa);
    float getRawPressure();
    float getPressure();
    float getLastPressure();
    float getMinPressure();
    float getMaxPressure();
    void resetMinMaxPressure();
    void debugPressure();

    uint8_t getHumidity();
    uint8_t getLastHumidity();
    uint8_t getMinHumidity();
    uint8_t getMaxHumidity();
    void resetMinMaxHumidity();
    void debugHumidity();

    void loop(uint32_t now);
};
