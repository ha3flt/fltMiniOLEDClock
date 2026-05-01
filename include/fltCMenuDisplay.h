/****************************************************************************
** Module...: fltCMenuDisplay.h                                            **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

class CClockButtons;

class CMenuDisplay
{
public:
    typedef enum {
        eNone = 0,
        eEscape,
        eMenuOrItemUp,
        eMenuOrItemDown,
        eEnter,
        eLast
    } MenuAction_t;

private:
    CClockButtons* Buttons = nullptr;

public:
    CMenuDisplay() {}

    void init();

    void DrawMenu();
    void ClearMenu();
    void DoAction(MenuAction_t action);
};
