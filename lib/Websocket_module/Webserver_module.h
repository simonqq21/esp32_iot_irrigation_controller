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

- ESP32 send relay states to browser
{
    cmd: "load",
    type: "relay_states",
    payload: {
        relay_states[NUMBER_OF_RELAYS]: bool[NUMBER_OF_RELAYS],
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
    type: "main_config",
    payload: {
        name: string,
        ntpEnabledSetting: bool,
        gmtOffsetSetting: int, 
    },
}

- ESP32 send relays configuration to browser
{
    cmd: "load",
    type: "relay_configs",
    payload: {
        relay_config: {
            index: int,
            ledSetting: int,
            operationModeSetting: int,
            relayManualSetting: bool,
            timeSlots[]: timeSlot[] {
                index: int,
                enabled: bool,
                onStartTime: string (ISO datetime),
                onEndTime: string (ISO datetime),
            },
            countdownDurationSetting: unsigned long,
        }
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
    type: "relay_states",
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
    type: "main_config",
    payload: {
    },
}

- Browser request relays configuration from ESP32
{
    cmd: "request",
    type: "relay_configs",
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
    type: "relay_states", 
    payload: {
        index: int,
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
    type: "main_config",
    payload: {
        name: string,
        ntpEnabledSetting: bool,
        gmtOffsetSetting: int, 
    },
}

- Browser send and set relay configuration to ESP32
{
    cmd: "save",
    type: "relay_configs",
    payload: {
        index: int,
        operationModeSetting: int,
        ledSetting: int,
        relayManualSetting: bool,
        timeSlots[]: timeSlot[] {
            index: int,
            enabled: bool,
            onStartTime: string (ISO time),
            onEndTime: string (ISO time),
        },
        countdownDurationSetting: long,
    },
}

- browser switch relay state (without saving)
switches relay state in manual mode and starts/stops the countdown timer
{
    cmd: "switch",
    type: "relay_states",
    payload: {
        index: int,
        relay_state: bool,
    },
}

*/




/*
    ESPWiFi behavior
    4,0
    wifi.status = 3 when successfully connected to AP 
    wifi.status = 6 when trying to connect
    wifi.status = 1 when expected wifi doesn't exist either because of wrong credentials
        or network just doens't exist.
    wifi.status = 255 when in STA mode

    when wifi credentials are wrong, 
    6, 1, 255

    when wifi credentials are correct, 
    6, 3

    when wifi hotspot doesn't exist upon startup,
    6, 1, 255

    when wifi hotspot stopped existing while working,
    3, 255

    Expected wifi connection behavior:
        When ESP32 starts and attempts to connect with a wifi hotspot with correct credentials,
    then it will connect successfully. 
        When ESP32 starts and attempts to connect with a wifi hotspot with wrong credentials,
    then it will fail to connect and start its own hotspot without resetting the saved wifi credentials.
        When ESP32 starts and attempts to connect with a wifi hotspot that doesn't exist,
    then it will start its own hotspot without resetting the saved wifi credentials.
        When ESP32 starts and connects successfully to a wifi hotspot that suddenly disappears during
    operation, then it must attempt to reconnect until the wifi hotspot reappears. 
        The WiFi credentials in itys EEPROM must not be reset to default values except via websocket or physical 
    button long press.

    */

// some definitions for JSON msg constants
const String CMD_KEY = "cmd";
const String TYPE_KEY = "type";
const String PAYLOAD_KEY = "payload";

const String LOAD_CMD = "load";
const String SAVE_CMD = "save";
const String REQUEST_CMD = "request";
const String SWITCH_CMD = "switch";

const String WIFIS_TYPE= "wifis";
const String CONNECTION_TYPE = "connection";
const String RELAY_STATES_TYPE = "relay_states"; 
const String DATETIME_TYPE = "datetime"; 
const String MAIN_CONFIG_TYPE = "main_config";
const String RELAY_CONFIGS_TYPE = "relay_configs";


class WebserverModule {
    public:
        WebserverModule();
        static void begin(EEPROMConfig* eC, RTCNTP* rtcntp, Relay relays[3]);
        
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
        static void sendCurrentRelayStates(bool curRelayStates[]);
        static void sendDateTime(JsonDocument inputPayloadJSON=JsonDocument());
        static void sendMainConfig(JsonDocument inputPayloadJSON=JsonDocument());
        static void sendRelayConfig(int rIndex, JsonDocument inputPayloadJSON=JsonDocument());
        // method to handle requests from the client browser 
        static void handleRequest(String type, JsonDocument payloadJSON); 
        // set callbacks for sending methods
        static void setSendConnectionCallback(void (*callback)() = NULL);
        static void setSendRelayStateCallback(void (*callback)() = NULL);
        static void setSendDateTimeCallback(void (*callback)() = NULL);
        static void setSendConfigCallback(void (*callback)() = NULL);

        // methods to receive and set new state from client browser 
        static void receiveConnection(JsonDocument inputPayloadJSON);
        // static void receiveRelayState(JsonDocument inputPayloadJSON);
        static void receiveDateTime(JsonDocument inputPayloadJSON);
        static void receiveMainConfig(JsonDocument inputPayloadJSON);
        static void receiveRelayConfigs(JsonDocument inputPayloadJSON);
        static void switchRelayState(JsonDocument inputPayloadJSON);
        // method to handle receiving different kinds of data from the client browser 
        static void receiveData(String cmd, String type, JsonDocument payloadJSON);
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
        static Relay* _relays[NUMBER_OF_RELAYS]; 

        static void (*_sendConnectionFunc)();
        static void (*_sendRelayStateFunc)();
        static void (*_sendDateTimeFunc)();
        static void (*_sendConfigFunc)();

        static void (*_receiveConnectionFunc)();
        static void (*_receiveRelayStateFunc)();
        static void (*_receiveDateTimeFunc)();
        static void (*_receiveConfigFunc)();

        static void (*_switchRelayStateFunc)();
};

#endif