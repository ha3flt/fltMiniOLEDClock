/****************************************************************************
** Module...: fltCWebServer.cpp                                            **
** Purpose..: Implements HTTPS webserver with parsing and responses        **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#undef min
#include <algorithm>
#include <StreamString.h>
#include "fltCWebServer.h"

// Files on the server available to be served
#include "fltMOLC_bkgnd-planets.h"
#include "fltMOLC_calottery-machine-otf-woff2.h"
#include "fltMOLC_favicon_ico.h"
#include "fltMOLC_main_html_header.h"

// Local definitions
#include "fltWifiConfig.h"

#if FMOC_DEBUG_WEB_SERVER
    #define DBGPRINTLN      Serial.println
    #define DBGPRINT        Serial.print
    #define DBGPRINTF_P     Serial.printf_P
#else
    #define DBGPRINTLN
    #define DBGPRINT
    #define DBGPRINTF_P
#endif

// The HTTPS server
//static BearSSL::WiFiServerSecure server(FMOC_HTTPS_SERVER_PORT);
static uint8_t serverBuf[sizeof(BearSSL::WiFiServerSecure)];
static BearSSL::WiFiServerSecure* server = nullptr;

#define USE_EC    // Enable Elliptic Curve signed cert
#ifndef USE_EC
// The server's private key which must be kept secret
// Do NOT change formatting here, do not add spaces, etc.
static const char server_private_key[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDJblrg47vF3qlE
NMRM7uG8QwE6v/AKpxOL+CLb/32s+dW9Psgf+oZKJgzGkYUoJdWpLitTmTZeykAs
Sq7Iax5Rq/mGqyAc7oJAUUAupfNRU0KwkD1XqtpQWEFoiqoIqZbOZ4CRX5q8z/MN
BH1aPVBMKaL33uwknkgJBzxwZJ2+uGKxRJt8+koj1CXgUCk5lEAEEG5kqE326MjN
O/c4gBqulBV8AIoq6/trY3apTS7FEOiN47qh1PVzoBm/oGVwXvoZAZOj7+gGGo91
sBC5oHJy5Y2BOcNB3opTNXQTiK3Z80b5wc3iQS+h83qAfHwhs6tfAW22WkAf+jtt
x8KdRWFNAgMBAAECggEAPd+jFL9/d1lc/zGCNuuN9YlTgFti/bKyo2UWOCOz1AVu
LVJyoLgQtggYFoqur1Vn2y7uaiB+/gD8U16hb7jPuGCuJjq8g4aUBfOvVmTtZ8a+
joPQA/TcWJ+zf8xQTJbjVwWeDYmje2oZC5+cbbK1zp9fiuoz+U+RawyI+TE+700i
ESCmsKFIHy2Ifruva8HgcPYIPpZ9zLxJj0Dii+WDs7zM9h2dzO4HfImSG/DPmgoV
ydo9IcrUE7KoMLa8Uo7u1b2h6BnTn7GfYiMSUsYcYR3CnpDBknBWjZMwrV0uqv9q
TbVc4QXt+c1q89HDg7BIJaOAzbCvJfgAfXUqZyqwQQKBgQD5ENFjicUzCqPw7fOy
Q5Z8GeUbIJ5urT1MheAq7SPd2kK8TsO3hUjNC0LLNSyKPs6gsYaIiObO3wDGeZZk
xeHBhrUVaz2nIjI7TrnCUpMDOrdxcPr4bc+ifV5YT4W3OFBWQ9chQEx3Nm3DbiX4
fpno34AiFrJF791JkTPFj9OIUQKBgQDPCgcae1pQr77q+GL5Q2tku3RrE4cWtExf
m8DzAb4Vxe3EhPz8bVr+71rqr/KqNfG1uKE3sT0fhB6VMTkHTOQU13jDrvpPUS3W
Vg8cVr5/+iiyF0xb+W8LQ+GVdR5xnMPSZHUtXyURvtzT4nnTAlAtN7lEytX9BzbX
xhltOOwGPQKBgA/Y/BnDSGLpCGlqGpl7J3YaB7PkLXCJYV8fHZZdpGyXWKu2r0lc
F7fEQanAZmcde/RJl2/UlisPkXMPhXxAAw9XTOph+nhJ+rw/VB6DNot8DvQO5kks
Y4vJQlmIJc/0q1fx1RxuhO8I7Y8D0TKwi4Z/wh1pKEq+6mul649kiWchAoGAWn8B
l9uvIHGRO9eSO23ytTcSrfL9Kzln4KqN7iom0hGP2kRe6F9MVP5+ePKrWSb3Hf0z
ysoX83ymeYPob352e32rda04EA9lv7giJrrrzbikrSNt5w3iMcRcCB4HTpW9Kmtq
pIhgBZ+tmpf1s/vg28LtoloeqtjKagpW9tzYnekCgYAZFZ84EGqS9SHw5LELgGY4
mQLMwbYZ6wBMA2PlqYi/17hoAVWz37mLDjtWDB4ir78QMoGbesQVtK9W/4vzmez4
ZLKlffdL5tCtA08Gq9aond1z83Xdnh1UjtwHIJvJPc/AoCFW1r5skv/G6acAk6I2
Zs0aiirNGTEymRX4rw26Qg==
-----END PRIVATE KEY-----
)EOF";

// The server's public certificate which must be shared
static const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDUTCCAjmgAwIBAgIJAOcfK7c3JQtnMA0GCSqGSIb3DQEBCwUAMD8xCzAJBgNV
BAYTAkFVMQ0wCwYDVQQIDAROb25lMQ0wCwYDVQQKDAROb25lMRIwEAYDVQQDDAlF
U1BTZXJ2ZXIwHhcNMTgwMzE0MTg1NTQ1WhcNMjkwNTMxMTg1NTQ1WjA/MQswCQYD
VQQGEwJBVTENMAsGA1UECAwETm9uZTENMAsGA1UECgwETm9uZTESMBAGA1UEAwwJ
RVNQU2VydmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyW5a4OO7
xd6pRDTETO7hvEMBOr/wCqcTi/gi2/99rPnVvT7IH/qGSiYMxpGFKCXVqS4rU5k2
XspALEquyGseUav5hqsgHO6CQFFALqXzUVNCsJA9V6raUFhBaIqqCKmWzmeAkV+a
vM/zDQR9Wj1QTCmi997sJJ5ICQc8cGSdvrhisUSbfPpKI9Ql4FApOZRABBBuZKhN
9ujIzTv3OIAarpQVfACKKuv7a2N2qU0uxRDojeO6odT1c6AZv6BlcF76GQGTo+/o
BhqPdbAQuaBycuWNgTnDQd6KUzV0E4it2fNG+cHN4kEvofN6gHx8IbOrXwFttlpA
H/o7bcfCnUVhTQIDAQABo1AwTjAdBgNVHQ4EFgQUBEk8LqgV+sMjdl/gpP1OlcNW
14EwHwYDVR0jBBgwFoAUBEk8LqgV+sMjdl/gpP1OlcNW14EwDAYDVR0TBAUwAwEB
/zANBgkqhkiG9w0BAQsFAAOCAQEAO1IrqW21KfzrxKmtuDSHdH5YrC3iOhiF/kaK
xXbigdtw6KHW/pIhGiA3BY5u+d5eVuHTR5YSwIbbRvOjuoNBATAw/8f5mt5Wa+C3
PDpLNxDys561VbCW45RMQ0x5kybvDYi0D1R/grqZ18veuFSfE6QMJ/mzvr575fje
8r5Ou0IZOYYF8cyqG5rA4U7BYXEnH44VgwlpkF8pitPsnyUWaAYqE0KnZ0qw0Py4
HCkfGJNlNOOamnr6KakVlocwKY0SdxcLoXSs5ogTQvTSrAOjwcm1RA0hOCXr8f/f
UsQIIGpPVh1plR1vYNndDeBpRJSFkoJTkgAIrlFzSMwNebU0pg==
-----END CERTIFICATE-----
)EOF";
#else
static const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIB0zCCAXqgAwIBAgIJALANi2eTiGD/MAoGCCqGSM49BAMCMEUxCzAJBgNVBAYT
AkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRn
aXRzIFB0eSBMdGQwHhcNMTkwNjExMjIyOTU2WhcNMjAwNjEwMjIyOTU2WjBFMQsw
CQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJu
ZXQgV2lkZ2l0cyBQdHkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAExIkZ
w7zjk6TGcScff1PAehuEGmKZTf8VfnkjyJH0IbBgZibZ+qwYGBEnkz4KpKv7TkHo
W+j7F5EMcLcSrUIpy6NTMFEwHQYDVR0OBBYEFI6A0f+g0HyxUT6xrbVmRU79urbj
MB8GA1UdIwQYMBaAFI6A0f+g0HyxUT6xrbVmRU79urbjMA8GA1UdEwEB/wQFMAMB
Af8wCgYIKoZIzj0EAwIDRwAwRAIgWvy7ofQTGZMNqxUfe4gjtkU+C9AkQtaOMW2U
5xFFSvcCICvcGrQpoi7tRTq8xsXFmr8MYWgQTpVAtj6opXMQct/l
-----END CERTIFICATE-----
)EOF";

// The server's private key which must be kept secret
static const char server_private_key[] PROGMEM = R"EOF(
-----BEGIN EC PARAMETERS-----
BggqhkjOPQMBBw==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIKyLR9/NT7ZdWM+2rklehveuk+jyIHJ+P8ZUQ392HOYvoAoGCCqGSM49
AwEHoUQDQgAExIkZw7zjk6TGcScff1PAehuEGmKZTf8VfnkjyJH0IbBgZibZ+qwY
GBEnkz4KpKv7TkHoW+j7F5EMcLcSrUIpyw==
-----END EC PRIVATE KEY-----
)EOF";
#endif

#define CACHE_SIZE 1  // Number of sessions to cache. (was: 5)
#define USE_CACHE     // Enable SSL session caching.
                      // Caching SSL sessions shortens the length of the SSL handshake.
                      // You can see the performance improvement by looking at the
                      // Network tab of the developer tools of your browser.
//#define DYNAMIC_CACHE // Whether to dynamically allocate the cache.

#if defined(USE_CACHE) && defined(DYNAMIC_CACHE)
 // Dynamically allocated cache.
 static BearSSL::ServerSessions serverCache(CACHE_SIZE);
#elif defined(USE_CACHE)
 // Statically allocated cache.
 static ServerSession store[CACHE_SIZE];
 static BearSSL::ServerSessions serverCache(store, CACHE_SIZE);
#endif

static const BearSSL::X509List serverCertList(server_cert);
static const BearSSL::PrivateKey serverPrivKey(server_private_key);

static StreamString PageHeader;
static StreamString PageContent;

/****************************************************************************
** Purpose: CWebServer class - Init                                        **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::init()
{
    PageContent.reserve(2048);

    char* ssid = getSettings()->getWifiSsid();
    if (ssid[0] != '\0') {
        strcpy(ssid, getSettings()->getWifiSsid());
    } else {
        strcpy(ssid, STASSID);
    }

    char* pass = getSettings()->getWifiPassword();
    if (pass[0] != '\0') {
        strcpy(pass, getSettings()->getWifiPassword());
    } else {
        strcpy(pass, STAPSK);
    }

    int16_t portNumber = getSettings()->getWebServerPort();
    server = new (serverBuf) BearSSL::WiFiServerSecure(portNumber);

    // We start by connecting to a WiFi network
    getDisplay()->wifiConnectInfo(CClockDisplay::eConnecting);
    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        main_toggleLed();
        delay(250);
        Serial.print(F("."));
        main_ledOn(true);
        // We should continue to make console commands working
        // Only raw functions work during the initialization
        if (getButtons()->isAnyButtonPushedRaw()) {
            delay(20);
            if (getButtons()->isAnyButtonPushedRaw()) {
                break;
            }
        }
    }
    main_ledOn(true);
    Serial.println(F(""));

    if (WiFi.status() != WL_CONNECTED) {
        getDisplay()->wifiConnectInfo(CClockDisplay::eFailure);
        Serial.println(F("WiFi is NOT connected!"));
        wifi_set_sleep_type(MODEM_SLEEP_T);
        delay(2000);
        return;
    }

    //TODO: Error handling: print WiFi.status() and reset or trying to connect forever
    getDisplay()->wifiConnectInfo(CClockDisplay::eConnected);
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());

    //TODO: Are these buffer sizes optimal?
    server->setBufferSizes(4096, 4096);

    // Attach the server private cert/key combo
#ifndef USE_EC
    server->setRSACert(&serverCertList, &serverPrivKey);
#else
    server->setECCert(&serverCertList, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, &serverPrivKey);
#endif

    // Set the server's cache
#if defined(USE_CACHE)
    server->setCache(&serverCache);
#endif

    getTimeManager()->init();
    Serial.println(F("Time manager inited"));

    // Actually start accepting connections
    server->begin();
    Serial.print(F("Server started, port: "));
    Serial.println(portNumber);

    wifi_set_sleep_type(LIGHT_SLEEP_T);
}

/****************************************************************************
** Purpose: Utility functions                                              **
****************************************************************************/
bool CWebServer::isWifiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String CWebServer::getLocalIP()
{
    if (WiFi.status() != WL_CONNECTED) {
        return "???";
    }
    return WiFi.localIP().toString();
}

int8_t CWebServer::getRSSI()
{
    if (WiFi.status() != WL_CONNECTED) {
        return -128;
    }
    return WiFi.RSSI();
}

int16_t CWebServer::getPort()
{
    return server->port();
}

/****************************************************************************
** HTTP request header constants                                           **
****************************************************************************/
static const char HTTP_REQUEST_HEADER_TEXT[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "\r\n";

static const char HTTP_REQUEST_HEADER_TEXT_ERR[] PROGMEM =
    "HTTP/1.0 %d %s\r\n"
    "Connection: close\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "\r\n";

static const char HTTP_REQUEST_HEADER_JSON[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-store\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: application/json; charset=utf-8\r\n"
    "\r\n";

static const char HTTP_REQUEST_HEADER_JPEG[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: image/jpeg\r\n"
    "\r\n";

static const char HTTP_REQUEST_HEADER_FONT[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: font/woff2\r\n"
    "\r\n";

static const char HTTP_REQUEST_HEADER_ICON[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: image/x-icon\r\n"
    "\r\n";

/****************************************************************************
** Main dashboard HTTP body constant                                       **
****************************************************************************/
static const char MAIN_HTML_BODY[] PROGMEM =
    "<body>\r\n"
    "<div class=\"wrap\">\r\n"
        "<div class=\"clock-display my-display-red mid-digits\">\r\n"
            "<div class=\"my-display-bg-glass\"></div>\r\n"
            "<div>\r\n"
                "<div class=\"my-display-numbers-blue\">\r\n"
                    "<div id=\"location\" class=\"norm-digits\"><span id=\"location_str\">%s</span></div>\r\n"
                "</div>\r\n"
                "<div class=\"my-display-numbers-red norm-digits\" style=\"margin-top: 1rem;\">\r\n"
                    "<div id=\"time\" class=\"big-digits\" style=\"margin-top: 0.5rem; margin-bottom: 1rem;\">%s</div>\r\n"
                    "<div id=\"date\" style=\"line-height: 1rem;\">%s</div>\r\n"
                    "<div id=\"day\" style=\"margin-top: 0; padding-top: 0;\">%s</div>\r\n"
                "</div>\r\n"
                "<div class=\"clock-grid my-display-numbers-green\" style=\"margin-top: 0.5rem;\">\r\n"
                    "<div><span id=\"temp1\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                    "<div class=\"clock-range small-digits right\">\r\n"
                        "<div><span id=\"tmax1\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                        "<div><span id=\"tmin1\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                    "</div>\r\n"
                "</div>\r\n"
                "<div class=\"norm-digits my-display-numbers-green\" style=\"margin-top: 0.5rem;\">\r\n"
                    "<span id=\"pressure\">%u</span><span class=\"unit\">hPa</span>\r\n"
                    "<span id=\"humidity\" style=\"margin-left: 0.5rem;\">%u%%</span>\r\n"
                "</div>\r\n"
                "<div class=\"clock-grid my-display-numbers-yellow\" style=\"margin-top: 0.5rem;\">\r\n"
                    "<div><span id=\"temp2\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                    "<div class=\"clock-range small-digits right\">\r\n"
                        "<div><span id=\"tmax2\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                        "<div><span id=\"tmin2\">%+2.1f</span><span class=\"unit\">&deg;C</span></div>\r\n"
                    "</div>\r\n"
                "</div>\r\n"
                "<div class=\"my-display-numbers-blue\" style=\"margin-top: 0.5rem;\">\r\n"
                    "<div id=\"version\" class=\"small-digits\">Version: v<span id=\"version_value\">%03u</span></div>\r\n"
                "</div>\r\n"
                "<button id=\"clear-minmax-btn\" class=\"mini-circle-btn\" type=\"button\" aria-label=\"Clear min/max sensor data\">\r\n"
                    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"0.85rem\" height=\"0.85rem\" viewBox=\"0 0 24 24\" fill=\"none\"\r\n"
                            "stroke=\"currentColor\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" style=\"filter: blur(0.4px);\">\r\n"
                        "<circle cx=\"12.5\" cy=\"12.5\" r=\"8\" stroke-dasharray=\"44 12\" stroke-dashoffset=\"24\"/>\r\n"
                        "<path d=\"M5 4v5h5\"/>\r\n"
                    "</svg>\r\n"
                "</button>\r\n"
                "<div class=\"myinfo\">\r\n"
                    "fltMiniOLEDClock - 2025-2026 &#169; https://ha3flt.tkenedi.hu\r\n"
                "</div>\r\n"
            "</div>\r\n"
        "</div>\r\n"
    "</div>\r\n"
    "<script>\r\n"
    "(function() {\r\n"
        "const REFRESH_MS = 30000;\r\n"
        "const DATA_URL = '/main-data.json';\r\n"
        "const CLEAR_MINMAX_URL = '/clearminmax.json';\r\n"
        "const setText = function(id, value) {\r\n"
            "const elem = document.getElementById(id);\r\n"
            "if (elem) { elem.textContent = value; }\r\n"
        "};\r\n"
        "const refreshMainData = async function() {\r\n"
            "try {\r\n"
                "const response = await fetch(DATA_URL, { cache: 'no-store' });\r\n"
                "if (!response.ok) { return; }\r\n"
                "const data = await response.json();\r\n"
                "setText('location_str', data.location_str);\r\n"
                "setText('time', data.time);\r\n"
                "setText('date', data.date);\r\n"
                "setText('day', data.day);\r\n"
                "setText('temp1', data.temp1);\r\n"
                "setText('tmax1', data.tmax1);\r\n"
                "setText('tmin1', data.tmin1);\r\n"
                "setText('pressure', data.pressure);\r\n"
                "setText('humidity', data.humidity + '%%');\r\n"
                "setText('temp2', data.temp2);\r\n"
                "setText('tmax2', data.tmax2);\r\n"
                "setText('tmin2', data.tmin2);\r\n"
                "setText('version_value', data.version);\r\n"
            "} catch (err) {\r\n"
            "}\r\n"
        "};\r\n"
        "const clearMinMaxValues = async function() {\r\n"
            "try {\r\n"
                "const response = await fetch(CLEAR_MINMAX_URL, { cache: 'no-store' });\r\n"
                "if (!response.ok) { return; }\r\n"
                "await response.json();\r\n"
                "refreshMainData();\r\n"
            "} catch (err) {\r\n"
            "}\r\n"
        "};\r\n"
        "window.addEventListener('load', function() {\r\n"
            "const clearBtn = document.getElementById('clear-minmax-btn');\r\n"
            "if (clearBtn) {\r\n"
                "clearBtn.addEventListener('click', function() {\r\n"
                    "clearMinMaxValues();\r\n"
                "});\r\n"
            "}\r\n"
            "setTimeout(refreshMainData, 1000);\r\n"
            "setInterval(refreshMainData, REFRESH_MS);\r\n"
        "});\r\n"
    "})();\r\n"
    "</script>\r\n"
    "</body>\r\n"
    "</html>";

/****************************************************************************
** Purpose: Sends back the main dashboard HTML page to the client          **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendMainPage(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";

    // Fields:
    // id="time" %s
    // id="date" %s
    // id="day" %s

    // id="temp1" %+2.1f
    // id="tmin1" %+2.1f
    // id="tmax1" %+2.1f

    // id="pressure" %u
    // id="humidity" %u

    // id="temp2" %+2.1f
    // id="tmin2" %+2.1f
    // id="tmax2" %+2.1f

    // id="location_str" %s
    // id="version" %03u
    PageContent.printf_P(MAIN_HTML_BODY,
        getSettings()->getDeviceLocation(),

        getTimeManager()->getFormattedTimeHM().c_str(),
        getTimeManager()->getFormattedDate().c_str(),
        dayStr(weekday()),

        getSensors()->getLastTemperature1(),
        getSensors()->getMaxTemperature1(),
        getSensors()->getMinTemperature1(),

        (uint16_t)((getSensors()->getLastPressure() + 0.5) / 100.0),
        //(uint16_t)((getSensors()->getMaxPressure() + 0.5) / 100.0),
        //(uint16_t)((getSensors()->getMinPressure() + 0.5) / 100.0),
        (uint16_t)getSensors()->getLastHumidity(),
        //(uint16_t)getSensors()->getMaxHumidity(),
        //(uint16_t)getSensors()->getMinHumidity(),

        getSensors()->getLastTemperature2(),
        getSensors()->getMaxTemperature2(),
        getSensors()->getMinTemperature2(),

        FirmwareVersion
    );

    // RESPONSE HEADER
    DBGPRINTLN(F(""));
    DBGPRINT(F("SEND MAIN - Total length in resp. header: "));
    DBGPRINTLN(__main_html_header_html_len + PageContent.length());

    PageHeader.printf_P(
        HTTP_REQUEST_HEADER_TEXT, __main_html_header_html_len + PageContent.length());

    DBGPRINT(F("SEND MAIN - PageHeader.length(): "));
    DBGPRINTLN(PageHeader.length());

    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());

    // HTML HEADER
    uint8_t contentBuf[256];
    const uint8_t* dataPtr = __main_html_header_html;
    size_t lenToSend = __main_html_header_html_len;

    DBGPRINT(F("SEND MAIN HEADER - "));
    DBGPRINT(F("dataPtr: "));
    DBGPRINT((uint32_t)dataPtr, HEX);
    DBGPRINT(F(", lenToSend: "));
    DBGPRINT(lenToSend);
    DBGPRINTLN(".");

    while (lenToSend > 0) {
        DBGPRINT(F("SEND MAIN HEADER - lenToSend: "));
        DBGPRINTLN(lenToSend);

        size_t writeSize = incoming.availableForWrite();
        if (writeSize == 0) {
            DBGPRINTLN(F("SEND MAIN HEADER - writeSize is zero..."));
            //yield();
            delay(1);
            continue;
        }
        size_t contentBufSize = sizeof(contentBuf);
        size_t toWrite = std::min(lenToSend, contentBufSize);
        if (toWrite > writeSize) {
            toWrite = writeSize;
        }

        DBGPRINT(F("SEND MAIN HEADER - memcpy_P - "));
        DBGPRINT(F("dataPtr: "));
        DBGPRINT((uint32_t)dataPtr, HEX);
        DBGPRINT(F(", toWrite: "));
        DBGPRINT(toWrite);
        DBGPRINTLN(F("."));
        memcpy_P(contentBuf, dataPtr, toWrite);

        DBGPRINTLN(F("SEND MAIN HEADER - write in the loop"));
        size_t written = incoming.write(contentBuf, toWrite);
        lenToSend -= written;
        dataPtr += written;
    }

    // HTML BODY
    dataPtr = (const uint8_t*)PageContent.c_str();
    lenToSend = PageContent.length();

    DBGPRINT(F("SEND MAIN - "));
    DBGPRINT(F("dataPtr: "));
    DBGPRINT((uint32_t)dataPtr, HEX);
    DBGPRINT(F(", lenToSend: "));
    DBGPRINT(lenToSend);
    DBGPRINTLN(".");

    while (lenToSend > 0) {
        DBGPRINT(F("SEND MAIN - lenToSend: "));
        DBGPRINTLN(lenToSend);

        size_t writeSize = incoming.availableForWrite();
        if (writeSize == 0) {
            DBGPRINTLN(F("SEND MAIN - writeSize is zero..."));
            //yield();
            delay(1);
            continue;
        }
        size_t contentBufSize = sizeof(contentBuf);
        size_t toWrite = std::min(lenToSend, contentBufSize);
        if (toWrite > writeSize) {
            toWrite = writeSize;
        }

        DBGPRINT(F("SEND MAIN - memcpy_P - "));
        DBGPRINT(F("dataPtr: "));
        DBGPRINT((uint32_t)dataPtr, HEX);
        DBGPRINT(F(", toWrite: "));
        DBGPRINT(toWrite);
        DBGPRINTLN(F("."));
        memcpy_P(contentBuf, dataPtr, toWrite);

        DBGPRINTLN(F("SEND MAIN - write in the loop"));
        size_t written = incoming.write(contentBuf, toWrite);
        lenToSend -= written;
        dataPtr += written;
    }

    DBGPRINTLN(F("SEND MAIN - done."));
}

/****************************************************************************
** Purpose: Sends back JSON data for dynamic refresh on the main page      **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendMainData(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";

    PageContent.printf_P(
        PSTR("{\"date\":\"%s\",\"time\":\"%s\",\"day\":\"%s\",\"temp1\":\"%+2.1f\",\"tmin1\":\"%+2.1f\",\"tmax1\":\"%+2.1f\",\"temp2\":\"%+2.1f\",\"tmin2\":\"%+2.1f\",\"tmax2\":\"%+2.1f\",\"pressure\":\"%u\",\"humidity\":\"%u\",\"location_str\":\"%s\",\"version\":\"%03u\"}"),

        getTimeManager()->getFormattedDate().c_str(),
        getTimeManager()->getFormattedTimeHM().c_str(),
        dayStr(weekday()),

        getSensors()->getLastTemperature1(),
        getSensors()->getMinTemperature1(),
        getSensors()->getMaxTemperature1(),

        getSensors()->getLastTemperature2(),
        getSensors()->getMinTemperature2(),
        getSensors()->getMaxTemperature2(),

        (uint16_t)((getSensors()->getLastPressure() + 0.5) / 100.0),
        (uint16_t)getSensors()->getLastHumidity(),

        getSettings()->getDeviceLocation(),
        FirmwareVersion
    );

    PageHeader.printf_P(HTTP_REQUEST_HEADER_JSON, PageContent.length());

    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());
    incoming.write((const uint8_t*)PageContent.c_str(), PageContent.length());
}

/****************************************************************************
** Short but still visually pleasing info page HTTP body constant          **
****************************************************************************/
static const char MAIN_SHORT_HTML[] PROGMEM =
    "<!DOCTYPE html>\r\n"
    "<html lang=\"en-US\">\r\n"
    "<head>\r\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8;\">\r\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\r\n"
        "<meta http-equiv='refresh' content='60'/>\r\n"
        "<title>flt Mini OLED Clock</title>\r\n"
        "<style>\r\n"
            "body { background-color: #202020; font-family: Arial, Helvetica, Sans-Serif; Color: #f8f8f8; }\r\n"
        "</style>\r\n"
    "</head>\r\n"
    "<body>\r\n"
        "<h4 style=\"display:inline;\">fltMiniOLEDClock <span id=\"version_num\">v%03u</span></h4>\r\n"

        "<div style=\"margin-top:1rem;\"><span id=\"location_str\" style=\"display:inline;\">Location: </span>\r\n"
        "<h2 style=\"display:inline;\">%s</h2></div>\r\n"

        "<p><div id=\"local_time\">UTC time: <b>%s</b></div>\r\n"
        "<div id=\"day\">Day: <b>%s</b></div></p>\r\n"

        "<p><div id=\"temp_1\">Temp 1: <b>%.1f &deg;C</b></div>\r\n"
        "<div id=\"temp_1_max_min\">Temp 1 min, max: <b>%.1f &deg;C</b>, <b>%.1f &deg;C</b></div></p>\r\n"

        "<p><div id=\"pressure\">Pressure: <b>%.0f hPa</b></div>\r\n"
        "<div id=\"pressure_max_min\">Pressure min, max: <b>%.0f hPa</b>, <b>%.0f hPa</b></div></p>\r\n"

        "<p><div id=\"humidity\">Humidity: <b>%u%%</b></div>\r\n"
        "<div id=\"humidity_max_min\">Humidity min, max: <b>%u%%</b>, <b>%u%%</b></div></p>\r\n"

        "<p><div id=\"temp_2\">Temp 2: <b>%.1f &deg;C</b></div>\r\n"
        "<div id=\"temp_2_max_min\">Temp 2 min, max: <b>%.1f &deg;C</b>, <b>%.1f &deg;C</b></div></p>\r\n"

        "<p id=\"uptime\">Uptime: <b>%.2f day(s)</b> (%u seconds)</p>\r\n"
        "<p id=\"mem_stat\">Free heap: <b>%u bytes</b>, max. free: <b>%u bytes</b></p>\r\n"
    "</body>\r\n"
    "</html>";

/****************************************************************************
** Purpose: Sends back short measured data info HTML page to the client    **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendShortMainPage(BearSSL::WiFiClientSecure& incoming)
{
    /*Serial.println("=== ESP8266 Info ===");
    Serial.println("Chip ID.................: " + String(ESP.getChipId(), HEX));
    Serial.println("CPU freq................: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("Flash speed.............: " + String(ESP.getFlashChipSpeed()) + " Hz");
    Serial.println("Flash chip size (real)..: " + String(ESP.getFlashChipRealSize()) + " bytes");
    Serial.println("Flash chip size (IDE)...: " + String(ESP.getFlashChipSize()) + " bytes");
    Serial.println("--------------------");
    Serial.println("Free heap..........: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("Sketch size........: " + String(ESP.getSketchSize()) + " bytes");
    Serial.println("Free sketch space..: " + String(ESP.getFreeSketchSpace()) + " bytes");
    Serial.println("--------------------");*/

    PageHeader = PageContent = "";

    float sec = millis() / 1000.0;
    float hr = sec / 3600.0;

    PageContent.printf_P(MAIN_SHORT_HTML,
        FirmwareVersion,
        getSettings()->getDeviceLocation(),
        getTimeManager()->getFormattedDateTime().c_str(),
        dayStr(weekday()),
        getSensors()->getLastTemperature1(),
        getSensors()->getMinTemperature1(),
        getSensors()->getMaxTemperature1(),
        (getSensors()->getLastPressure() + 0.5) / 100.0,
        (getSensors()->getMinPressure() + 0.5) / 100.0,
        (getSensors()->getMaxPressure() + 0.5) / 100.0,
        getSensors()->getLastHumidity(),
        getSensors()->getMinHumidity(),
        getSensors()->getMaxHumidity(),
        getSensors()->getLastTemperature2(),
        getSensors()->getMinTemperature2(),
        getSensors()->getMaxTemperature2(),
        hr / 24.0, (uint32_t)sec,
        ESP.getFreeHeap(), ESP.getMaxFreeBlockSize()
    );

    PageHeader.printf_P(HTTP_REQUEST_HEADER_TEXT, PageContent.length());

    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());
    incoming.write((const uint8_t*)PageContent.c_str(), PageContent.length());
}

/****************************************************************************
** Purpose: Clears the minimum and maximum values of sensor data           **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendClearMinMax(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";

    // Reset sensor min/max values
    DBGPRINTLN(F("Resetting sensor min/max values..."));
    getSensors()->resetMinMaxTemperature1();
    getSensors()->resetMinMaxTemperature2();

    PageContent.printf_P(PSTR("{\"Result\":\"OK\"}"));
    PageHeader.printf_P(HTTP_REQUEST_HEADER_JSON, PageContent.length());

    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());
    incoming.write((const uint8_t*)PageContent.c_str(), PageContent.length());
}

/****************************************************************************
** Purpose: Sends back the big background picture to the client            **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendBackgroundPicture(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";
    PageHeader.printf_P(HTTP_REQUEST_HEADER_JPEG, __bkgnd_planets_jpg_len);
    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());

    uint8_t contentBuf[256];
    const uint8_t* dataPtr = __bkgnd_planets_jpg;
    size_t lenToSend = __bkgnd_planets_jpg_len;
    while (lenToSend > 0) {
        size_t writeSize = incoming.availableForWrite();
        if (writeSize == 0) {
            //yield();
            delay(1);
            continue;
        }
        size_t contentBufSize = sizeof(contentBuf);
        size_t toWrite = std::min(lenToSend, contentBufSize);
        if (toWrite > writeSize) {
            toWrite = writeSize;
        }
        memcpy_P(contentBuf, dataPtr, toWrite);

        size_t written = incoming.write(contentBuf, toWrite);
        lenToSend -= written;
        dataPtr += written;
    }
}

/****************************************************************************
** Purpose: Sends back the default font file to the client                 **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendMatrixFont(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";
    PageHeader.printf_P(HTTP_REQUEST_HEADER_FONT, __calottery_machine_otf_woff2_len);
    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());

    uint8_t contentBuf[256];
    const uint8_t* dataPtr = __calottery_machine_otf_woff2;
    size_t lenToSend = __calottery_machine_otf_woff2_len;
    while (lenToSend > 0) {
        size_t writeSize = incoming.availableForWrite();
        if (writeSize == 0) {
            //yield();
            delay(1);
            continue;
        }
        size_t contentBufSize = sizeof(contentBuf);
        size_t toWrite = std::min(lenToSend, contentBufSize);
        if (toWrite > writeSize) {
            toWrite = writeSize;
        }
        memcpy_P(contentBuf, dataPtr, toWrite);

        size_t written = incoming.write(contentBuf, toWrite);
        lenToSend -= written;
        dataPtr += written;
    }
}

/****************************************************************************
** Purpose: Sends back the favicon to the client                           **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::sendFavIcon(BearSSL::WiFiClientSecure& incoming)
{
    PageHeader = PageContent = "";
    PageHeader.printf_P(HTTP_REQUEST_HEADER_ICON, __favicon_ico_len);
    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());

    uint8_t contentBuf[256];
    const uint8_t* dataPtr = __favicon_ico;
    size_t lenToSend = __favicon_ico_len;
    while (lenToSend > 0) {
        size_t writeSize = incoming.availableForWrite();
        if (writeSize == 0) {
            //yield();
            delay(1);
            continue;
        }
        size_t contentBufSize = sizeof(contentBuf);
        size_t toWrite = std::min(lenToSend, contentBufSize);
        if (toWrite > writeSize) {
            toWrite = writeSize;
        }
        memcpy_P(contentBuf, dataPtr, toWrite);

        size_t written = incoming.write(contentBuf, toWrite);
        lenToSend -= written;
        dataPtr += written;
    }
}

/****************************************************************************
** Purpose: Sends back an error with error code and string to the client   **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
** Returns: None                                                           **
****************************************************************************/
static const char HTML_ERROR_HTML[] PROGMEM =
    "<html><body><h1>fltMiniOLEDClock - Error %d - %s</h1></body></html>";

void CWebServer::sendErrorPage(BearSSL::WiFiClientSecure& incoming, int code, const char* msg)
{
    PageHeader = PageContent = "";
    PageContent.printf_P(HTML_ERROR_HTML, code, msg);
    PageHeader.printf_P(HTTP_REQUEST_HEADER_TEXT_ERR, code, msg, PageContent.length());

    incoming.write((const uint8_t*)PageHeader.c_str(), PageHeader.length());
    incoming.write((const uint8_t*)PageContent.c_str(), PageContent.length());
}

/****************************************************************************
** Purpose: Handle HTTPS request, dispatch it to the resource handlers     **
** Param's: BearSSL::WiFiClientSecure& - request data, response place      **
**          const String& - the already parsed URL                         **
** Returns: None                                                           **
****************************************************************************/
void CWebServer::handleRequest(BearSSL::WiFiClientSecure& incoming, const String& url)
{
    DBGPRINT(F("*** Client requested URL: \""));
    DBGPRINT(url);
    DBGPRINTLN(F("\" ***"));

    if (url == "/" || url == "/index.html" || url == "/index.htm") {
        sendMainPage(incoming);
    } else if (url == "/main-data.json" || url.startsWith("/main-data.json?")) {
        sendMainData(incoming);
    } else if (url == "/clearminmax.json" || url.startsWith("/clearminmax.json?")) {
        sendClearMinMax(incoming);
    } else if (url == "/bkgnd-planets-1.jpg") {
        sendBackgroundPicture(incoming);
    } else if (url == "/CaLotteryMachineFlt-Regular.otf.woff2") {
        sendMatrixFont(incoming);
    } else if (url == "/favicon.ico") {
        sendFavIcon(incoming);
    } else /*if (url == "/short.html")*/ {
        sendShortMainPage(incoming);
    } /*else {
        sendErrorPage(incoming, 404, PSTR("Not Found"));
    }*/

    DBGPRINT(F("*** Processing finished for URL: "));
    DBGPRINT(url);
    DBGPRINTLN(F("\" ***"));
}

/****************************************************************************
** Purpose: Parse the requested URL from a HTTPS request line              **
** Param's: const String& - the line to be parsed                          **
** Returns: None                                                           **
****************************************************************************/
String CWebServer::parseUrlFromRequestLine(const String& line)
{
    // Example GET line: GET /status HTTP/1.1
    if (!line.startsWith("GET")) {
        return "";
    }
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    if (firstSpace < 0 || secondSpace < 0) {
        return "";
    }
    return line.substring(firstSpace + 1, secondSpace);
}

/****************************************************************************
** Purpose: Read a full HTTPS request line with a short timeout            **
** Param's: BearSSL::WiFiClientSecure& - request data, no response here    **
**          const String& - the request line output                        **
** Returns: None                                                           **
****************************************************************************/
bool CWebServer::readHttpRequest(BearSSL::WiFiClientSecure& incoming, String& reqLine)
{
    uint32_t startTime = millis();
    while (millis() - startTime < 1000) {
        if (incoming.available()) {
            reqLine = incoming.readStringUntil('\n');
            reqLine.trim();
            DBGPRINT(F("Got a HTTP request, size: "));
            DBGPRINT(reqLine.length());
            DBGPRINTLN(F("..."));
            return true;
        }
        //yield();
        delay(1);
    }
    return false;
}

/****************************************************************************
** Purpose: The WebServer's callback function, reads HTTPS requests        **
** Param's: None                                                           **
** Returns: This function reads and processes HTTPS requests by calling    **
**          the request line parser and the request handling functions.    **
****************************************************************************/
static const char MSG_INCOMING_CONNECTION[] PROGMEM =
    "Incoming connection %d...\r\n";

void CWebServer::loop(uint32_t now)
{
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    static int16_t cnt = 0;
    BearSSL::WiFiClientSecure incoming = server->accept();
    // The accept() does not block...
    if (!incoming.connected()) {
        return;
    }
    // Wait for the request
    while (!incoming.available()) {
        //TODO: It this value optimal?
        if (millis() - now > WEBSERVER_REQUEST_READ_TIMEOUT_MSEC) {
            return;
        }
        //yield();
        delay(1);
    }
    DBGPRINTF_P(MSG_INCOMING_CONNECTION, ++cnt);
    // Read HTTPS request
    String requestLine;
    if (!readHttpRequest(incoming, requestLine)) {
        sendErrorPage(incoming, 408, PSTR("Request Timeout"));
        DBGPRINTLN(F("Request Timeout"));
        return;
    }
    // Parse URL
    String url = parseUrlFromRequestLine(requestLine);
    if (url.length() == 0) {
        sendErrorPage(incoming, 400, PSTR("Bad Request"));
        DBGPRINTLN(F("Bad Request"));
        return;
    }
    // Handle the request
    handleRequest(incoming, url);
    // Close cleanly
    incoming.flush();
    incoming.stop();
    DBGPRINTLN(F("Connection closed"));
}
