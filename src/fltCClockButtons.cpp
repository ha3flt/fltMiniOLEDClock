/****************************************************************************
** Module...: fltCClockButtons.cpp                                         **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include "fltCClockButtons.h"

void CClockButtons::init()
{
    pinMode(D3, INPUT);
    pinMode(D8, INPUT);
}

/****************************************************************************
** Purpose: Raw button functions                                           **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
bool CClockButtons::isButtonPushedRaw(uint8_t buttonNr)
{
    assert(buttonNr < FMOC_NR_OF_BUTTONS);
    bool result;
    switch (buttonNr) {
        case 0:     result = !digitalRead(D3);  break;
        case 1:     result = digitalRead(D8);   break;
        default:    result = false; break;
    }
    return result;
}

bool CClockButtons::isAnyButtonPushedRaw()
{
    bool result = false;
    for (uint8_t nr = 0; nr < FMOC_NR_OF_BUTTONS; ++nr) {
        result |= isButtonPushedRaw(nr);
    }
    return result;
}

bool CClockButtons::isButtonPushed(uint8_t buttonNr)
{
    assert(buttonNr < FMOC_NR_OF_BUTTONS);
    return (ButtonData[buttonNr].debouncedState);
}

bool CClockButtons::isAnyButtonPushed()
{
    bool result = false;
    for (uint8_t nr = 0; nr < FMOC_NR_OF_BUTTONS; ++nr) {
        result |= isButtonPushed(nr);
    }
    return result;
}

/****************************************************************************
** Purpose: Low-level button functions                                     **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockButtons::resetCountersAndSetState(uint8_t buttonNr, ButtonState_t state)
{
    if (ButtonData[buttonNr].state != eNotPressing && state == eNotPressing) {
        ButtonData[buttonNr].debounceSlices = 0;
    }
    BothCounting = false;
    ButtonData[buttonNr].slicesCounter = 0;
    ButtonData[buttonNr].state = state;
}

void CClockButtons::processPushing(uint8_t buttonNr)
{
    switch (ButtonData[buttonNr].state) {
        case eNotPressing:
            // The button has just been pushed
            resetCountersAndSetState(buttonNr, eCounting);
            break;
        case eCounting:
            // Count until the button is released
            ++ButtonData[buttonNr].slicesCounter;
            // Layout specific function
            if (BothCounting) {
                if (isButtonPushed(0) && isButtonPushed(1)) {
                    // If both buttons are still pushed
                    if (ButtonData[0].slicesCounter >= BUTTON_LONG_PRESS_SLICES
                            && ButtonData[1].slicesCounter >= BUTTON_LONG_PRESS_SLICES) {
                        evaluateButtonStates(buttonNr, eBothLongPress);
                        resetCountersAndSetState(0, eBothReleasing);
                        resetCountersAndSetState(1, eBothReleasing);
                    }
                }
                // If the other button is released, nothing should
                //   happen, we will wait for this button to be released
            } else {
                if (isButtonPushed(0)
                        && ButtonData[0].state == eCounting
                        && isButtonPushed(1)
                        && ButtonData[1].state == eCounting) {
                    BothCounting = true;
                } else {
                    if (ButtonData[buttonNr].slicesCounter >= BUTTON_LONG_PRESS_SLICES) {
                        evaluateButtonStates(buttonNr, eLongPress);
                        resetCountersAndSetState(buttonNr, eReleased);
                    }
                }
            }
            break;
        default:
            break;
    }
}

void CClockButtons::processNotPushing(uint8_t buttonNr)
{
    switch (ButtonData[buttonNr].state) {
        case eCounting:
            ++ButtonData[buttonNr].slicesCounter;
            // Long presses are handled in the positive function (processPushing)
            if (BothCounting) {
                // Layout specific function. Wait until the other button is also released
                if (!isButtonPushed(0) && !isButtonPushed(1)) {
                    // Check for how long the buttons were pressed
                    if (ButtonData[0].slicesCounter >= BUTTON_SHORT_PRESS_SLICES
                            && ButtonData[1].slicesCounter >= BUTTON_SHORT_PRESS_SLICES) {
                        evaluateButtonStates(buttonNr, eBothShortPress);
                        resetCountersAndSetState(0, eBothReleasing);
                        resetCountersAndSetState(1, eBothReleasing);
                    } else {
                        // The buttons were released while they were in the counting state
                        // We haven't reached the minimum length of a valid button press
                        resetCountersAndSetState(0, eNotPressing);
                        resetCountersAndSetState(1, eNotPressing);
                    }
                }
                // If the other button is still pushed, nothing should
                //   happen, we are waiting for the other button to be released
            } else {
                // Check for how long the button are pressed
                if (ButtonData[buttonNr].slicesCounter >= BUTTON_SHORT_PRESS_SLICES) {
                    evaluateButtonStates(buttonNr, eShortPress);
                    resetCountersAndSetState(buttonNr, eReleased);
                } else {
                    // The button was released while it was in the counting state
                    // We haven't reached the minimum length of a valid button press
                    resetCountersAndSetState(buttonNr, eNotPressing);
                }
            }
            break;
        case eBothReleasing:
            // Wait indefinitely until both buttons are released
            if (!isButtonPushed(0) && !isButtonPushed(1)) {
                resetCountersAndSetState(0, eReleased);
                resetCountersAndSetState(1, eReleased);
            }
            break;
        case eReleased:
            // This state prevents processing of an early button press
            if (++ButtonData[buttonNr].slicesCounter >= BUTTON_RELEASE_SLICES) {
                // End of waiting time before accepting a new button press
                resetCountersAndSetState(buttonNr, eNotPressing);
            }
            break;
        default:
            break;
    }
}

void CClockButtons::debounceButton(uint8_t buttonNr)
{
    // This algorithm runs forever, repeatedly reading each button. However, it
    //   can easily be modified so that it only fires when a button is pressed.
    // It's designed for a small number of buttons
    if (isButtonPushedRaw(buttonNr)) {
        if (ButtonData[buttonNr].debouncePressingNumerator < BUTTON_DEBOUNCE_SLICES) {
            ++ButtonData[buttonNr].debouncePressingNumerator;
        }
    } else {
        if (ButtonData[buttonNr].debouncePressingNumerator > 0) {
            --ButtonData[buttonNr].debouncePressingNumerator;
        }
    }
    // This is for the very first time when this logic has just been started
    if (ButtonData[buttonNr].debounceSlices < BUTTON_DEBOUNCE_SLICES) {
        ++ButtonData[buttonNr].debounceSlices;
        return;
    }
    // Calculate button state
    ButtonData[buttonNr].debouncedState =
        (ButtonData[buttonNr].debouncePressingNumerator >= BUTTON_DEBOUNCE_VALUE);
}

void CClockButtons::processButtonStates()
{
    // Loop through the buttons
    for (uint8_t nr = 0; nr < FMOC_NR_OF_BUTTONS; ++nr) {
        // This function may change debouncedState
        debounceButton(nr);
        // Check if the button is pushed, change the states accordingly
        if (ButtonData[nr].debouncedState) {
            processPushing(nr);
        } else {
            processNotPushing(nr);
        }
    }
}

/****************************************************************************
** Purpose: The callback function                                          **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockButtons::loop(uint32_t now)
{
    // Units are 10 msec slices
    static uint32_t lastSampleTime = ULONG_MAX / 2;
    auto sampleDiff = (now - lastSampleTime) / 10;
    // LED management
    if (sampleDiff >= 1) {
        lastSampleTime = now;
        processButtonStates();
    }
}

/****************************************************************************
** Purpose: High-level and layout specific button functions                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockButtons::setScanCode(ScanCode_t code)
{
    LastScanCode = code;
}

void CClockButtons::evaluateButtonStates(uint8_t buttonNr, ButtonState_t state)
{
    switch (state) {
        case eShortPress:       setScanCode(buttonNr == 0 ? eButton0Short : eButton1Short); break;
        case eLongPress:        setScanCode(buttonNr == 0 ? eButton0Long : eButton1Long);   break;
        case eBothShortPress:   setScanCode(eBothButtonsShort); break;
        case eBothLongPress:    setScanCode(eBothButtonsLong);  break;
        default: break;
    }
}


const __FlashStringHelper* CClockButtons::getNameForScanCode(ScanCode_t code)
{
    const __FlashStringHelper* p;
    switch (code) {
        case eButtonNone:       p = F("eButtonNone");          break;
        case eButton0Short:     p = F("eButton0Short");        break;
        case eButton1Short:     p = F("eButton1Short");        break;
        case eButton0Long:      p = F("eButton0Long");         break;
        case eButton1Long:      p = F("eButton1Long");         break;
        case eBothButtonsShort: p = F("eBothButtonsShort");    break;
        case eBothButtonsLong:  p = F("eBothButtonsLong");     break;
        default: p = F("???"); break;
    }
    return p;
}

CClockButtons::ScanCode_t CClockButtons::peekScanCode()
{
    return LastScanCode;
}

CClockButtons::ScanCode_t CClockButtons::getScanCode()
{
    if (LastScanCode == eButtonNone) {
        return eButtonNone;
    }
    ScanCode_t scanCode = LastScanCode;
    LastScanCode = eButtonNone;
    return scanCode;
}

/****************************************************************************
** Purpose: Debug function                                                 **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CClockButtons::debugButtons()
{
    Serial.print(F("Button0 pushed: "));
    Serial.print(isButtonPushed(0));
    Serial.print(F(", Button1 pushed: "));
    Serial.print(isButtonPushed(1));
    Serial.print(F(", peek Scan Code: *** \""));
    Serial.print(getNameForScanCode(peekScanCode()));
    Serial.println(F("\" ***"));
}
