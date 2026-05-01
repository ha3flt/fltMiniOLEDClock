/****************************************************************************
** Module...: main.cpp                                                     **
** Purpose..: Main module with setup() and loop()                          **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include "fltMiniOLEDClock.h"

// Global definitions
const uint16_t FirmwareVersion = FMOC_VERSION_NUMBER;

// Local definitions
static CClockButtons* cButtons = nullptr;
static CClockDisplay* cDisplay = nullptr;
static CMenuDisplay* cMenuDisplay = nullptr;
static CWebServer* cWebServer = nullptr;
static CSettings* cSettings = nullptr;
static CTimeManager* cTimeManager = nullptr;
static CSensors* cSensors = nullptr;
static CSerialCmds* cSerialCmds = nullptr;

static bool LedOn = false;

// Local function prototypes
static void evaluateButtonPresses();

/****************************************************************************
** Purpose: Manage the built-in LED                                        **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void main_ledOn(bool on)
{
    digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
}

void main_flashLed()
{
    main_ledOn(true);
    uint32_t usLastTime = micros();
    while (micros() - usLastTime < 100) {}
    main_ledOn(false);
}

void main_toggleLed()
{
    LedOn = !LedOn;
    main_ledOn(LedOn);
}

/****************************************************************************
** Purpose: The SETUP function                                             **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void setup()
{
    //system_update_cpu_freq(160);    // MHz

	// Init world
    pinMode(LED_BUILTIN, OUTPUT);
    main_ledOn(true);

    // Init serial
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Init world
    //TODO: Handle errors
    cSettings = new CSettings();
    cSettings->init();

    cButtons = new CClockButtons();
    cButtons->init();

    cDisplay = new CClockDisplay();
    cDisplay->init();
    cMenuDisplay = new CMenuDisplay();
    cMenuDisplay->init();

    cSensors = new CSensors();
    (void) cSensors->init();

    cWebServer = new CWebServer();
    cWebServer->init();

    cTimeManager = new CTimeManager();

    cSerialCmds = new CSerialCmds();
    cSerialCmds->flushInput();

    main_ledOn(false);

    cSerialCmds->prompt();
}

/****************************************************************************
** Purpose: Make these objects globally available                          **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
CClockButtons* getButtons()
{
    return cButtons;
}

CClockDisplay* getDisplay()
{
    return cDisplay;
}

CMenuDisplay* getMenuDisplay()
{
    return cMenuDisplay;
}

CSettings* getSettings()
{
    return cSettings;
}

CTimeManager* getTimeManager()
{
    return cTimeManager;
}

CSensors* getSensors()
{
    return cSensors;
}

CWebServer* getWebServer()
{
    return cWebServer;
}

CSerialCmds* getSerialCmds()
{
    return cSerialCmds;
}

/****************************************************************************
** Purpose: Evaluate button presses                                        **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void evaluateButtonPresses()
{
#define DEBUG_KEYBOARD  0
#if !DEBUG_KEYBOARD
    auto scanCode = cButtons->getScanCode();
#else
    auto scanCode = cButtons->peekScanCode();
#endif
    switch (scanCode) {
        case CClockButtons::eButton0Short:
            Serial.println(F("eButton0Short - Try short and long presses"));
            cDisplay->messageBox(F("Try short and"), F("long presses"));
            break;
        case CClockButtons::eButton0Long:
            Serial.println(F("eButton0Long"));
            Serial.println(F("Resetting min/max values of pressure and humidity sensors"));
            cSensors->resetMinMaxPressure();
            cSensors->resetMinMaxHumidity();
            cDisplay->messageBox(F("Press&humid."), F("min/max reset"));
            break;

        case CClockButtons::eButton1Short: {
            Serial.println(F("eButton1Short"));
            Serial.print(F("Min/max. temperature 1: "));
            Serial.print(cSensors->getMinTemperature1());
            Serial.println(cSensors->getMaxTemperature1());
            Serial.print(F("Min/max. temperature 2: "));
            Serial.print(cSensors->getMinTemperature2());
            Serial.println(cSensors->getMaxTemperature2());
            char buf1[24];
            snprintf_P(buf1, sizeof(buf1), PSTR("1:%+02.1f %+02.1f"),
                cSensors->getMinTemperature1(), cSensors->getMaxTemperature1());
            char buf2[24];
            snprintf_P(buf2, sizeof(buf2), PSTR("2:%+02.1f %+02.1f"),
                cSensors->getMinTemperature2(), cSensors->getMaxTemperature2());
            cDisplay->messageBox(buf1, buf2, CClockDisplay::eMsgBoxTwoLinesLeft);
            break;
        }
        case CClockButtons::eButton1Long:
            Serial.println(F("eButton1Long"));
            Serial.println(F("Resetting min/max values of temperature sensors"));
            cSensors->resetMinMaxTemperature1();
            cSensors->resetMinMaxTemperature2();
            cDisplay->messageBox(F("Tempr. min/"), F("max reset"));
            break;

        case CClockButtons::eBothButtonsShort: {
            Serial.println(F("eBothButtonsShort"));
            bool highLow = getSettings()->getDisplayHighLow();
            highLow = !highLow;
            Serial.print(F("Changing display brightness to: "));
            Serial.print(highLow ? F("High") : F("Low"));
            Serial.println(F("."));
            cDisplay->dimScreen(!highLow);
            getSettings()->setDisplayHighLow(highLow);
            if (highLow) {
                cDisplay->messageBox(F("Brightness"), F("High"));
            } else {
                cDisplay->messageBox(F("Brightness"), F("Low"));
            }
            break;
        }
        case CClockButtons::eBothButtonsLong:
            Serial.println(F("eBothButtonsLong - Reset config to defaults"));
            cDisplay->messageBox(F("Resetting"), F("configuration"));
            cSettings->setDefaults();
            cSettings->saveSettings();
            Serial.println(F("Settings memory erased"));
            break;

        default:
            break;
    }
#if DEBUG_KEYBOARD
    if (scanCode > CClockButtons::eButtonNone) {
        cButtons->debugButtons();
    }
    (void) cButtons->getScanCode();
#endif
}

/****************************************************************************
** Purpose: The LOOP function                                              **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void loop()
{
    uint32_t now = millis();

    // Lower level processing
    cButtons->loop(now);
    cSensors->loop(now);
    cWebServer->loop(now);
    cDisplay->loop(now);
    // Higher level functions
    evaluateButtonPresses();
    cSerialCmds->loop();

    // LED management
    static uint32_t lastLedTime = ULONG_MAX / 2;
    auto diffLed = (now - lastLedTime) / 1000;
    if (diffLed >= 1) {
        lastLedTime = now;
        main_flashLed();
    }
    delay(10);
}
