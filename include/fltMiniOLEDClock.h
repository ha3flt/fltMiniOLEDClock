/****************************************************************************
** Module...: fltOledStation.h                                             **
** Purpose..: Header of main module with setup() and loop()                **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include <Arduino.h>

// Global definitions
extern const uint16_t FirmwareVersion;

#define FMOC_NR_OF_BUTTONS      (2)

#define FMOC_NTP_SERVER         "pool.ntp.org"

#include <cstdint>
#include <limits.h>
#include "fltCClockButtons.h"
#include "fltCClockDisplay.h"
#include "fltCLogManager.h"
#include "fltCMenuDisplay.h"
#include "fltCSensors.h"
#include "fltCSettings.h"
#include "fltCTimeManager.h"
#include "fltCWebServer.h"
#include "fltCSerialCmds.h"

// Hardware info for a WeMos D1 mini (ESP8266) module
//
// WeMos D1 <--> OLED (I2C), BMP085, buttons
//    3V3           VCC
//    GND           GND
//     D4           PCB LED     (GPIO2)
//     D2           SDA         (GPIO4)
//     D1           SCL         (GPIO5)
//     D3           Button1     (GPIO0)
//     D8           Button2     (GPIO15)
//     D5           DS1820      (GPIO14)

// Global functions - make these objects globally available
// Forward declarations
class CClockButtons;
class CClockDisplay;
class CMenuDisplay;
class CWebServer;
class CSettings;
class CTimeManager;
//TODO: static/singleton
class CSensors;
// Functions
extern CClockButtons* getButtons();
extern CClockDisplay* getDisplay();
extern CMenuDisplay* getMenuDisplay();
extern CSettings* getSettings();
extern CTimeManager* getTimeManager();
extern CSensors* getSensors();
extern CWebServer* getWebServer();

// Utility functions
extern void main_ledOn(bool on);
extern void main_flashLed();
extern void main_toggleLed();
