import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    // set page to the WiFi hotspot list upon load.
    setPage("listwifi");

    // let port = window.location.port;
    // let hostname = window.location.hostname;
    // let hostname = "192.168.5.70";
    let hostname = "192.168.4.1";
    let port = 7777;
    let url = `ws://${hostname}:${port}/ws`;
    console.log(`url=${url}`);
    let ws = new WebSocket(url);

    // WiFi ssid, security, password, port, and IP address index variables
    let connectionConfig = {
        ssidVal: "",
        securityVal: "",
        passVal: "",
        portVal: "",
        ipIndexVal: -1,
    };
    
    /**
     * Updates the displayed system time. 
     * 
     * @typedef {Object} payload - the payload object containing the ISO datetime string.
     * @param {String} payload.datetime - the ISO datetime string of the ESP32 system time.
     */
    function updateDisplayedTime(payload) {
        document.getElementById("systemTime").textContent = payload["datetime"];
    }
    
    /**
     * Loads and updates the list of WiFi hotspots displayed in the WiFi configuration page.
     * 
     * @param {Object} payload - the payload object containing the array of wifi hotspots.
     * @param {Object[]} payload.wifis - the array of wifi hotspot objects.
     * @param {String} payload.wifis[].ssid - the SSID of each WiFi hotspot
     * @param {String} payload.wifis[].security - the security type of each WiFi hotspot 
     * @param {Number} payload.wifis[].rssi - the signal strength of each WiFi hotspot
     */
    function updateWiFiTable(payload) {
        console.log(JSON.stringify(payload));
        // get the table body of the WiFi hotspot list table
        let wifilisttbody = document.getElementById("wifilisttable").getElementsByTagName("tbody")[0];
        
        // clear all existing rows from the WiFi hotspot table class="wifihotspot" 
        let wifilistrows = wifilisttbody.getElementsByClassName("wifihotspot");
        while (wifilistrows.length) {
            wifilisttbody.removeChild(wifilistrows[0]);
        }
        let wifihotspotTemplate = document.getElementById("wifihotspotTemplate");

        let wifiIndex = 0;
        // iterate over each WiFi hotspot
        for (let wifi of payload["wifis"]) {
            console.log(wifi["ssid"]);
            // create table row for the current WiFi hotspot
            let newwifihotspot = wifihotspotTemplate.cloneNode(true);
            newwifihotspot.classList.add("wifihotspot");
            newwifihotspot.style.display = "block";
            newwifihotspot.setAttribute("wifiindex", wifiIndex);
            wifiIndex++;
            let wifiname = newwifihotspot.getElementsByClassName("wifihotspotName")[0];
            let wifisecurity = newwifihotspot.getElementsByClassName("wifihotspotSecurity")[0];
            let wifirssi = newwifihotspot.getElementsByClassName("wifihotspotRSSI")[0];
            let wificonnectBtn = newwifihotspot.getElementsByClassName("wifihotspotConnectBtn")[0];
        
            wifiname.textContent = wifi["ssid"];
            wifisecurity.textContent = wifi["security"];
            wifirssi.textContent = `${wifi["rssi"]} dBm`;
            // add click function
            // when the connect button is clicked, the SSID and security of the corresponding WiFi
            // hotspot are saved, then the connection menu is opened.
            wificonnectBtn.addEventListener("click", function() {
                let wifilistrow = this.parentElement.parentElement;
                connectionConfig.ssidVal = wifilistrow.getElementsByClassName("wifihotspotName")[0].textContent;
                connectionConfig.securityVal = wifilistrow.getElementsByClassName("wifihotspotSecurity")[0].textContent;
                setPage("connect");
            });
            wifilisttbody.append(newwifihotspot);
        }
        // console.log(`l${wifilistrows.length}`);
        // for (row of wifilistrows) {
        //     console.log(row.getAttribute("wifiindex"));
        // }
    }
    
    /**
     * Sets the displayed page between the wifi list page and connect page.
     * 
     * @param {String} pagename - the name of the page to make visible
     * If pagename == "listwifi", the wifi list page will be visible.
     * Else if pagename == "connect", the connection menu page will be visible.
     */
    function setPage(pagename) {
        let listwifipage = document.getElementById("listwifipage");
        let wificonnectpage = document.getElementById("wificonnectpage");
    
        if (pagename == "listwifi") {
            listwifipage.style.display = "block";
            wificonnectpage.style.display = "none";
        }
        else if (pagename == "connect") {
            // set WiFi SSID text
            let ssidNameSpan = document.getElementById("ssidNameSpan");
            ssidNameSpan.textContent = connectionConfig.ssidVal;
            listwifipage.style.display = "none";
            wificonnectpage.style.display = "block";
            console.log(connectionConfig.securityVal);
            // only display the password input if the selected WiFi hotspot requires a PIN/password to connect.
            if (connectionConfig.securityVal == "open") {
                document.getElementById("passwordinputdiv").style.display = "none";
            }
            else {
                document.getElementById("passwordinputdiv").style.display = "block";
            }
        }
    }

    // request for the list of scanned WiFi hotspots upon websocket open
    ws.addEventListener("open", (event) => {
        wsMod.requestWifis(ws);
    })

    // websocket message handler
    ws.addEventListener("message", (event) => {
        // define callbacks for receiving different types of data
        // callbacks[cmd][type] = function(payload)
        let callbacks = {
            "datetime": updateDisplayedTime,
            "wifis": updateWiFiTable,
        }
        wsMod.receiveData(event, callbacks);
    });

    // websocket close handler
    ws.addEventListener("close", (event) => {
        console.log("ws closed");
    });

    // password input, port number input, and IP address index number input
    let connectPassInput = document.getElementById("passwordinputdiv").getElementsByTagName("input")[0];
    let connectPortInput = document.getElementById("portinputdiv").getElementsByTagName("input")[0];
    let connectIPIndexInput = document.getElementById("ipindexdiv").getElementsByTagName("input")[0];

    // save password to variable upon input change
    connectPassInput.addEventListener("change", function() {
        connectionConfig.passVal = connectPassInput.value;
    });

    // save port number to variable upon input change 
    connectPortInput.addEventListener("change", function() {
        connectionConfig.portVal = connectPortInput.value;
    });

    // save IP address index number to variable upon input change 
    connectIPIndexInput.addEventListener("change", function() {
        connectionConfig.ipIndexVal = connectIPIndexInput.value;
    });

    // back button in connect page will return to the wifi list page when clicked.
    document.getElementById("wificonnectpagebackbtn").addEventListener("click", function(event) {
        setPage("listwifi");
    });
    
    // connect button in connect page will send the wifi credentials and connection config to the ESP32, 
    // restarting it to attempt reconnecting. 
    // If the credentials are correct, the ESP32 can be accessed through the WiFi network.
    // If the credentials are wrong, the ESP32 will open a wifi access point for initial configuration
    // and out-of-the-box standalone use.
    document.getElementById("wificonnectpageconnectbtn").addEventListener("click", function(event) {
        event.preventDefault();
        let payload = {
            ssid: connectionConfig.ssidVal, 
            pass: connectionConfig.passVal, 
            ipIndex: connectionConfig.ipIndexVal, 
            port: connectionConfig.portVal
        };
        console.log(`sendConnection: payload=${JSON.stringify(payload)}`);
        wsMod.saveConnection(ws, payload);
    });
});