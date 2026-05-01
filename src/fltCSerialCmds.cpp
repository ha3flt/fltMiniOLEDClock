/****************************************************************************
** Module...: fltCSerialCmds.cpp                                           **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include "fltCSerialCmds.h"
#include <stdlib.h>

// Local definitions
#define SERIAL_INPUTLINE_SIZE   (39+1)

// CSerialCmds class
static char inputLine[SERIAL_INPUTLINE_SIZE];
static uint16_t inputLineLength = 0;

void CSerialCmds::flushInput()
{
    while (Serial.available()) {}
}

bool CSerialCmds::isAvailable()
{
    return Serial.available();
}

// Get available characters and process them
bool CSerialCmds::readInput(bool wait /*= false*/)
{
    for (;;) {
        while (!Serial.available()) {
            if (!wait) {
                return false;
            }
        }
        // Read next char
        int c = Serial.read();
        if (c <= 0 || c == '\r') {
            continue;
        }
        // Delete
        if (c == 127 || c == 8) { 
            if (inputLineLength > 0) { 
                // Have you ever used octal numbers? :-)
                Serial.print(F("\010 \010"));
                --inputLineLength; 
            }
            continue;
        }
        // Skip new inputLine chars
        if (c == '\n') {
            break;
        }
        // Print the read character and add to the inputLine
        Serial.print((char)c);
        if (inputLineLength < sizeof(inputLine)) {
            inputLine[inputLineLength] = c;
        }
        ++inputLineLength;
    }

    // The inputLine is ready
    if (inputLineLength < sizeof(inputLine)) {
        inputLine[inputLineLength] = '\0';
    } else {
        inputLine[sizeof(inputLine) - 1] = '\0';
    }

    // Remove quotes. Print and return
    normalizeQuotedInputLine();
    Serial.println();

    return inputLineLength > 0;
}

void CSerialCmds::clearInput()
{
    inputLineLength = 0;
    inputLine[0] = '\0';
}

bool CSerialCmds::getParams(char** param1, char** param2 /*= nullptr*/)
{
    if (param1 == nullptr) {
        return false;
    }
    // command + space + param1 + space + param2, 1+1+20+1+20
    if (inputLineLength > 43) {
        return false;
    }
    bool result = false;
    if (inputLineLength > 2 && inputLine[1] == ' ') {
        // First param
        char* params = strtok(inputLine + 2, " ");
        if (params != nullptr) {
            restoreQuotedSpaces(params);
            result = true;
            *param1 = params;
            // Second param
            if (param2 != nullptr) {
                params = strtok(NULL, " ");
                if (params != nullptr) {
                    restoreQuotedSpaces(params);
                    *param2 = params;
                }
            }
        }
    }
    return result;
}

// Remove quotes and protect spaces inside quoted segments from strtok.
void CSerialCmds::normalizeQuotedInputLine()
{
    bool inQuotes = false;
    uint16_t writePos = 0;
    uint16_t readLen = inputLineLength;

    if (readLen >= sizeof(inputLine)) {
        readLen = sizeof(inputLine) - 1;
    }

    for (uint16_t readPos = 0; readPos < readLen; ++readPos) {
        char ch = inputLine[readPos];
        if (ch == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        if (inQuotes && ch == ' ') {
            ch = '\t';
        }
        if (writePos < sizeof(inputLine) - 1) {
            inputLine[writePos++] = ch;
        }
    }

    inputLineLength = writePos;
    inputLine[writePos] = '\0';
}

void CSerialCmds::restoreQuotedSpaces(char* text)
{
    if (text == nullptr) {
        return;
    }
    for (; *text != '\0'; ++text) {
        if (*text == '\t') {
            *text = ' ';
        }
    }
}

/****************************************************************************
** Purpose: Serial commands                                                **
** Param's:                                                                **
** Returns:                                                                **
****************************************************************************/
void CSerialCmds::prompt()
{
    Serial.print(F("\n>"));
}

void CSerialCmds::printHelp()
{
    Serial.print(F("fltMiniOLEDClock v"));
    Serial.println(FirmwareVersion);
    Serial.println(F("-------------------------------------------------------"));
    Serial.println(F("Quotes can be used for strings containing spaces."));
    Serial.println(F("-------------------------------------------------------"));
    Serial.println(F("?                 This help"));
    Serial.println(F("d [h|l]           Get/set display high/low"));
    Serial.println(F("e <13857>         Erase settings memory"));
    Serial.println(F("l [location]      Get/set device location (<=23 chars)"));
    Serial.println(F("s [ssid]          Get/set Wifi SSID (<=31 chars)"));
    Serial.println(F("o [password]      Get/set Wifi password (<=31 chars)"));
    Serial.println(F("r                 Reset"));
    Serial.println(F("w <13857>         Print Wifi status with password"));
    Serial.println(F("n [server]        Get/set first NTP server (N.A.)"));
    Serial.println(F("z [offset mins]   Get/set Time Zone offset in minutes"));
    Serial.println(F("t                 Get date and time"));
    Serial.println(F("p [port]          Get/set web server port"));
    Serial.println(F("1 [celsius*10]    Get/set temp. sensor 1 offset * 10"));
    Serial.println(F("2 [celsius*10]    Get/set temp. sensor 2 offset * 10"));
    Serial.println(F("a [meters]        Get/set altitude in meters"));
    Serial.println(F("h [Pa]            Get/set pressure offset in Pa"));
    Serial.println(F("u [percent]       Get/set humidity offset in percent"));
    Serial.println(F("m                 Memory status"));
    Serial.println(F("i [13857]         Print all info [with password]"));
    Serial.println(F("x                 Reset min/max values"));
    Serial.println(F("-------------------------------------------------------"));
}

void CSerialCmds::resetCommand()
{
    Serial.println(F("*** Resetting ***"));
    ESP.restart();
}

void CSerialCmds::getDateTimeCommand()
{
    Serial.println(F("Getting date/time..."));
    String dateTime = getTimeManager()->getFormattedDateTime(true);
    Serial.print(F("Date/time: "));
    Serial.println(dateTime);
}

void CSerialCmds::eraseSettingsCommand()
{
    Serial.println(F("Erasing memory..."));
    char* param1;
    // Instead of yes/no questions...
    if (getParams(&param1) && strcmp(param1, "13857") == 0) {
        getSettings()->setDefaults();
        getSettings()->saveSettings();
        Serial.println(F("Settings memory erased"));
    } else {
        Serial.println(F("Invalid erase code. Use: e <13857>"));
    }
}

void CSerialCmds::getSetDeviceLocationCommand()
{
    Serial.println(F("Get/set Device Location..."));
    char* param1;
    if (getParams(&param1)) {
        getSettings()->setDeviceLocation(param1);
    }
    Serial.print(F("Device Location: \""));
    Serial.print(getSettings()->getDeviceLocation());
    Serial.println(F("\""));
}

void CSerialCmds::getSetWifiSsidCommand()
{
    Serial.println(F("Get/set Wifi SSID..."));
    char* param1;
    if (getParams(&param1)) {
        getSettings()->setWifiSsid(param1);
    }
    Serial.print(F("Wifi SSID: "));
    Serial.println(getSettings()->getWifiSsid());
}

void CSerialCmds::getSetWifiPasswordCommand()
{
    Serial.println(F("Get/set Wifi password..."));
    char* param1;
    if (getParams(&param1)) {
        getSettings()->setWifiPassword(param1);
    }
    Serial.print(F("Wifi password: "));
    Serial.println(getSettings()->getWifiPassword());
}

void CSerialCmds::printWifiStatusCommand()
{
    Serial.println(F("Get Wifi status..."));
    char* param1;
        // Instead of yes/no questions...
    if (getParams(&param1) && strcmp(param1, "13857") == 0) {
        WiFi.printDiag(Serial);
    } else {
        Serial.println(F("Invalid code. Use: w <13857>"));
    }
}

void CSerialCmds::getSetWebServerPortCommand()
{
    Serial.println(F("Get/set web server port..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        unsigned long port = strtoul(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0' ||
                port == 0 || port > 65535UL) {
            Serial.println(F("Invalid port. Range: 1..65535"));
            return;
        }
        getSettings()->setWebServerPort((uint16_t)port);
        Serial.println(F("To apply the port change, RESTART the device"));
    }
    Serial.print(F("Web server port: "));
    Serial.println(getSettings()->getWebServerPort());
}

void CSerialCmds::getSetFirstNtpServerCommand()
{
    Serial.println(F("Get/set first NTP server..."));
    Serial.println(F("Not implemented yet"));
    /*char* param1;
    if (getParams(&param1)) {
        getSettings()->setFirstNtpServer(param1);
    }
    Serial.print(F("First NTP server: "));
    Serial.println(getSettings()->getFirstNtpServer());*/
}

void CSerialCmds::getSetTimeZoneOffsetCommand()
{
    Serial.println(F("Get/set Time Zone offset minutes..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        long offset = strtol(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid Time Zone offset"));
            return;
        }
        getSettings()->setTimeZoneOffsetMins((int16_t)offset);
    } else {
        Serial.print(F("Time Zone offset: "));
        Serial.print(getSettings()->getTimeZoneOffsetMins());
        Serial.println(F(" minutes"));
    }
}

void CSerialCmds::printMemoryStatusCommand()
{
    Serial.println(F("Get memory status..."));
    Serial.print(F("Free heap: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes, max. free: "));
    Serial.print(ESP.getMaxFreeBlockSize());
    Serial.println(F(" bytes"));
}

void CSerialCmds::getSetDisplayHighLowCommand()
{
    Serial.println(F("Get/set display brightness..."));
    char* param1;
    if (getParams(&param1)
        && (param1[0]== 'l' || param1[0] == 'L'
            || param1[0]== 'h' || param1[0] == 'H')) {
        bool highLow = (param1[0]== 'l' || param1[0] == 'L');
        getDisplay()->dimScreen(highLow);
        getSettings()->setDisplayHighLow(highLow);
    }
    Serial.print(F("Display brightness: "));
    bool highLow = !getSettings()->getDisplayHighLow();
    Serial.println(highLow ? F("High") : F("Low"));
}

void CSerialCmds::resetMinMaxValuesCommand()
{
    Serial.println(F("Reset min/max. values..."));
    getSensors()->resetMinMaxTemperature1();
    getSensors()->resetMinMaxTemperature2();
    getSensors()->resetMinMaxPressure();
    getSensors()->resetMinMaxHumidity();
}

void CSerialCmds::getSetTemperatureOffset1Command()
{
    Serial.println(F("Get/set temperature offset for sensor 1..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        unsigned long offset = strtoul(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid temperature offset"));
            return;
        }
        getSensors()->setTemperatureSensorOffset1((float)offset / 10.0);
    } else {
        Serial.print(F("Temperature offset 1: "));
        Serial.print(getSensors()->getTemperatureSensorOffset1());
        Serial.println(F(" *C"));
    }
}


void CSerialCmds::getSetTemperatureOffset2Command()
{
    Serial.println(F("Get/set temperature offset for sensor 2..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        unsigned long offset = strtoul(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid temperature offset"));
            return;
        }
        getSensors()->setTemperatureSensorOffset2((float)offset / 10.0);
    } else {
        Serial.print(F("Temperature offset 2: "));
        Serial.print(getSensors()->getTemperatureSensorOffset2());
        Serial.println(F(" *C"));
    }
}

void CSerialCmds::getSetAltitudeCommand()
{
    Serial.println(F("Get/set altitude in meters..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        unsigned long alt = strtoul(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid altitude"));
            return;
        }
        getSensors()->setAltitudeMeters((uint16_t)alt);
    } else {
        Serial.print(F("Altitude: "));
        Serial.print(getSensors()->getAltitudeMeters());
        Serial.println(F(" meters"));
    }
}

void CSerialCmds::getSetPressureOffset()
{
    Serial.println(F("Get/set pressure offset in Pa..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        long offset = strtol(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid pressure offset"));
            return;
        }
        getSettings()->setPressureOffset(offset);
    } else {
        Serial.print(F("Pressure offset: "));
        Serial.print(getSettings()->getPressureOffset());
        Serial.println(F(" Pa"));
    }
}

void CSerialCmds::getSetHumidityOffset()
{
    Serial.println(F("Get/set humidity offset in percent..."));
    char* param1;
    if (getParams(&param1)) {
        char* endPtr = nullptr;
        long offset = strtol(param1, &endPtr, 10);
        if (param1[0] == '\0' || endPtr == nullptr || *endPtr != '\0') {
            Serial.println(F("Invalid humidity offset"));
            return;
        }
        getSettings()->setHumidityOffset((int16_t)offset);
    } else {
        Serial.print(F("Pressure offset: "));
        Serial.print(getSettings()->getHumidityOffset());
        Serial.println(F(" Pa"));
    }
}

void CSerialCmds::printAllInfoCommand()
{
    bool allInfo = false;
    char* param1;
    // Instead of yes/no questions...
    if (getParams(&param1) && strcmp(param1, "13857") == 0) {
        allInfo = true;
    }
    Serial.println(F("Get all info..."));
    Serial.println(F("***"));
    String dateTime = getTimeManager()->getFormattedDateTime(true);
    Serial.print(F("Date/time: "));
    Serial.println(dateTime);
    Serial.print(F("Wifi SSID: "));
    Serial.println(getSettings()->getWifiSsid());
    Serial.print(F("Wifi password: "));
    if (allInfo) {
        Serial.println(getSettings()->getWifiPassword());
    } else {
        Serial.println(F("********"));
    }
    Serial.print(F("Web server port: "));
    Serial.println(getSettings()->getWebServerPort());
    Serial.print(F("First NTP server: "));
    Serial.println(getSettings()->getFirstNtpServer());
    if (allInfo) {
        Serial.println(F("*** Wifi status ***"));
        printWifiStatusCommand();
    }
    Serial.println(F("*** Misc. ***"));
    Serial.print(F("Display brightness: "));
    Serial.println(getSettings()->getDisplayHighLow() ? F("High") : F("Low"));
    printMemoryStatusCommand();
}

/****************************************************************************
** Purpose: Process commands read from the serial inputLine                **
** Param's:                                                                **
** Returns:                                                                **
****************************************************************************/
bool CSerialCmds::processCommand()
{
    inputLine[0] = toupper(inputLine[0]);
    
    bool result = false;
    if (inputLine[0] == '?' || inputLine[0] == '\0') {
        printHelp();
        result = true;
    }
    else if (inputLine[0] == 'R') {
        resetCommand();
        result = true;
    }
    else if (inputLine[0] == 'T') {
        getDateTimeCommand();
        result = true;
    }
    else if (inputLine[0] == 'E') {
        eraseSettingsCommand();
        result = true;
    }
    else if (inputLine[0] == 'L') {
        getSetDeviceLocationCommand();
        result = true;
    }
    else if (inputLine[0] == 'S') {
        getSetWifiSsidCommand();
        result = true;
    }
    else if (inputLine[0] == 'O') {
        getSetWifiPasswordCommand();
        result = true;
    }
    else if (inputLine[0] == 'W') {
        printWifiStatusCommand();
        result = true;
    }
    else if (inputLine[0] == 'P') {
        getSetWebServerPortCommand();
        result = true;
    }
    else if (inputLine[0] == 'N') {
        getSetFirstNtpServerCommand();
        result = true;
    }
    else if (inputLine[0] == 'Z') {
        getSetTimeZoneOffsetCommand();
        result = true;
    }
    else if (inputLine[0] == 'M') {
        printMemoryStatusCommand();
        result = true;
    }
    else if (inputLine[0] == 'D') {
        getSetDisplayHighLowCommand();
        result = true;
    }
    else if (inputLine[0] == 'X') {
        resetMinMaxValuesCommand();
        result = true;
    }
    else if (inputLine[0] == '1') {
        getSetTemperatureOffset1Command();
        result = true;
    }
    else if (inputLine[0] == '2') {
        getSetTemperatureOffset2Command();
        result = true;
    }
    else if (inputLine[0] == 'A') {
        getSetAltitudeCommand();
        result = true;
    }
    else if (inputLine[0] == 'H') {
        getSetPressureOffset();
        result = true;
    }
    else if (inputLine[0] == 'U') {
        getSetHumidityOffset();
        result = true;
    }
    else if (inputLine[0] == 'I') {
        printAllInfoCommand();
        result = true;
    }
    clearInput();
    return result;
}

/****************************************************************************
** Purpose:                                                                **
** Param's:                                                                **
** Returns:                                                                **
****************************************************************************/
void CSerialCmds::loop()
{
    if (isAvailable()) {
        if (readInput()) {
            bool ok = processCommand();
            if (!ok) {
                printHelp();
            }
            prompt();
        }
    }
}
