/****************************************************************************
** Module...: fltCClockDisplay.cpp                                         **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include <StreamString.h>
#include "fltCClockDisplay.h"

// Local definitions

// Bitmaps as characters
const uint64_t DegreeChar[] = {
    0x00000000040a0400
};
const int16_t DegreeCharLen = sizeof(DegreeChar) / 8;

const uint64_t HpaChar[] = {
    0xc5a5cb0939483800
};
const int16_t HpaCharLen = sizeof(HpaChar) / 8;

const uint64_t SmallMinusChar[] = {
    0x00000000000f0000
};
const int16_t SmallMinusCharLen = sizeof(SmallMinusChar) / 8;

/****************************************************************************
** Purpose: CClockDisplay class - Init                                     **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::init()
{
    Display = new SH1106Wire(0x3c, SDA, SCL);
    Display->init();
    Display->normalDisplay();
    //Display->invertDisplay();

    Display->flipScreenVertically();
    Display->setFont(ArialMT_Plain_10);

    bool highLow = getSettings()->getDisplayHighLow();
    dimScreen(!highLow);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
/*CClockDisplay::DisplayPageMode_t CClockDisplay::getDisplayPage()
{
    return DisplayPage;
}*/

/*void CClockDisplay::setDisplayPage(DisplayPageMode_t displayPage)
{
    switch (displayPage) {
        eMainPage:
            DisplayPage = displayPage;
            break;
        eSecondPage:
            DisplayPage = displayPage;
            break;
        default:
            break;
    }
}*/

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::dimScreen(bool dim)
{
    uint8_t brightness = (dim) ? 96 : 255;
    Display->setBrightness(brightness);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::wifiConnectInfo(const DisplayConnState_t state)
{
    //Display->setColor(WHITE);
    //Display->drawRect(10, 15, 108, 39);

    Display->setFont(ArialMT_Plain_16);
    Display->setTextAlignment(TEXT_ALIGN_CENTER);

    Display->setColor(WHITE);
    Display->fillRect(11, 16, 106, 37);

    switch (state) {
        case eClearWindow:
        default:
            Display->setColor(WHITE);
            Display->fillRect(11, 16, 106, 37);
            break;
        case eDisconnected:
            Display->setColor(BLACK);
            Display->drawString(64, 25, F("Disconnected"));
            Display->drawString(65, 25, F("Disconnected"));
            break;
        case eConnecting:
            Display->setColor(WHITE);
            Display->fillRect(11, 12, 106, 41);
            Display->setColor(BLACK);
            Display->drawString(64, 13, F("Connecting..."));
            Display->drawString(65, 13, F("Connecting..."));
            Display->drawString(64, 31, F("Button to exit"));
            Display->drawString(65, 31, F("Button to exit"));
            break;
        case eConnected:
            Display->setColor(BLACK);
            Display->drawString(64, 25, F("Connected"));
            Display->drawString(65, 25, F("Connected"));
            break;
        case eDisconnecting:
            Display->setColor(BLACK);
            Display->drawString(64, 25, F("Disconnect..."));
            Display->drawString(65, 25, F("Disconnect..."));
            break;
        case eFailure:
            Display->setColor(BLACK);
            Display->drawString(64, 25, F("FAILED!"));
            Display->drawString(65, 25, F("FAILED!"));
            break;
    }
    Display->setColor(WHITE);
    Display->display();
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
const uint32_t CClockDisplay::getMessageBoxDelaySec()
{
    return MessageBoxDelaySec;
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::setMessageBoxDelaySec(const uint16_t seconds)
{
    if (seconds >= 1 && seconds <= 5) {
        MessageBoxDelaySec = seconds;
    }
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::messageBox(
    const String& messageString1,
    const String& messageString2,
    MessageBoxType_t msgType /*= eMsgBoxTwoLines*/)
{
    Display->setFont(ArialMT_Plain_16);
    Display->setTextAlignment(TEXT_ALIGN_CENTER);

    int xPos = 64;  // Center

    switch (msgType) {
        default:
            break;
        case eMsgBoxOneLine:
            Display->setColor(WHITE);
            Display->fillRect(11, 16, 106, 37);
            Display->setColor(BLACK);
            Display->drawString(xPos, 25, messageString1.c_str());
            Display->drawString(xPos + 1, 25, messageString1.c_str());
            break;

        case eMsgBoxTwoLinesLeft:
            Display->setTextAlignment(TEXT_ALIGN_LEFT);
            xPos = 13;
        case eMsgBoxTwoLines:
            Display->setColor(WHITE);
            Display->fillRect(11, 12, 106, 41);
            Display->setColor(BLACK);
            Display->drawString(xPos, 13, messageString1.c_str());
            Display->drawString(xPos + 1, 13, messageString1.c_str());
            Display->drawString(xPos, 31, messageString2.c_str());
            Display->drawString(xPos + 1, 31, messageString2.c_str());
            break;

        case eMsgBoxMultiLine:
            Display->setColor(WHITE);
            Display->fillRect(11, 12, 106, 41);
            Display->setColor(BLACK);
            Display->drawStringMaxWidth(13, 20, 102, messageString1.c_str());
            break;
    }
    Display->setColor(WHITE);
    Display->display();

    MessageBoxDelayCounterSec = MessageBoxDelaySec;
}

void CClockDisplay::messageBox(const String& messageString)
{
    messageBox(messageString, "", eMsgBoxOneLine);
}

void CClockDisplay::messageBox(
        const __FlashStringHelper* messageString1,
        const __FlashStringHelper* messageString2,
        MessageBoxType_t msgType /*= eMsgBoxTwoLines*/)
{
    messageBox(String(messageString1), String(messageString2), msgType);
}

void CClockDisplay::messageBox(const __FlashStringHelper* messageString)
{
    messageBox(String(messageString));
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::startScreenUpdate()
{  
    Display->clear();
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::endScreenUpdate()
{  
    // Write the buffer to the display
    Display->display();
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayFrame()
{
    Display->setColor(WHITE);
    Display->drawRect(0, 0, 128, 64);

    // Separator 1-2
    Display->drawRect(1, 19, 126, 1);
    Display->drawRect(62, 20, 1, 13);

    // Separator 2-3 and vertical separators
    Display->drawRect(1, 32, 126, 1);
    Display->drawRect(47, 33, 1, 18);
    Display->drawRect(80, 33, 1, 18);

    // Separator 3-4
    Display->drawRect(1, 51, 127, 1);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayBattery(uint8_t percent)
{
    // Battery indicator
    Display->drawRect(4, 4, (53 * percent) / 100, 2);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayDateTime(uint32_t now)
{
    // First line: date + time
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_LEFT);
    Display->drawString(4, 6, getTimeManager()->getFormattedDate());

    Display->setFont(ArialMT_Plain_16);
    Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Display->drawString(125, 1, getTimeManager()->getFormattedTime());
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayWeekDayHumidityPressure()
{
    // Second line
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_LEFT);
    Display->drawString(4, 20, dayStr(weekday()));

    // Humidity
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Display->drawString(87, 20, F("00%"));

    // Pressure
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Display->drawString(114, 20, String(getSensors()->getPressure() / 100.0, 0).c_str());
    Display->drawXbm(116, 22, HpaCharLen * 8, 8, (const uint8_t *)HpaChar);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayTemperature12()
{
    // Third line - temperatures
    float temp = getSensors()->getLastTemperature1();
    float temp2 = getSensors()->getLastTemperature2();

    Display->setFont(ArialMT_Plain_16);
    Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    if (temp < 0.0) {
        Display->drawXbm(4, 40, SmallMinusCharLen * 8, 8, (const uint8_t *)SmallMinusChar);
    }
    Display->drawString(40, 33, String(abs(temp), 1).c_str());
    Display->drawXbm(40, 35, DegreeCharLen * 8, 8, (const uint8_t *)DegreeChar);

    //Display->setFont(ArialMT_Plain_16);
    //Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    if (temp2 < 0.0) {
        Display->drawXbm(84, 40, SmallMinusCharLen * 8, 8, (const uint8_t *)SmallMinusChar);
    }
    Display->drawString(120, 33, String(abs(temp2), 1).c_str());
    Display->drawXbm(120, 35, DegreeCharLen * 8, 8, (const uint8_t *)DegreeChar);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayMiddle()
{
    // Still the third line - Test message
    // ArialMT_Plain_16 at line 33 or
    //   ArialMT_Plain_10 at line 3?
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_CENTER);
    char version[10];
    Display->drawStringf(64, 35, version, "v%03u", FirmwareVersion);
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::displayNetRelated()
{
    if (!getWebServer()) return;

    // Fourth line
    // Net related / ...?
    // Don't use texts with gqj, etc.
    Display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_LEFT);
    Display->drawString(4, 51, getWebServer()->getLocalIP().c_str());
    //display->drawString(4, 51, "288.288.288.288");
    //display->setFont(ArialMT_Plain_10);
    Display->setTextAlignment(TEXT_ALIGN_RIGHT);
    int8_t rssi = getWebServer()->getRSSI();
    if (rssi >= -50) {
        Display->drawString(125, 51, F("IIIII"));
    } else if (rssi >= -60) {
        Display->drawString(125, 51, F("IIII"));
    } else if (rssi >= -70) {
        Display->drawString(125, 51, F("III"));
    } else if (rssi >= -80) {
        Display->drawString(125, 51, F("II"));
    } else {
        Display->drawString(125, 51, F("I"));
    }
}

/****************************************************************************
** Purpose:                                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockDisplay::loop(uint32_t now)
{
    if ((now - LastDisplayTimeMsec) / 1000 >= 1) {
        LastDisplayTimeMsec = now;

        // Do not update the display while a message box is "open"
        if (MessageBoxDelayCounterSec > 0) {
            --MessageBoxDelayCounterSec;
        } else {
            startScreenUpdate();
            displayFrame();
            displayBattery(50);
            displayDateTime(now);
            displayWeekDayHumidityPressure();
            displayTemperature12();
            displayMiddle();
            // Flash text(s)
            /*if (LastDisplayMode != 0) {
                LastDisplayMode = 0;
                displayMiddle();
            } else {
                LastDisplayMode = 1;
            }*/
            displayNetRelated();
            endScreenUpdate();
        }
    }
}
