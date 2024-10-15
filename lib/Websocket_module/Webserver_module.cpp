#include "Webserver_module.h"

AsyncWebServer* WebserverModule::_server;
AsyncWebSocket WebserverModule::_ws = AsyncWebSocket("/ws");
JsonDocument WebserverModule::_jsonDoc;
char WebserverModule::_strData[1250];
EEPROMConfig* WebserverModule::_eC;
RTCNTP* WebserverModule::_rtcntp;
IPAddress  WebserverModule::_apIP;
// Relay* WebserverModule::_relays;
Relay* WebserverModule::_relays[NUMBER_OF_RELAYS];

void (*WebserverModule::_sendConnectionFunc)();
void (*WebserverModule::_sendRelayStateFunc)();
void (*WebserverModule::_sendDateTimeFunc)();
void (*WebserverModule::_sendConfigFunc)();
void (*WebserverModule::_receiveConnectionFunc)();
void (*WebserverModule::_receiveRelayStateFunc)();
void (*WebserverModule::_receiveDateTimeFunc)();
void (*WebserverModule::_receiveConfigFunc)();
void (*WebserverModule::_switchRelayStateFunc)();

unsigned int previousPrintTime;

WebserverModule::WebserverModule() {

}

void WebserverModule::begin(EEPROMConfig* eC, RTCNTP* rtcntp, Relay relays[3]) {
    _eC = eC;
    _rtcntp = rtcntp;
    for (int i=0;i<NUMBER_OF_RELAYS;i++) {
        _relays[i] = &relays[i];
    }
    // start websockets and webserver   
    _ws.onEvent(onEvent);
    _server = new AsyncWebServer(_eC->getPort());
    _server = new AsyncWebServer(7777);
    _server->addHandler(&_ws);
    _server->begin();
    Serial.println("initialized ws");

    // start serving webpages
    AsyncWebHandler testHandler = _server->on("/test", HTTP_GET, [](AsyncWebServerRequest* request) {
        // request->send(200, "hhelloworld");
        request->send(200, "text/plain", "Hhhelloworld");
    });
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", String(), false);
    });
    _server->on("/index.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", String(), false);
    });
    _server->on("/wifi.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/wifi.html", String(), false);
    });
    _server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css", false);
    });
    _server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.css", "text/css", false);
    });
    _server->on("/wifi.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/wifi.css", "text/css", false);
    });
    _server->on("/wsMod.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/wsMod.js", "text/javascript", false);
    });
    _server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.js", "text/javascript", false);
    });
    _server->on("/wifi.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/wifi.js", "text/javascript", false);
    });
}

/*
*/
void WebserverModule::connect() {
    Serial.println("Connecting to wifi");
    // attempt to connect to wifi 
    IPAddress localIP;
    IPAddress gateway;
    IPAddress subnet(255,255,255,0);
    IPAddress dns(8,8,8,8);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(_eC->getSSID().c_str(), _eC->getPassword().c_str());
    delay(3000);
    // Serial.printf("Connected to %s\n", WiFi.SSID());
    Serial.printf("wifi status = %d\n", WiFi.status());

    // attempt to reconnect 5 times maximum, 2 seconds each.
    for (int i=0;i<5;i++) {
        if (WiFi.status() == WL_CONNECTED) {
            localIP = WiFi.localIP();
            Serial.println(WiFi.localIP());
            gateway = WiFi.gatewayIP();
            Serial.print("gateway IP = ");
            Serial.println(gateway);
            localIP[3] = _eC->getIPAddressIndex();
            WiFi.config(localIP, gateway, subnet, dns);
            Serial.println(WiFi.localIP());
            break;
        }
        delay(2000);
    }
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP("ESP32_wifi_manager");
        IPAddress apIP = IPAddress(192, 168, 4, 1);
        IPAddress apSubnet = IPAddress(255,255,255,0);
        WiFi.softAPConfig(apIP, apIP, apSubnet);
        _apIP = WiFi.softAPIP();
        Serial.println("started softAP");
    }
}

/*
*/
void WebserverModule::scanWiFi() {
    Serial.println("Scanning wifi...");
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks(false);
    Serial.printf("%d networks found.\n", n);
}

void WebserverModule::sendWiFiScanResults() {
    String ssid, security;
    int rssi;
    int n = WiFi.scanComplete();
    if (n > -2) {
        Serial.println(n);
    }
    if (n > -1) {
         _jsonDoc.clear();
        _jsonDoc[CMD_KEY] = LOAD_CMD;
        _jsonDoc[TYPE_KEY] = WIFIS_TYPE;
        JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
        JsonArray wifisJSON = payloadJSON[WIFIS_TYPE].to<JsonArray>();
        Serial.println("scan done");
        if (n < 1) {
            Serial.println("no wifis found.");
        }
        else {
            Serial.printf("%d networks found:\n", n);
            for (int i=0;i<n;i++) {
                ssid = WiFi.SSID(i).c_str();
                rssi = WiFi.RSSI(i);
                switch (WiFi.encryptionType(i)) {
                    case WIFI_AUTH_OPEN:
                        security = "open";
                        break;
                    case WIFI_AUTH_WEP:
                        security = "wep";
                        break;
                    case WIFI_AUTH_WPA_PSK:
                        security = "wpa";
                        break;
                    case WIFI_AUTH_WPA_WPA2_PSK:
                        security = "wpa_wpa2";
                        break;
                    case WIFI_AUTH_WPA2_PSK:
                        security = "wpa2";
                        break;
                    case WIFI_AUTH_WPA2_ENTERPRISE:
                        security = "wpa2_ent";
                        break;
                    case WIFI_AUTH_WPA2_WPA3_PSK:
                        security = "wpa2_wpa3";
                        break;
                    case WIFI_AUTH_WPA3_PSK:
                        security = "wpa3";
                        break;
                    case WIFI_AUTH_WAPI_PSK:
                        security = "wapi";
                        break;
                    default:
                        security = "unknown";
                }
                Serial.printf("%d. SSID=%-32.32s, RSSI=%2d, security=%s\n", i, ssid, rssi, security);
                JsonObject wifiJSON;
                wifiJSON = wifisJSON.add<JsonObject>();
                wifiJSON["ssid"] = ssid;
                wifiJSON["rssi"] = rssi;
                wifiJSON["security"] = security;
            }
        }
        // WiFi.scanDelete();
        serializeJson(_jsonDoc, _strData);
        Serial.printf("serialized JSON = %s\n", _strData);
        _ws.textAll(_strData);
    }
}

void WebserverModule::cleanupClients() {
    _ws.cleanupClients();
}

/*
this method is called in the void loop 
*/
void WebserverModule::checkWiFiStatusLoop() {
    if (millis() - previousPrintTime > 1000) {
        Serial.printf("wifi status = %d\n", WiFi.status());
        previousPrintTime = millis();
    }

    yield();
}

void WebserverModule::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;

        DeserializationError error = deserializeJson(_jsonDoc, (char*)data); 
        String cmd = _jsonDoc[CMD_KEY];
        String type = _jsonDoc[TYPE_KEY];
        JsonDocument payloadJSON = _jsonDoc[PAYLOAD_KEY];

        Serial.printf("cmd=%s\n", cmd);
        Serial.printf("type=%s\n", type);
        // JsonObject payload = pj.as<JsonObject>();
        if (cmd == REQUEST_CMD) {
            handleRequest(type, payloadJSON);
        }
        else if (cmd == SAVE_CMD) {
            receiveData(cmd, type, payloadJSON);
        }
        else if (cmd == SWITCH_CMD) {
            receiveData(cmd, type, payloadJSON);
        }
    }
}

void WebserverModule::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
    void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

// methods to send ESP32 state to client browser
void WebserverModule::sendConnection(JsonDocument inputPayloadJSON) {
    _jsonDoc.clear();
    _jsonDoc[CMD_KEY] = LOAD_CMD;
    _jsonDoc[TYPE_KEY] = CONNECTION_TYPE;
    JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
    payloadJSON["ssid"] = _eC->getSSID();
    payloadJSON["ipIndex"] = _eC->getIPAddressIndex();
    payloadJSON["port"] = _eC->getPort();
    serializeJson(_jsonDoc, _strData);
    Serial.printf("serialized JSON = %s\n", _strData);
    _ws.textAll(_strData);
}

void WebserverModule::sendCurrentRelayStates(bool curRelayStates[]) {
    _jsonDoc.clear();
    _jsonDoc[CMD_KEY] = LOAD_CMD;
    _jsonDoc[TYPE_KEY] = RELAY_STATES_TYPE;
    JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
    JsonArray relayStatesArr = payloadJSON["relay_states"].to<JsonArray>();
    for (int i=0;i<NUMBER_OF_RELAYS;i++) {
        relayStatesArr.add(curRelayStates[i]);
    }
    serializeJson(_jsonDoc, _strData);
    Serial.printf("serialized JSON = %s\n", _strData);
    _ws.textAll(_strData);
}

void WebserverModule::sendDateTime(JsonDocument inputPayloadJSON) {
    _jsonDoc.clear();
    _jsonDoc[CMD_KEY] = LOAD_CMD;
    _jsonDoc[TYPE_KEY] = DATETIME_TYPE;
    _rtcntp->getRTCTime();
    JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
    payloadJSON["datetime"] = _rtcntp->getISODateTime();
    serializeJson(_jsonDoc, _strData);
    Serial.printf("serialized JSON = %s\n", _strData);
    _ws.textAll(_strData);
}

void WebserverModule::sendMainConfig(JsonDocument inputPayloadJSON) {
    _jsonDoc.clear();
    _jsonDoc[CMD_KEY] = LOAD_CMD;
    _jsonDoc[TYPE_KEY] = MAIN_CONFIG_TYPE;
    JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
        payloadJSON["name"] = _eC->getName();
        payloadJSON["ntpEnabledSetting"] = _eC->getNTPEnabled();
        payloadJSON["gmtOffsetSetting"] = _eC->getGMTOffset();
    serializeJson(_jsonDoc, _strData);
    Serial.printf("serialized JSON = %s\n", _strData);
    _ws.textAll(_strData);
}

void WebserverModule::sendRelayConfig(int rIndex, JsonDocument inputPayloadJSON) {
    _jsonDoc.clear();
    _jsonDoc[CMD_KEY] = LOAD_CMD;
    _jsonDoc[TYPE_KEY] = RELAY_CONFIGS_TYPE;
    JsonObject payloadJSON = _jsonDoc[PAYLOAD_KEY].to<JsonObject>();
        payloadJSON["index"] = rIndex;
        payloadJSON["ledSetting"] = _eC->getLEDSetting(rIndex);
        payloadJSON["operationModeSetting"] = _eC->getOperationMode(rIndex);
        payloadJSON["relayManualSetting"] = _eC->getRelayManualSetting(rIndex);
        JsonArray timeSlotsJSON = payloadJSON["timeSlots"].to<JsonArray>();
        JsonObject timeSlotJSON;
        for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
            TimeSlot* curTS = _eC->getTimeSlot(rIndex, i);
            timeSlotJSON = timeSlotsJSON.add<JsonObject>();
            timeSlotJSON["index"] = curTS->getIndex();
            timeSlotJSON["enabled"] = curTS->getEnabled();
            timeSlotJSON["onStartTime"] = curTS->getOnStartTimeISOString();
            timeSlotJSON["onEndTime"] = curTS->getOnEndTimeISOString();
            // timeSlotJSON["durationInSeconds"] = curTS->getDuration();
        }
        payloadJSON["countdownDurationSetting"] = _eC->getCountdownDuration(rIndex);
    serializeJson(_jsonDoc, _strData);
    Serial.printf("serialized JSON = %s\n", _strData);
    _ws.textAll(_strData);
}

// method to handle requests from the client browser 
void WebserverModule::handleRequest(String type, JsonDocument payloadJSON) {
    if (type == CONNECTION_TYPE) {
        sendConnection(payloadJSON);
        if (_sendConnectionFunc != NULL) {
            _sendConnectionFunc();
        }
    }
    else if (type == RELAY_STATES_TYPE) {
        bool relaystates[NUMBER_OF_RELAYS];
        // _relay->readState()
        for (int i=0;i<NUMBER_OF_RELAYS;i++) {
            relaystates[i] = _relays[i]->readState();
        }
        sendCurrentRelayStates(relaystates);
        if (_sendRelayStateFunc != NULL) {
            _sendRelayStateFunc();
        }
    }
    else if (type == DATETIME_TYPE) {
        sendDateTime(payloadJSON);
        if (_sendDateTimeFunc != NULL) {
            _sendDateTimeFunc();
        }  
    }
    else if (type == MAIN_CONFIG_TYPE) {
        sendMainConfig(payloadJSON);
        if (_sendConfigFunc != NULL) {
            _sendConfigFunc();           
        }
    }
    else if (type == RELAY_CONFIGS_TYPE) {
        for (int i=0;i<NUMBER_OF_RELAYS;i++) {
            sendRelayConfig(i, payloadJSON);
        }
    }
    else if (type == WIFIS_TYPE) {
        // scanWiFi(payloadJSON);
        sendWiFiScanResults();
    }
    else {
        _ws.textAll("Invalid request");
    }
}

void WebserverModule::setSendConnectionCallback(void (*callback)()) {
    _sendConnectionFunc = callback;
}

void WebserverModule::setSendRelayStateCallback(void (*callback)()) {
    _sendRelayStateFunc = callback;
}

void WebserverModule::setSendDateTimeCallback(void (*callback)()) {
    _sendDateTimeFunc = callback;
}

void WebserverModule::setSendConfigCallback(void (*callback)()) {
    _sendConfigFunc = callback;
}

// methods to receive and set new state from client browser 
// also attempt to reconnect to wifi after saving the credentials.
void WebserverModule::receiveConnection(JsonDocument inputPayloadJSON) {
    _eC->setSSID(inputPayloadJSON["ssid"]);
    _eC->setPassword(inputPayloadJSON["pass"]);
    _eC->setIPAddressIndex(inputPayloadJSON["ipIndex"]);
    _eC->setPort(inputPayloadJSON["port"]);
    _eC->saveConnectionConfig();
    Serial.println("saved connection");
    ESP.restart();
}

// void WebserverModule::receiveRelayState(JsonDocument inputPayloadJSON) {
//     _eC->setRelayManualSetting(inputPayloadJSON["relay_state"]);
//     _eC->saveMainConfig();
//     Serial.println("saved relay manual state");
// }

void WebserverModule::receiveDateTime(JsonDocument inputPayloadJSON) {
    String dtisostr = inputPayloadJSON["datetime"];
    _rtcntp->setISODateTime(dtisostr);
    Serial.println("saved datetime");
}

void WebserverModule::receiveMainConfig(JsonDocument inputPayloadJSON) {
    _eC->setName(inputPayloadJSON["name"]);
    _eC->setNTPEnabled(inputPayloadJSON["ntpEnabledSetting"]);
    _eC->setGMTOffset(inputPayloadJSON["gmtOffsetSetting"]);
    _eC->saveMainConfig();
    Serial.println("saved config");
}

void WebserverModule::receiveRelayConfigs(JsonDocument inputPayloadJSON) {
    int relayIndex = inputPayloadJSON["index"];
    _eC->setLEDSetting(relayIndex, inputPayloadJSON["ledSetting"]);
    _eC->setOperationMode(relayIndex, inputPayloadJSON["operationModeSetting"]);
    _eC->setRelayManualSetting(relayIndex, inputPayloadJSON["relayManualSetting"]);
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        _eC->getTimeSlot(relayIndex, i)->setIndex(inputPayloadJSON["timeSlots"][i]["index"]);
        _eC->getTimeSlot(relayIndex, i)->setEnabled(inputPayloadJSON["timeSlots"][i]["enabled"]);
        _eC->getTimeSlot(relayIndex, i)->setOnStartTimeISOString(inputPayloadJSON["timeSlots"][i]["onStartTime"], 
            _rtcntp->getRTCTime());
        _eC->getTimeSlot(relayIndex, i)->setOnEndTimeISOString(inputPayloadJSON["timeSlots"][i]["onEndTime"], 
            _rtcntp->getRTCTime());
    }
    _eC->setCountdownDuration(relayIndex, inputPayloadJSON["countdownDurationSetting"]);
    _eC->saveRelayConfig(relayIndex);
}

/**
 * when relay state is switched,
 * if in manual operation mode, set relay state to value.
 * else if in countdown timer mode, start or stop the countdown timer.
 */
void WebserverModule::switchRelayState(JsonDocument inputPayloadJSON) {
    int relayIndex = inputPayloadJSON["index"];
    if (_eC->getOperationMode(relayIndex) == 1) {
        _eC->setRelayManualSetting(relayIndex, inputPayloadJSON["relay_state"]);
    }
    else if (_eC->getOperationMode(relayIndex) == 3) {
        if (inputPayloadJSON["relay_state"]) {
            _eC->startCountdownTimer(relayIndex);
        }
        else {
            _eC->stopCountdownTimer(relayIndex);
        }
    }
}

void WebserverModule::receiveData(String cmd, String type, JsonDocument payloadJSON) {
    if (cmd == SAVE_CMD) {
        if (type == CONNECTION_TYPE) {
            receiveConnection(payloadJSON);
            if (_receiveConnectionFunc != NULL) {
                _receiveConnectionFunc();
            }
        }
        // else if (type == RELAY_STATES_TYPE) {
        //     receiveRelayState(payloadJSON);
        //     if (_receiveRelayStateFunc != NULL) {
        //         _receiveRelayStateFunc();
        //     }
        // }
        else if (type == DATETIME_TYPE) {
            receiveDateTime(payloadJSON);
            if (_receiveDateTimeFunc != NULL) {
                _receiveDateTimeFunc();
            }  
        }
        else if (type == MAIN_CONFIG_TYPE) {
            receiveMainConfig(payloadJSON);
            if (_receiveConfigFunc != NULL) {
                _receiveConfigFunc();
            }
        }
        else if (type == RELAY_CONFIGS_TYPE) {
            receiveRelayConfigs(payloadJSON);

        }

    }
    else if (cmd == SWITCH_CMD) {
        if (type == RELAY_STATES_TYPE) {
            switchRelayState(payloadJSON);
            if (_switchRelayStateFunc != NULL) {
                _switchRelayStateFunc();
            }
        }
    }
    
    else {
        _ws.textAll("Invalid save");
    }
}

void WebserverModule::setReceiveConnectionCallback(void (*callback)()){
    _receiveConnectionFunc = callback;
}

void WebserverModule::setReceiveRelayStateCallback(void (*callback)()){
    _receiveRelayStateFunc = callback;
}

void WebserverModule::setReceiveDateTimeCallback(void (*callback)()){
    _receiveDateTimeFunc = callback;
}

void WebserverModule::setReceiveConfigCallback(void (*callback)()){
    _receiveConfigFunc = callback;
}


