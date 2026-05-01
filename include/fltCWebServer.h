/****************************************************************************
** Module...: fltCWebServer.h                                              **
** Purpose..: Implements HTTPS webserver with parsing and responses        **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#pragma once
#include <ESP8266WiFi.h>
#include "fltMiniOLEDClock.h"

#define WEBSERVER_REQUEST_READ_TIMEOUT_MSEC     (2000)

class CWebServer
{
public:
    typedef enum {
        eDisconnected = 0,
        eConnecting,
        eConnected,
        eDisconnecting,
        eFailure = 255
    } ConnectionState_t;

private:
    void sendMainPage(BearSSL::WiFiClientSecure& incoming);
    void sendMainData(BearSSL::WiFiClientSecure& incoming);
    void sendShortMainPage(BearSSL::WiFiClientSecure& incoming);
    void sendClearMinMax(BearSSL::WiFiClientSecure& incoming);
    void sendBackgroundPicture(BearSSL::WiFiClientSecure& incoming);
    void sendMatrixFont(BearSSL::WiFiClientSecure& incoming);
    void sendFavIcon(BearSSL::WiFiClientSecure& incoming);
    void sendErrorPage(BearSSL::WiFiClientSecure& incoming, int code, const char* msg);

    void handleRequest(BearSSL::WiFiClientSecure& incoming, const String& url);
    String parseUrlFromRequestLine(const String& line);
    bool readHttpRequest(BearSSL::WiFiClientSecure& incoming, String& reqLine);

public:
    CWebServer() {};

    void init();

    bool isWifiConnected();
    String getLocalIP();
    int8_t getRSSI();
    int16_t getPort();

    void loop(uint32_t now);
};
