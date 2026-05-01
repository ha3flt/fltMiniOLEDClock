/****************************************************************************
** Module...: fltCTimeManager.cpp                                          **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "fltCTimeManager.h"

// Local definitions
CTimeManager* TimeManagerInstance = nullptr;

// NTP Servers
#define NTP_SERVERS_NUM     4
static const char NtpServerName[NTP_SERVERS_NUM][32+1] PROGMEM = {
    { "hu.pool.ntp.org" },
    { "pool.ntp.org" },
    { "195.228.75.149" },
    { "80.249.171.204" }
};
static uint8_t NtpServerToTry = 0;

// Daylight Saving Time start and end dates
typedef struct {
    uint8_t yearOffset;
    uint8_t dstStart;
    uint8_t dstEnd;
} DstEntry_t;
// Year offset: DST_BASE_YEAR, see below
static const DstEntry_t DstTable[] PROGMEM = {
    {  0, 30, 26 },     // 2025
    {  1, 29, 25 },     // 2026
    {  2, 28, 31 },     // 2027
    {  3, 26, 29 },     // 2028
    {  4, 25, 28 },     // 2029
    {  5, 31, 27 },     // 2030
    {  6, 30, 26 },     // 2031
    {  7, 28, 31 },     // 2032
    {  8, 27, 30 },     // 2033
    {  9, 26, 29 },     // 2034
    { 10, 25, 28 },     // 2035
    { 11, 30, 26 },     // 2036
    { 12, 29, 25 },     // 2037
    { 13, 28, 31 },     // 2038
    { 14, 27, 30 },     // 2039
    { 15, 25, 28 },     // 2040
    { 16, 31, 27 },     // 2041
    { 17, 30, 26 },     // 2042
    { 18, 29, 25 },     // 2043
    { 19, 27, 30 },     // 2044
    { 20, 26, 29 },     // 2045
    { 21, 25, 28 },     // 2046
    { 22, 31, 27 },     // 2047
    { 23, 29, 25 },     // 2048
    { 24, 28, 31 },     // 2049
    { 25, 27, 30 },     // 2050
    { 26, 26, 29 },     // 2051
    { 27, 31, 27 },     // 2052
    { 28, 30, 26 },     // 2053
    { 29, 29, 25 },     // 2054
    { 30, 28, 31 },     // 2055
    { 31, 26, 29 },     // 2056
    { 32, 25, 28 },     // 2057
    { 33, 31, 27 },     // 2058
    { 34, 30, 26 },     // 2059
    { 35, 28, 31 },     // 2060
    { 36, 27, 30 },     // 2061
    { 37, 26, 29 },     // 2062
    { 38, 25, 28 },     // 2063
    { 39, 30, 26 },     // 2064
    { 40, 29, 25 },     // 2065
    { 41, 28, 31 },     // 2066
    { 42, 27, 30 },     // 2067
    { 43, 25, 28 },     // 2068
    { 44, 31, 27 },     // 2069
    { 45, 30, 26 },     // 2070
    { 46, 29, 25 },     // 2071
    { 47, 27, 30 },     // 2072
    { 48, 26, 29 },     // 2073
    { 49, 25, 28 },     // 2074
    { 50, 31, 27 },     // 2075
    { 51, 29, 25 },     // 2076
    { 52, 28, 31 },     // 2077
    { 53, 27, 30 },     // 2078
    { 54, 26, 29 },     // 2079
    { 55, 31, 27 },     // 2080
    { 56, 30, 26 },     // 2081
    { 57, 29, 25 },     // 2082
    { 58, 28, 31 },     // 2083
    { 59, 26, 29 },     // 2084
    { 60, 25, 28 },     // 2085
    { 61, 31, 27 },     // 2086
    { 62, 30, 26 },     // 2087
    { 63, 28, 31 },     // 2088
    { 64, 27, 30 },     // 2089
    { 65, 26, 29 },     // 2090
    { 66, 25, 28 },     // 2091
    { 67, 30, 26 },     // 2092
    { 68, 29, 25 },     // 2093
    { 69, 28, 31 },     // 2094
    { 70, 27, 30 },     // 2095
    { 71, 25, 28 },     // 2096
    { 72, 31, 27 },     // 2097
    { 73, 30, 26 },     // 2098
    { 74, 29, 25 },     // 2099
    { 75, 28, 31 },     // 2100
};

static const uint8_t DST_TABLE_SIZE PROGMEM = sizeof(DstTable) / sizeof(DstTable[0]);
static const uint16_t DST_BASE_YEAR = 2025;

// Local port to listen for UDP packets
static WiFiUDP Udp;
static uint16_t localPort = 8888;

static const int8_t NTP_PACKET_SIZE PROGMEM = 48;   // NTP time is in the first 48 bytes of message
static byte packetBuffer[NTP_PACKET_SIZE];          // Buffer to hold incoming & outgoing packets

// CTimeManager class
CTimeManager::CTimeManager()
{
    TimeManagerInstance = this;
}

/****************************************************************************
** Purpose: CTimeManager - Init                                            **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CTimeManager::init()
{
    Serial.println(F("Starting UDP"));
    Udp.begin(localPort);

    Serial.print(F("Local port: "));
    Serial.println(Udp.localPort());

    Serial.println(F("Waiting for sync..."));
    setSyncProvider(_getNtpTime);

    setSyncInterval(60*60);
}

/****************************************************************************
** Purpose: NTP functions                                                  **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
// Send an NTP request to the time server at the given address
void CTimeManager::sendNTPpacket(IPAddress& address)
{
    memset(packetBuffer, 0, pgm_read_byte(&NTP_PACKET_SIZE));
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // NTP requests are to port 123
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, pgm_read_byte(&NTP_PACKET_SIZE));
    Udp.endPacket();
}

// Static forwarder with global instance
time_t CTimeManager::_getNtpTime()
{
    return TimeManagerInstance->getNtpTime();
}

// This is a callback function, do not call it directly
time_t CTimeManager::getNtpTime()
{
    time_t now = (time_t)0;
    // NTP server's ip address
    IPAddress ntpServerIP;
    // Discard any previously received packets
    while (Udp.parsePacket() > 0) {
    }
    // Wait for the response longer
    uint32_t beginWait = millis();
    while (millis() - beginWait < 60*1000) {
        // Get a random server from the pool
        Serial.println("Transmit NTP Request");
        char buf[sizeof(NtpServerName[0])];
        strncpy_P(buf, NtpServerName[NtpServerToTry], sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';

        WiFi.hostByName(buf, ntpServerIP);
        // Send a request
        Serial.print(buf);
        Serial.print(F(": "));
        Serial.println(ntpServerIP);
        sendNTPpacket(ntpServerIP);
        // Try
        int size = Udp.parsePacket();
        if (size >= pgm_read_byte(&NTP_PACKET_SIZE)) {
            Serial.println(F("Received an NTP response"));
            Udp.read(packetBuffer, pgm_read_byte(&NTP_PACKET_SIZE));
            // Convert four bytes starting at location 40 
            //   to a long integer that is the time_t type
            unsigned long secsSince1900;
            secsSince1900  = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            time_t utcNow = (time_t)(secsSince1900 - 2208988800UL);
            now = (time_t)(utcNow
                    + getSettings()->getTimeZoneOffsetMins() * SECS_PER_MIN
                    + getDstOffset(utcNow));
            return now;
        }
        // Use another server
        if (++NtpServerToTry >= NTP_SERVERS_NUM) {
            NtpServerToTry = 0;
        }
    }
    Serial.println(F("No NTP Response! Restarting the board..."));
    ESP.restart();
    return 0;
}

// DST switching times is calculated based on the UTC
static int dstOffsetFromDates(uint8_t dstStart, uint8_t dstEnd, int month, int day, int hour)
{
    if (month < 3)  return 0;
    if (month == 3  && day  < dstStart)              return 0;
    if (month == 3  && day == dstStart && hour < 2)  return 0;
    if (month > 3   && month < 10)                   return 3600;
    if (month == 10 && day  < dstEnd)                return 3600;
    if (month == 10 && day == dstEnd && hour < 1)    return 3600;
    return 0;
}

int CTimeManager::getDstOffset(time_t utcTime)
{
    struct tm *t = gmtime(&utcTime);
    int year  = t->tm_year + 1900;
    int month = t->tm_mon  + 1;
    int day   = t->tm_mday;
    int hour  = t->tm_hour;

    // Fallback after 2100: fixed value of 30 March-30 Oct
    if (year > 2100) {
        return dstOffsetFromDates(30, 30, month, day, hour);
    }

    // Seaerch in the table - a safe method instead of direct indexing
    uint8_t targetOffset = (uint8_t)(year - DST_BASE_YEAR);
    uint8_t tableSize    = pgm_read_byte(&DST_TABLE_SIZE);

    for (uint8_t i = 0; i < tableSize; i++) {
        uint8_t yearOffset = pgm_read_byte(&DstTable[i].yearOffset);
        if (yearOffset != targetOffset) continue;

        uint8_t dstStart = pgm_read_byte(&DstTable[i].dstStart);
        uint8_t dstEnd = pgm_read_byte(&DstTable[i].dstEnd);
        return dstOffsetFromDates(dstStart, dstEnd, month, day, hour);
    }

    // Yead not found (before 2025): no DST offset
    return 0;
}

String CTimeManager::getFormattedDate()
{
    char dateBuf[32];
    snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%4u-%02u-%02u"), year(), month(), day());
    return String(dateBuf);
}

String CTimeManager::getFormattedTime(bool withMillis /*= false*/)
{
    char dateBuf[32];
    if (withMillis) {
        snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%02u:%02u:%02u.%03lu"),
            hour(), minute(), second(), millis() % 1000);
    } else {
        snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%02u:%02u:%02u"),
            hour(), minute(), second());
    }
    return String(dateBuf);
}

String CTimeManager::getFormattedTimeHM()
{
    char dateBuf[16];
    snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%02u:%02u"), hour(), minute());
    return String(dateBuf);
}

String CTimeManager::getFormattedDateTime(bool withMillis /*= false*/)
{
    char dateBuf[32];
    // ISO 8601: The 'T' letter can be omitted in unambiguous cases
    if (withMillis) {
        snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%4u-%02u-%02u %02u:%02u:%02u.%03lu"),
            year(), month(), day(), hour(), minute(), second(), millis() % 1000);
    } else {
        snprintf_P(dateBuf, sizeof(dateBuf), PSTR("%4u-%02u-%02u %02u:%02u:%02u"),
            year(), month(), day(), hour(), minute(), second());
    }
    return String(dateBuf);
}

/****************************************************************************
** Purpose: Debug function                                                 **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CTimeManager::DebugNTP()
{
    Serial.print(F("Date/time now: "));
    Serial.println(getFormattedDateTime(true).c_str());
}
