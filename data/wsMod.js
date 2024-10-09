// functions for requesting data from ESP32
/**
 * Request data from the ESP32.
 * @param {WebSocket} ws - websocket instance
 * @param {String} typeOfData - type of data to request.
 */
function requestData(ws, typeOfData) {
    let msg = {cmd: "request",
        type: typeOfData,
    };
    ws.send(JSON.stringify(msg));
}
/**
 * Request for the list of scanned WiFi hotspots from the ESP32.
 * @param {WebSocket} ws - websocket instance
 */
export function requestWifis(ws) {
    requestData(ws, "wifis");
}
/**
 * Request for the current connection details from the ESP32.
 * @param {WebSocket} ws - websocket instance
 */
export function requestConnection(ws) {
    requestData(ws, "connection");
}
/**
 * Request for the current relay state from the ESP32.
 * @param {WebSocket} ws - websocket instance
 */
export function requestRelayState(ws) {
    requestData(ws, "relay_state");
}
/**
 * Request for the system date and time from the ESP32.
 * @param {WebSocket} ws - websocket instance
 */
export function requestDateTime(ws) {
    requestData(ws, "datetime");
}
/**
 * Request for the configuration from the ESP32.
 * @param {WebSocket} ws - websocket instance
 */
export function requestConfig(ws) {
    requestData(ws, "config");
}



// functions for receiving data from ESP32 
/**
 * Receive data from the ESP32 and execute callbacks to update the displayed
 * data on the browser.
 * 
 * @param {Event} event - event containing the data from the websocket.
 * @param {Object} callbacks - object containing key-value pairs.
 * @param {String} callbacks.keys() - keys are String types of data
 * @param {Function} callback.values() - values are functions that receive
 *  the payload from the websocket as a single parameter.
 */
export function receiveData(event, callbacks) {
    let jsonMsg = JSON.parse(event.data);
    let cmd = jsonMsg['cmd'];
    let type = jsonMsg['type'];
    let payload = jsonMsg["payload"];
    console.log(`${cmd}, ${type}, ${JSON.stringify(payload)}`);

    // call callback functions depending on the cmd and type values with the payload
    // from the websocket.
    callbacks[type](payload);
}



// functions for sending data to ESP32
/**
 * Save data to the ESP32. 
 * @param {WebSocket} ws - websocket instance
 * @param {String} cmd - command string
 * @param {String} typeOfData - type of data to save.
 * @param {Object} payload - payload of data to save.
 */
function sendData(ws, cmd, typeOfData, payload) {
    let msg = {};
    msg["cmd"] = cmd;
    msg["type"] = typeOfData;
    msg["payload"] = payload;
    console.log(JSON.stringify(msg));
    ws.send(JSON.stringify(msg));
}

/**
 * Save connection credentials and configuration to the ESP32.
 * @param {WebSocket} ws - websocket instance
 * @param {Object} payload - object containing connection credentials 
 * and configuration to be saved to the ESP32.
 * @param {String} payload.ssid - SSID of the WiFi hotspot to connect to.
 * @param {String} payload.pass - password of the WiFi hotspot to connect to.
 * @param {Number} payload.ipIndex - IP address index for the static IP of the ESP32.
 * @param {Number} payload.port - port number of the ESP32.
 */
export function saveConnection(ws, payload) {
    sendData(ws, "save", "connection", payload);
}

/**
 * Save manual relay state to the ESP32.
 * @param {WebSocket} ws - websocket instance
 * @param {Object} payload - object containing manual relay state to be saved 
 * to the ESP32.
 * @param {Boolean} payload.relay_state - manual relay state
 */
export function saveRelayState(ws, payload) {
    sendData(ws, "save", "relay_state", payload);
}

/**
 * Save system date and time to the ESP32.
 * @param {WebSocket} ws - websocket instance
 * @param {Object} payload - object containing system date and time to be saved 
 * to the ESP32.
 * @param {String} payload.datetime - system date and time in ISO format
 */
export function saveDateTime(ws, payload) {
    sendData(ws, "save", "datetime", payload);
}

/**
 * Save  main configuration to the ESP32.
 * @param {WebSocket} ws - websocket instance
 * @param {Object} payload - object containing main configuration to be saved 
 * to the ESP32.
 * @param {String} payload.name - name of the ESP32 relay
 * @param {Boolean} payload.ntpEnabledSetting - NTP time enabled setting
 * @param {Number} payload.gmtOffsetSetting - GMT offset in hours
 * @param {Boolean} payload.timerEnabledSetting - automatic relay timer enabled setting
 * @param {Number} payload.ledSetting - integer status LED setting
 * @param {Object[]} payload.timeSlots[] - array of timeslot objects
 * @param {Number} payload.timeSlots[].index - index of each timeslot
 * @param {Boolean} payload.timeSlots[].enabled - enabled setting of each timeslot
 * @param {String} payload.timeSlots[].onStartTime - start time of each timeslot in ISO format
 * @param {String} payload.timeSlots[].onEndTime - end time of each timeslot in ISO format
 */
export function saveConfig(ws, payload) {
    sendData(ws, "save", "config", payload);
}

export function switchRelayState(ws, payload) {
    sendData(ws, "switch", "relay_state", payload);
}