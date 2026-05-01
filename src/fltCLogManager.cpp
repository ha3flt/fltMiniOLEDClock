/****************************************************************************
** Module...: fltLogManager.cpp                                            **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include "fltCLogManager.h"

// Local definitions
static CLogManager LogMgr;

// CClockDisplay class
ProjectLogLevels_t CLogManager::getLogLevel()
{
    return LogMgr.projectLogLevel;
}

void CLogManager::setLogLevel(ProjectLogLevels_t level)
{
    //TODO: Check value
    LogMgr.projectLogLevel = level;
}

bool CLogManager::isLevelActive(ProjectLogLevels_t level)
{
    return (level <= LogMgr.projectLogLevel);
}
