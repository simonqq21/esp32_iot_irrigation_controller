// functions for requesting data from ESP32
function requestData(ws, typeOfData) {
    let msg = {cmd: "request",
        type: typeOfData,
    };
    ws.send(JSON.stringify(msg));
}

export function requestWifis(ws) {
    requestData(ws, "wifis");
}

export function requestConnection(ws) {
    requestData(ws, "connection");
}

export function requestRelayState(ws) {
    requestData(ws, "relay_state");
}

export function requestDateTime(ws) {
    requestData(ws, "datetime");
}

export function requestConfig(ws) {
    requestData(ws, "config");
}



// functions for receiving data from ESP32 




// functions for sending data to ESP32
function saveData(ws, typeOfData, payload) {
    let msg = {};
    msg["cmd"] = "save";
    msg["type"] = typeOfData;
    msg["payload"] = payload;
    console.log(JSON.stringify(msg));
    ws.send(JSON.stringify(msg));
}

export function sendConnection(ws, payload) {
    saveData(ws, "connection", payload);
}

export function sendRelayState(ws, payload) {
    saveData(ws, "relay_state", payload);
}

export function sendDateTime(ws, payload) {
    saveData(ws, "datetime", payload);
}

export function sendConfig(ws, payload) {
    saveData(ws, "config", payload);
}