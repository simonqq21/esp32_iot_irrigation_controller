import { requestWifis, sendConnection } from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    // set page to the WiFi hotspot list upon load.
    setPage("listwifi");
    
    let portValTemp = 5555;
    // let hostname = window.location.hostname;
    let hostname = "192.168.5.70";
    // let hostname = "192.168.4.1";
    // let hostname = "192.168.34.2";
    let url = `ws://${hostname}:${portValTemp}/ws`;
    console.log(`hostname=${hostname}`);
    let ws = new WebSocket(url);

    // WiFi ssid, security, password, port, and IP address index variables
    let ssidVal = "";
    let securityVal = "open";
    let passVal = "";
    let portVal = -1;
    let ipIndexVal = -1;
    
    /**
     * Updates the displayed system time. 
     * 
     * @typedef {Object} payload - the payload object containing the ISO datetime string.
     * @param {String} payload.datetime - the ISO datetime string of the ESP32 system time.
     */
    function updateDisplayTime(payload) {
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
    function updateWiFis(payload) {
        console.log(JSON.stringify(payload));
        // get the table body of the WiFi hotspot list table
        let wifilisttbody = document.getElementById("wifilisttable").getElementsByTagName("tbody")[0];
        
        // clear all existing rows from the WiFi hotspot table
        let wifilistrows = wifilisttbody.getElementsByTagName("tr");
        while (wifilistrows.length) {
            wifilisttbody.removeChild(wifilistrows[0]);
        }

        let wifiIndex = 0;
        // iterate over each WiFi hotspot
        for (let wifi of payload["wifis"]) {
            console.log(wifi["ssid"]);
            // create table row for the current WiFi hotspot
            let wifitablerow = document.createElement("tr");
            wifitablerow.classList.add("wifihotspot");
            wifitablerow.setAttribute("wifiindex", wifiIndex);
            wifiIndex++;
            
            // create and append current WiFi hotspot SSID to the table row
            let wifiname = document.createElement("td");
            wifiname.classList.add("wifiname");
            wifiname.textContent = wifi["ssid"];
            wifitablerow.appendChild(wifiname);
            
            // create and append current WiFi hotspot security to the table row
            let wifisecurity = document.createElement("td");
            wifisecurity.classList.add("wifisecurity");
            wifisecurity.textContent = wifi["security"];
            wifitablerow.appendChild(wifisecurity);
            
            // create and append current WiFi hotspot rssi to the table row
            let wifirssi = document.createElement("td");
            wifirssi.textContent = `${wifi["rssi"]} dBm`;
            wifitablerow.appendChild(wifirssi);
            
            // create a table data containing a button to open up the connection menu for its
            // corresponding WiFi hotspot.
            let wificonnectbtntd = document.createElement("td");
            let wificonnectbtn = document.createElement("button");
            wificonnectbtn.textContent = "Connect";

            // add click function
            // when the connect button is clicked, the SSID and security of the corresponding WiFi
            // hotspot are saved, then the connection menu is opened.
            wificonnectbtn.addEventListener("click", function() {
                let wifilistrow = this.parentElement.parentElement;
                ssidVal = wifilistrow.getElementsByClassName("wifiname")[0].textContent;
                securityVal = wifilistrow.getElementsByClassName("wifisecurity")[0].textContent;
                setPage("connect");
            });
            wificonnectbtntd.appendChild(wificonnectbtn);
            wifitablerow.appendChild(wificonnectbtntd);

            // append the current WiFi hotspot table row to the table.
            wifilisttbody.appendChild(wifitablerow);
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
            listwifipage.style.display = "none";
            wificonnectpage.style.display = "block";
            console.log(securityVal);
            // only display the password input if the selected WiFi hotspot requires a PIN/password to connect.
            if (securityVal == "open") {
                document.getElementById("passwordinputdiv").style.display = "none";
            }
            else {
                document.getElementById("passwordinputdiv").style.display = "block";
            }
        }
    }

    // request for the list of scanned WiFi hotspots upon websocket open
    ws.addEventListener("open", (event) => {
        requestWifis(ws);
    })

    // websocket message handler
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
        passVal = connectPassInput.value;
        // alert(connectPass);
    });

    // save port number to variable upon input change 
    connectPortInput.addEventListener("change", function() {
        portVal = connectPortInput.value;
        // alert(connectPort);
    });

    // save IP address index number to variable upon input change 
    connectIPIndexInput.addEventListener("change", function() {
        ipIndexVal = connectIPIndexInput.value;
        // alert(connectIPIndex);
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
        sendConnection(ws, 
            {
                ssid: ssidVal, 
                pass: passVal, 
                ipIndex: ipIndexVal, 
                port: portVal
            }
        );

    });
});