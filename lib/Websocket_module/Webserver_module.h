#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "ArduinoJson.h"

#include "RTCNTPlib.h"
#include "EEPROMConfig.h"
#include "Relay.h"

/*
Other libraries and modules to include:
IO module
RTC module
EEPROM configuration module 

Operation:
esp32 will attempt to connect to WiFi with the SSID 
and password saved on the EEPROM. If the credentials 
are incorrect, then it will start a soft AP with a 
static IP address that the client device can connect 
to. The client device loads up the wifi manager
webpage, which lists down the available wifi networks
and refreshes it every 5 seconds. When the user clicks 
on a wifi ssid div, it will then ask you for the 
password, IP address index, and port. Upon submission
of the connection details, the ESP32 will attempt to 
reconnect to the wifi with the new credentials. If it
succeeds, it will continue operating normally. But if the 
wifi fails to connect for any reason, either out of range,
router turned off, or wrong credentials, it will restart
its soft AP so it can continue to be used standalone or 
be reconfigured to connect somewhere else. 

messages:
Standard contents of every JSON payload:
{
    cmd: string,
    type: string,
    payload: any,
}



from ESP32 to browser
- ESP32 send list of wifi hotspots to the browser
{
    cmd: "load",
    type: "wifis",
    payload: {
        wifis: [] {
            ssid: string, 
            rssi: int,
            security: string,
        }
    },
}
- ESP32 send connection details to browser
{
    cmd: "load",
    type: "connection",
    payload: {
        ssid: string,
        ipIndex: int,
        port: int,
    },
}

- ESP32 send relay state to browser
{
    cmd: "load",
    type: "relay_state",
    payload: {
        relay_state: bool,
    },
}

- ESP32 send system date and time to browser
{
    cmd: "load",
    type: "datetime", 
    payload: {
        datetime: string (ISO datetime format),
    },
}

- ESP32 send main configuration to browser
{
    cmd: "load",
    type: "config",
    payload: {
        name: string,
        ntpEnabledSetting: bool,
        gmtOffsetSetting: int, 
        timerEnabledSetting: bool,
        ledSetting: int,
        relayManualSetting: bool,
        timeSlots[]: timeSlot[] {
            index: int,
            enabled: bool,
            onStartTime: string (ISO datetime),
            onEndTime: string (ISO datetime),
        },
    },
}



from browser to ESP32
- Browser request list of wifi hotspots from ESP32 
{
    cmd: "request",
    type: "wifis",
    payload: {
    },
}

- Browser request connection details from ESP32
{
    cmd: "request",
    type: "connection",
    payload: {
    },
}

- Browser request relay state from ESP32
{
    cmd: "request",
    type: "relay_state",
    payload: {
    },
}

- Browser request system date and time from ESP32
{
    cmd: "request",
    type: "datetime",
    payload: {
    },
}

- Browser request main configuration from ESP32
{
    cmd: "request",
    type: "config",
    payload: {
    },
}

- Browser send connection details to ESP32
{
    cmd: "save",
    type: "connection",
    payload: {
        ssid: string,
        pass: string,
        ipIndex: int,
        port: int,
    },
}

- Browser send and set relay state to ESP32
{
    cmd: "save",
    type: "relay_state", 
    payload: {
        relay_state: bool,
    },
}

- Browser send and set system date and time to ESP32
{
    cmd: "save",
    type: "datetime", 
    payload: {
        datetime: string (ISO datetime format),
    },
}

- Browser send and set main configuration to ESP32
{
    cmd: "save",
    type: "config",
    payload: {
        name: string,
        ntpEnabledSetting: bool,
        gmtOffsetSetting: int, 
        timerEnabledSetting: bool,
        ledSetting: int,
        timeSlots[]: timeSlot[] {
            index: int,
            enabled: bool,
            onStartTime: string (ISO time),
            onEndTime: string (ISO time),
        },
    },
}
*/

// some definitions for JSON msg constants
const String CMD_KEY = "cmd";
const String TYPE_KEY = "type";
const String PAYLOAD_KEY = "payload";

const String LOAD_CMD = "load";
const String SAVE_CMD = "save";
const String REQUEST_CMD = "request";
const String CONNECTION_TYPE = "connection";
const String RELAY_STATE_TYPE = "relay_state"; 
const String DATETIME_TYPE = "datetime"; 
const String CONFIG_TYPE = "config";
const String WIFIS_TYPE= "wifis";

class WebserverModule {
    public:
        WebserverModule();
        static void begin(EEPROMConfig* eC, RTCNTP* rtcntp, Relay* relay);
        
        // wifi connection methods
        static void connect();
        static void scanWiFi();
        static void sendWiFiScanResults();
        static void cleanupClients();
        static void checkWiFiStatusLoop();

        // websocket methods
        static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
            void *arg, uint8_t *data, size_t len);
        static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

        // methods to send ESP32 state to client browser
        static void sendConnection(JsonDocument payloadJSON=JsonDocument());
        static void sendCurrentRelayState(bool curRelayState);
        static void sendDateTime(JsonDocument inputPayloadJSON=JsonDocument());
        static void sendConfig(JsonDocument inputPayloadJSON=JsonDocument());
        // method to handle requests from the client browser 
        static void handleRequest(String type, JsonDocument payloadJSON); 
        // set callbacks for sending methods
        static void setSendConnectionCallback(void (*callback)() = NULL);
        static void setSendRelayStateCallback(void (*callback)() = NULL);
        static void setSendDateTimeCallback(void (*callback)() = NULL);
        static void setSendConfigCallback(void (*callback)() = NULL);

        // methods to receive and set new state from client browser 
        static void receiveConnection(JsonDocument inputPayloadJSON);
        static void receiveRelayState(JsonDocument inputPayloadJSON);
        static void receiveDateTime(JsonDocument inputPayloadJSON);
        static void receiveConfig(JsonDocument inputPayloadJSON);
        // method to handle receiving different kinds of data from the client browser 
        static void receiveData(String type, JsonDocument payloadJSON);
        // set callbacks for receiving methods
        static void setReceiveConnectionCallback(void (*callback)() = NULL);
        static void setReceiveRelayStateCallback(void (*callback)() = NULL);
        static void setReceiveDateTimeCallback(void (*callback)() = NULL);
        static void setReceiveConfigCallback(void (*callback)() = NULL);

    private:
        static AsyncWebServer* _server;
        static AsyncWebSocket _ws;
        static JsonDocument _jsonDoc;
        static char _strData[1250];
        static EEPROMConfig* _eC;
        static RTCNTP* _rtcntp;
        static IPAddress _apIP;
        static Relay* _relay; 

        static void (*_sendConnectionFunc)();
        static void (*_sendRelayStateFunc)();
        static void (*_sendDateTimeFunc)();
        static void (*_sendConfigFunc)();

        static void (*_receiveConnectionFunc)();
        static void (*_receiveRelayStateFunc)();
        static void (*_receiveDateTimeFunc)();
        static void (*_receiveConfigFunc)();
};

#endif