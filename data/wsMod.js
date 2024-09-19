export function requestWifis(ws) {
    let msg = {cmd: "request",
        type: "wifis",
    };
    ws.send(JSON.stringify(msg));
}

export function requestConnection(ws) {
    let msg = {cmd: "request",
        type: "connection",
    };
    ws.send(JSON.stringify(msg));
}

export function requestRelayState(ws) {
    let msg = {cmd: "request",
        type: "relay_state",
    };
    ws.send(JSON.stringify(msg));
}

export function requestDateTime(ws) {
    let msg = {cmd: "request",
        type: "datetime",
    };
    ws.send(JSON.stringify(msg));
}

export function requestConfig(ws) {
    let msg = {cmd: "request",
        type: "config",
    };
    ws.send(JSON.stringify(msg));
}

export function sendConnection(ws, payload) {
    console.log(`payload=${JSON.stringify(payload)}`);
    let msg = {};
    msg["cmd"] = "save";
    msg["type"] = "connection";
    msg["payload"] = payload;
    console.log(JSON.stringify(msg));
    ws.send(JSON.stringify(msg));
}

export function sendRelayState(ws) {

}

export function sendDateTime(ws) {

}

export function sendConfig(ws) {

}