/****************************************************************************
** Module...: fltCClockButtons.h                                           **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

#define BUTTON_CYCLE_TIME_MILLIS    (10)
#define BUTTON_CALC_SLICES(millis)  (millis / BUTTON_CYCLE_TIME_MILLIS)

#define BUTTON_DEBOUNCE_PERCENT     (66)
#define BUTTON_DEBOUNCE_SLICES      BUTTON_CALC_SLICES(30)
// result = (n * p + 50) / 100 -- p: percent as an integer, e.g.: 66
#define BUTTON_DEBOUNCE_VALUE       ((BUTTON_DEBOUNCE_SLICES * BUTTON_DEBOUNCE_PERCENT + 50) / 100)

#define BUTTON_SHORT_PRESS_SLICES   BUTTON_CALC_SLICES(40)
#define BUTTON_LONG_PRESS_SLICES    BUTTON_CALC_SLICES(1000)
#define BUTTON_RELEASE_SLICES       BUTTON_CALC_SLICES(160)
//#define BUTTON_MIN_REPEAT_SLICES    BUTTON_CALC_SLICES(500)

class CClockButtons
{
private:
    typedef enum {
        eNotPressing = 0,
        eReleased,
        eBothReleasing,
        eCounting,
        eShortPress,
        eLongPress,
        eBothShortPress,
        eBothLongPress
    } ButtonState_t;

    typedef struct {
        uint16_t debounceSlices = 0;
        uint8_t debouncePressingNumerator = 0;
        bool debouncedState = false;

        uint16_t slicesCounter = 0;

        ButtonState_t state = eNotPressing;
        ButtonState_t lastState = eNotPressing;
    } ButtonData_t;

    // Layout specific functions
    bool BothCounting = false;

public:
    // Layout specific; two buttons - many functions
    typedef enum {
        eButtonNone = 0,
        eButton0Short,
        eButton1Short,
        eButton0Long,
        eButton1Long,
        eBothButtonsShort,
        eBothButtonsLong
    } ScanCode_t;

private:
    ButtonData_t ButtonData[FMOC_NR_OF_BUTTONS];
    ScanCode_t LastScanCode = eButtonNone;

    void resetCountersAndSetState(uint8_t buttonNr, ButtonState_t state);
    void processPushing(uint8_t buttonNr);
    void processNotPushing(uint8_t buttonNr);

    void debounceButton(uint8_t buttonNr);
    void processButtonStates();
    void evaluateButtonStates(uint8_t buttonNr, ButtonState_t state);

public:
    CClockButtons() {};

    void init();

    bool isButtonPushedRaw(uint8_t buttonNr);
    bool isAnyButtonPushedRaw();

    bool isButtonPushed(uint8_t buttonNr);
    bool isAnyButtonPushed();

    void loop(uint32_t now);

    void setScanCode(ScanCode_t code);

    const __FlashStringHelper* getNameForScanCode(ScanCode_t code);
    ScanCode_t peekScanCode();
    ScanCode_t getScanCode();

    void debugButtons();
};
