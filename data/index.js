import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    let port = 5555;
    // let hostname = window.location.hostname;
    let hostname = "192.168.5.70";
    // let hostname = "192.168.4.1";
    let url = `ws://${hostname}:${port}/ws`;
    console.log(`hostname=${hostname}`);
    let ws = new WebSocket(url);

    // DOM elements
    // relay status indicator text and visual
    const relayStatusTextOutput = document.getElementById("relayStatusText");
    const relayStatusIndicatorOutput = document.getElementById("relayStatusIndicator");
    // device name input
    const deviceNameInput = document.getElementById("deviceNameInput");
    // NTP enable input
    const ntpEnableInput = document.getElementById("ntpEnableInput");
    // GMT offset input div which is only visible if NTP is enabled.
    const gmtOffsetInputDiv = document.getElementById("gmtOffsetInputDiv");
        const gmtOffsetInput = document.getElementById("gmtOffsetInput");
    // manual time input div which will only be shown if NTP is disabled.
    const manualTimeInputDiv = document.getElementById("manualTimeInputDiv");
        const manualTimeInput = document.getElementById("manualTimeInput");
    // status LED mode input
    const ledModeInput = document.getElementById("ledModeInput");
    // automatic timer enable input
    const timerEnableInput = document.getElementById("timerEnableInput");
    // manual relay state input div, which is only shown when automatic timer is disabled.
    const manualRelayDiv = document.getElementById("manualRelayDiv");
        // manual relay state input
        const manualRelayInput = document.getElementById("manualRelayInput");
    // time slots relay input div, which is only shown when automatic timer is enabled.
    const timeSlotsRelayDiv = document.getElementById("timeSlotsRelayDiv"); 
        // list of all timeslots
        const timeSlotsInputs = timeSlotsRelayDiv.getElementsByClassName("timeSlot");
    // save config button
    const saveConfigBtn = document.getElementById("saveConfigBtn");

    /*
    TODO:
    .load all configuration from ESP32 on startup 
    .update system date and time display
    .load all timeslots
    .create variables to hold the local configuration
    */
    
    // variables for configuration and data
    let curDateTimeVal = new Date(Date.UTC(2024, 0, 1, 0, 0, 0)).toISOString();
    console.log(curDateTimeVal);
    let curRelayStateVal = false;
    let config = {
        name: "",
        ntpEnabledSetting: false, 
        gmtOffsetSetting: 0,
        timerEnabledSetting: false,
        ledSetting: 0,
        timeSlots: [
            {   
                index: 0,
                enabled: false, 
                onStartTime: new Date(2024, 1, 1, 0, 0, 0),
                onEndTime: new Date(2024, 1, 1, 1, 0, 0),
            },
        ]
    };

    updateNtpEnableDivDisplay();
    updateTimerEnableDivDisplay();

    // request for the date and time, relay state, and configuration upon websocket open
    ws.addEventListener("open", (event) => {
        wsMod.requestDateTime(ws);
        wsMod.requestRelayState(ws);
        wsMod.requestConfig(ws);
    })

    // websocket message handler
    ws.addEventListener("message", (event) => {
        // define callbacks for receiving different types of data
        // callbacks[cmd][type] = function(payload)
        let callbacks = {
            "datetime": updateDisplayedTime,
            "relay_state": updateDisplayedRelayState,
            "config": updateDisplayedConfig,
        }
        wsMod.receiveData(event, callbacks);
    });

    // websocket close handler
    ws.addEventListener("close", (event) => {
        console.log("ws closed");
    });

    /**
     * Callback function to update displayed system date and time
     */
    function updateDisplayedTime(payload) {
        document.getElementById("systemTime").textContent = payload["datetime"];
    }

    /**
     * Callback function to update displayed relay state
     */
    function updateDisplayedRelayState(payload) {
        console.log(JSON.stringify(payload));
        if (payload["relay_state"]) {
            relayStatusTextOutput.textContent = "On";
            relayStatusIndicatorOutput.classList.add("on");
            relayStatusIndicatorOutput.classList.remove("off");
        }
        else {
            relayStatusTextOutput.textContent = "Off";
            relayStatusIndicatorOutput.classList.add("off");
            relayStatusIndicatorOutput.classList.remove("on");
        }
    }

    /**
     * Callback function to update displayed configuration
     */
    function updateDisplayedConfig(payload) {
        // update name input value
        deviceNameInput.value = payload["name"];
        // update NTP enable value
        ntpEnableInput.checked = payload["ntpEnabledSetting"];
        // update GMT offset value 
        gmtOffsetInput.value = payload["gmtOffsetSetting"];
        updateNtpEnableDivDisplay();
        ledModeInput.value = payload["ledSetting"];
        timerEnableInput.checked = payload["timerEnabledSetting"];
        updateTimerEnableDivDisplay();
        manualRelayInput.checked = payload["relayManualSetting"];

        while (timeSlotsInputs.length > 1) {
            timeSlotsRelayDiv.removeChild(timeSlotsInputs[0]);
        }
        let timeSlotTemplate = timeSlotsRelayDiv.getElementsByClassName("timeSlotTemplate")[0];
        console.log(`timeSlotTemplate = ${timeSlotTemplate}`);
        timeSlotTemplate.style.display = "none";
        for (let i in payload["timeSlots"]) {
            console.log(i);
            let newTimeSlot = timeSlotTemplate.cloneNode(true);
            newTimeSlot.classList.add("timeSlot");
            newTimeSlot.classList.remove("timeSlotTemplate");
            let timeSlotIndex = newTimeSlot.getElementsByClassName("index")[0];
            let timeSlotEnabled = newTimeSlot.getElementsByClassName("enabled")[0]; 
            let timeSlotStartTime = newTimeSlot.getElementsByClassName("startTime")[0];  
            let timeSlotEndTime = newTimeSlot.getElementsByClassName("endTime")[0];  
            let timeSlotDuration = newTimeSlot.getElementsByClassName("duration")[0];  
            timeSlotIndex.textContent = payload["timeSlots"][i]["index"];
            timeSlotEnabled.checked = payload["timeSlots"][i]["enabled"];
            timeSlotStartTime.value = payload["timeSlots"][i]["onStartTime"].slice(0,8);
            timeSlotEndTime.value = payload["timeSlots"][i]["onEndTime"].slice(0,8);
            // let endDate
            // timeSlotStartTime.value = payload["timeSlots"][i]["onStartTime"];
            newTimeSlot.style.display = "block";
            timeSlotsRelayDiv.appendChild(newTimeSlot);
        }
        // console.log(`timeslots length = ${payload["timeSlots"].length}`);
    }

    ntpEnableInput.addEventListener("change", (event) => {
        updateNtpEnableDivDisplay();
    });

    /**
     * update the time setting div based on if NTP is enabled.
     * If NTP is enabled, display the div with the input to set GMT offset. 
     * Else, display the div to set the system date and time manually. 
     */
    function updateNtpEnableDivDisplay() {
        if (ntpEnableInput.checked) {
            manualTimeInputDiv.style.display = "none";
            gmtOffsetInputDiv.style.display = "block";
        } else {
            manualTimeInputDiv.style.display = "block";
            gmtOffsetInputDiv.style.display = "none";
            const now = new Date();
            const year = now.getFullYear();
            const month = String(now.getMonth() + 1).padStart(2, '0'); // Months are 0-indexed, so +1
            const day = String(now.getDate()).padStart(2, '0');
            const hours = String(now.getHours()).padStart(2, '0');
            const minutes = String(now.getMinutes()).padStart(2, '0');
            manualTimeInput.value = `${year}-${month}-${day}T${hours}:${minutes}`;
            // 2024-09-25T01:08:34Z
        }
    }

    timerEnableInput.addEventListener("change", (event) => {
        updateTimerEnableDivDisplay();
    });

    function updateTimerEnableDivDisplay() {
        if (timerEnableInput.checked) {
            manualRelayDiv.style.display = "none";
            timeSlotsRelayDiv.style.display = "block";
        } else {
            manualRelayDiv.style.display = "block";
            timeSlotsRelayDiv.style.display = "none";
        }
    }

    // let element = document.querySelector('.timeSlot[data-index="1"]');
    // console.log(element);
    // element.getElementsByClassName('tsEnabledInput')[0].checked = true;
    // console.log(element.getElementsByClassName('tsEnabledInput')[0].checked);
    
});
