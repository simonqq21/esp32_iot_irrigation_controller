document.addEventListener("DOMContentLoaded", function() {
    let port = 5555;
    // let hostname = window.location.hostname;
    let hostname = "192.168.5.71";
    // let hostname = "192.168.4.1";
    let url = `ws://${hostname}:${port}/ws`;
    console.log(`hostname=${hostname}`);
    let ws = new WebSocket(url);

    let connectSSID;
    let connectSecurity;
    let connectPass;
    let connectPort;
    let connectIPIndex;

    function requestWifis(ws) {
        msg = {cmd: "request",
            type: "wifis",
        };
        ws.send(JSON.stringify(msg));
    }
    
    function requestDateTime(ws) {
        msg = {cmd: "request",
            type: "datetime",
        };
        ws.send(JSON.stringify(msg));
    }
    
    function sendConnection(ws) {
        msg = {};
        msg["cmd"] = "save";
        msg["type"] = "connection";
        msg["payload"] = {};
        payload = msg["payload"];
        payload["ssid"] = connectSSID;
        payload["pass"] = connectPass;
        payload["ipIndex"] = connectIPIndex;
        payload["port"] = connectPort;
        console.log(JSON.stringify(msg));
        ws.send(JSON.stringify(msg));
    }
    
    function updateDisplayTime(payload) {
        document.getElementById("systemTime").textContent = payload["datetime"];
    }
    
    function updateWiFis(payload) {
        console.log(JSON.stringify(payload));
        let wifilisttbody = document.getElementById("wifilisttable").getElementsByTagName("tbody")[0];
        let wifilistrows = wifilisttbody.getElementsByTagName("tr");
        
        while (wifilistrows.length) {
            wifilisttbody.removeChild(wifilistrows[0]);
        }
        let wifiIndex = 0;
        for (wifi of payload["wifis"]) {
            console.log(wifi["ssid"]);
            let wifitablerow = document.createElement("tr");
            wifitablerow.classList.add("wifihotspot");
            wifitablerow.setAttribute("wifiindex", wifiIndex);
            wifiIndex++;
    
            let wifiname = document.createElement("td");
            wifiname.classList.add("wifiname");
            wifiname.textContent = wifi["ssid"];
            wifitablerow.appendChild(wifiname);
    
            let wifisecurity = document.createElement("td");
            wifisecurity.classList.add("wifisecurity");
            wifisecurity.textContent = wifi["security"];
            wifitablerow.appendChild(wifisecurity);
    
            let wifirssi = document.createElement("td");
            wifirssi.textContent = `${wifi["rssi"]} dBm`;
            wifitablerow.appendChild(wifirssi);
    
            let wificonnectbtntd = document.createElement("td");
            let wificonnectbtn = document.createElement("button");
            wificonnectbtn.textContent = "Connect";
            wificonnectbtn.addEventListener("click", function() {
                let wifilistrow = this.parentElement.parentElement;
                connectSSID = wifilistrow.getElementsByClassName("wifiname")[0].textContent;
                connectSecurity = wifilistrow.getElementsByClassName("wifisecurity")[0].textContent;
                // alert(connectSecurity);
                setPage("connect");
            });
            wificonnectbtn.id = "";
            wificonnectbtntd.appendChild(wificonnectbtn);
            wifitablerow.appendChild(wificonnectbtntd);
            wifilisttbody.appendChild(wifitablerow);
        }
        // console.log(`l${wifilistrows.length}`);
        // for (row of wifilistrows) {
        //     console.log(row.getAttribute("wifiindex"));
        // }
    }
    
    function setPage(pagename) {
        let listwifipage = document.getElementById("listwifipage");
        let wificonnectpage = document.getElementById("wificonnectpage");
    
        if (pagename == "listwifi") {
            listwifipage.style.display = "block";
            wificonnectpage.style.display = "none";
        }
        else if (pagename == "connect") {
            listwifipage.style.display = "none";
            wificonnectpage.style.display = "block";
            console.log(connectSecurity);
            if (connectSecurity == "open") {
                document.getElementById("passwordinputdiv").style.display = "none";
            }
            else {
                document.getElementById("passwordinputdiv").style.display = "block";
            }
        }
    }




    ws.addEventListener("open", (event) => {
        requestWifis(ws);
    })

    ws.addEventListener("message", (event) => {
        // console.log(`message received: ${event.data}`);
        let jsonMsg = JSON.parse(event.data);
        let cmd = jsonMsg['cmd'];
        let type = jsonMsg['type'];
        let payload = jsonMsg["payload"];

        if (cmd == "load") {
            if (type == "datetime") {
                updateDisplayTime(payload);
            }
            else if (type == "wifis") {
                updateWiFis(payload);
            }
        }
        console.log(`${cmd}, ${type}, ${JSON.stringify(payload)}`);
    });

    ws.addEventListener("close", (event) => {
        console.log("ws closed");
    });

    let connectPassInput = document.getElementById("passwordinputdiv").getElementsByTagName("input")[0];
    let connectPortInput = document.getElementById("portinputdiv").getElementsByTagName("input")[0];
    let connectIPIndexInput = document.getElementById("ipindexdiv").getElementsByTagName("input")[0];

    connectPassInput.addEventListener("change", function() {
        connectPass = connectPassInput.value;
        // alert(connectPass);
    });

    connectPortInput.addEventListener("change", function() {
        connectPort = connectPortInput.value;
        // alert(connectPort);
    });

    connectIPIndexInput.addEventListener("change", function() {
        connectIPIndex = connectIPIndexInput.value;
        // alert(connectIPIndex);
    });

    document.getElementById("wificonnectpagebackbtn").addEventListener("click", function() {
        setPage("listwifi");
    });
    
    document.getElementById("wificonnectpageconnectbtn").addEventListener("click", function() {
        console.log("Connect!");
        sendConnection(ws);
    });

    setPage("listwifi");
});