/****************************************************************************
** Module...: fltCClockDisplay.h                                           **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include <SH1106Wire.h>
#include "fltMiniOLEDClock.h"

#define DISPLAY_MESSAGE_BOX_DELAY_SEC   (3)

class CClockDisplay
{
public:
    typedef enum {
        eMainPage = 0,
        eSecondPage
    } DisplayPageMode_t;

    typedef enum {
        eClearWindow = 0,
        eDisconnected,
        eConnecting,
        eConnected,
        eDisconnecting,
        eFailure = 255
    } DisplayConnState_t;

    typedef enum {
        eMsgBoxOneLine = 0,
        eMsgBoxTwoLines,
        eMsgBoxTwoLinesLeft,
        eMsgBoxMultiLine,
        eDefault = -1
    } MessageBoxType_t;

private:
    SH1106Wire* Display = nullptr;
    DisplayPageMode_t DisplayPage = eMainPage;

    uint32_t MessageBoxDelaySec = DISPLAY_MESSAGE_BOX_DELAY_SEC;
    uint32_t MessageBoxDelayCounterSec = 0;

    uint32_t LastDisplayTimeMsec = ULONG_MAX / 2;
    //uint32_t LastDisplayMode = 1;

public:
    CClockDisplay() {};

    void init();
    /*DisplayPageMode_t getDisplayPage();
    void setDisplayPage(DisplayPageMode_t DisplayPage);*/
    void dimScreen(bool dim);

    void wifiConnectInfo(const DisplayConnState_t state);

    const uint32_t getMessageBoxDelaySec();
    void setMessageBoxDelaySec(const uint16_t seconds);

    void messageBox(const String& messageString1,
        const String& messageString2, MessageBoxType_t msgType = eMsgBoxTwoLines);
    void messageBox(const String& messageString);
    void messageBox(const __FlashStringHelper* messageString1,
        const __FlashStringHelper* messageString2, MessageBoxType_t msgType = eMsgBoxTwoLines);
    void messageBox(const __FlashStringHelper* messageString);

    void startScreenUpdate();
    void endScreenUpdate();
    
    void displayFrame();
    void displayBattery(uint8_t percent);
    void displayDateTime(uint32_t now);
    void displayWeekDayHumidityPressure();
    void displayTemperature12();
    void displayMiddle();
    void displayNetRelated();

    void loop(uint32_t now);
};
