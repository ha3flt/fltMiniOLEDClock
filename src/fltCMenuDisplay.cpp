/****************************************************************************
** Module...: fltCMenuDisplay.cpp                                          **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include "fltCMenuDisplay.h"

const char MenuArray[3][12+1] PROGMEM = {
    "Menu 1",
    "Menu 2",
    "Menu 3"
};

// CMenuDisplay class
void CMenuDisplay::init()
{
    getButtons()->init();
}

void CMenuDisplay::DrawMenu()
{
}

void CMenuDisplay::ClearMenu()
{
}

void CMenuDisplay::DoAction(MenuAction_t action)
{
}
