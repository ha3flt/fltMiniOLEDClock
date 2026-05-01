/****************************************************************************
** Module...: fltLogManager.h                                              **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include "fltMiniOLEDClock.h"

// Global definitions
typedef enum {
    eLogLevelNone = 0,
    eLogLevelFatal,
    eLogLevelError,
    eLogLevelWarning,
    eLogLevelInfo,
    eLogLevelDebug,
    eLogLevelVerbose,
    eLogLevelAll = 255
}  ProjectLogLevels_t;

class CLogManager
{
private:
    ProjectLogLevels_t projectLogLevel = eLogLevelNone;

public:
    CLogManager() {};

    ProjectLogLevels_t getLogLevel();
    void setLogLevel(ProjectLogLevels_t level);

    bool isLevelActive(ProjectLogLevels_t level);
};
