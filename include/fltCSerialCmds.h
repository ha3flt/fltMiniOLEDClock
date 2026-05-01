/****************************************************************************
** Module...: fltCSerialCmds.h                                             **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

// Global definitions

class CSerialCmds
{
private:
    bool getParams(char** param1, char** param2 = nullptr);
    void normalizeQuotedInputLine();
    void restoreQuotedSpaces(char* text);

    void printHelp();
    void resetCommand();
    void getDateTimeCommand();
    void getSetDeviceLocationCommand();
    void eraseSettingsCommand();
    void getSetWifiSsidCommand();
    void getSetWifiPasswordCommand();
    void printWifiStatusCommand();
    void getSetWebServerPortCommand();
    void getSetFirstNtpServerCommand();
    void getSetTimeZoneOffsetCommand();
    void printMemoryStatusCommand();
    void getSetDisplayHighLowCommand();
    void resetMinMaxValuesCommand();
    void getSetAltitudeCommand();
    void getSetTemperatureOffset1Command();
    void getSetTemperatureOffset2Command();
    void getSetPressureOffset();
    void getSetHumidityOffset();
    void printAllInfoCommand();

public:
    CSerialCmds() {};

    void prompt();

    void flushInput();
    bool isAvailable();

    bool readInput(bool wait = false);
    void clearInput();

    bool processCommand();
    void loop();
};
