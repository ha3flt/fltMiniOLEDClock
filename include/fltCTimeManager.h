/****************************************************************************
** Module...: fltCTimeManager.h                                            **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include <TimeLib.h>
#include "IPAddress.h"
#include "fltMiniOLEDClock.h"

// Global definitions

class CTimeManager
{
public:
    CTimeManager();
    void init();

private:
    void sendNTPpacket(IPAddress& address);
    static time_t _getNtpTime();
    time_t getNtpTime();

    int getDstOffset(time_t utcTime);

public:
    String getFormattedDate();
    String getFormattedTime(bool withMillis = false);
    String getFormattedTimeHM();
    String getFormattedDateTime(bool withMillis = false);
    void DebugNTP();
};
