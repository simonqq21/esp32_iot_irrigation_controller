import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    let port = 7777;
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
        // invisible timeSlot html template
        let timeSlotTemplate = document.getElementsByClassName("timeSlotTemplate")[0];
        timeSlotTemplate.style.display = "none";
        // list of all timeslots
        const timeSlotsInputs = timeSlotsRelayDiv.getElementsByClassName("timeSlot");
        // let timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        let timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        let timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        let timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        let timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
    // save config button
    const saveConfigBtn = document.getElementById("saveConfigBtn");

    deviceNameInput.addEventListener("change", () => {
        config.name = deviceNameInput.value;
    });

    // change eventListener for ntpEnableInput
    ntpEnableInput.addEventListener("change", () => {
        // set visible div when ntpEnableInput is changed
        updateNtpEnableDivDisplay();
        config.ntpEnableInput = ntpEnableInput.value;
    });

    gmtOffsetInput.addEventListener("change", () => {
        config.gmtOffsetInput = gmtOffsetInput.value;
    });

    manualTimeInput.addEventListener("change", () => {
        manualDateTimeVal = manualTimeInput.value;
        // alert(manualDateTimeVal);
    });

    ledModeInput.addEventListener("change", () => {
        config.ledSetting = ledModeInput.value;
    });

    timerEnableInput.addEventListener("change", () => {
        // set visible div when timerEnableInput is changed
        updateTimerEnableDivDisplay();
        config.timerEnabledSetting = timerEnableInput.value;
    });

    manualRelayInput.addEventListener("change", () => {
        config.relayManualSetting = manualRelayInput.value;
    });

    /**
     * Setsthe callbacks for the input elements inside each timeslot.
     */
    function setTimeSlotsCallbacks() {
        timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
        // console.log(timeSlotsEnableds.length);
        timeSlotsEnableds.forEach(element => {
            element.addEventListener("change", () => {
                let tsIndex = element.dataset.tsIndex;
                config.timeSlots[tsIndex].enabled = element.checked;
                console.log(JSON.stringify(config)); 
            });
        });
        
    }

    /*
    TODO:
    .load all configuration from ESP32 on startup 
    .update system date and time display
    .load all timeslots
    .create variables to hold the local configuration
    */
    
    // variables for configuration and data
    let curDateTimeVal = new Date(Date.UTC(2024, 0, 1, 0, 0, 0)).toISOString();
    let manualDateTimeVal = new Date(Date.UTC(2024,1,1,0,0,0)).toISOString();
    // console.log(`manualDateTimeVal = ${manualDateTimeVal}`);
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

    // update the visible divs based on certain settings
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
            "config": receiveConfigCallback,
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
     * Callback function to update current relay state display
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
    function receiveConfigCallback(payload) {
        // save payload into config
        config = payload;
        // update name input value
        deviceNameInput.value = config["name"];
        // update NTP enable value
        ntpEnableInput.checked = config["ntpEnabledSetting"];
        // update GMT offset value 
        gmtOffsetInput.value = config["gmtOffsetSetting"];
        // set visible div based on NTP enabled value
        updateNtpEnableDivDisplay();
        // update status LED value
        ledModeInput.value = config["ledSetting"];
        // update timer enabled value
        timerEnableInput.checked = config["timerEnabledSetting"];
        // set visible div based on timer enabled value
        updateTimerEnableDivDisplay();
        manualRelayInput.checked = config["relayManualSetting"];
        // remove all timeslots
        while (timeSlotsInputs.length > 1) {
            timeSlotsRelayDiv.removeChild(timeSlotsInputs[0]);
        }
        // refresh all timeslots
        for (let i in config["timeSlots"]) {
            // clone timeslot template, make it visible, and change the classname
            let newTimeSlot = timeSlotTemplate.cloneNode(true);
            newTimeSlot.classList.add("timeSlot");
            newTimeSlot.classList.remove("timeSlotTemplate");
            // get the fields of the new timeslot element
            let timeSlotIndex = newTimeSlot.getElementsByClassName("timeSlotIndex")[0];
            let timeSlotEnabled = newTimeSlot.getElementsByClassName("timeSlotEnabled")[0]; 
            let timeSlotStartTime = newTimeSlot.getElementsByClassName("timeSlotStartTime")[0];  
            let timeSlotEndTime = newTimeSlot.getElementsByClassName("timeSlotEndTime")[0];  
            let timeSlotDuration = newTimeSlot.getElementsByClassName("timeSlotDuration")[0]; 
            // add attributes indicating the index of the timeslot
            timeSlotIndex.dataset.tsIndex = i;
            timeSlotEnabled.dataset.tsIndex = i;
            timeSlotStartTime.dataset.tsIndex = i;
            timeSlotEndTime.dataset.tsIndex = i;
            timeSlotDuration.dataset.tsIndex = i;
            console.log(`dataset.tsIndex = ${timeSlotIndex.dataset.tsIndex}`);
            // set the values from the config into the fields of the new timeslot element
            timeSlotIndex.textContent = config["timeSlots"][i]["index"];
            timeSlotEnabled.checked = config["timeSlots"][i]["enabled"];
            // get time ISO strings of start and end times
            let startTimeString = config["timeSlots"][i]["onStartTime"].slice(0,8);
            let endTimeString = config["timeSlots"][i]["onEndTime"].slice(0,8);
            // set values for onStartTime and onEndTime
            timeSlotStartTime.value = startTimeString;
            timeSlotEndTime.value = endTimeString;
            // calculate the duration in seconds
            let startTimeInDate = new Date();
            startTimeInDate.setUTCHours(startTimeString.slice(0,2));
            startTimeInDate.setUTCMinutes(startTimeString.slice(3,5));
            startTimeInDate.setUTCSeconds(startTimeString.slice(6,8));
            let endTimeInDate = new Date();
            endTimeInDate.setUTCHours(endTimeString.slice(0,2));
            endTimeInDate.setUTCMinutes(endTimeString.slice(3,5));
            endTimeInDate.setUTCSeconds(endTimeString.slice(6,8));
            let duration = (endTimeInDate-startTimeInDate)/1000;
            console.log(startTimeInDate.toISOString());
            console.log(endTimeInDate.toISOString());
            console.log(duration);
            if (duration < 0) {
                endTimeInDate.setUTCDate(endTimeInDate.getUTCDate() + 1);
            }
            duration = (endTimeInDate-startTimeInDate)/1000;
            // set duration value in timeslots
            timeSlotDuration.value = duration;            
            // display new timeslot and append it to the timeslots div
            newTimeSlot.style.display = "block";
            timeSlotsRelayDiv.appendChild(newTimeSlot);
        }
        
        setTimeSlotsCallbacks();
    }

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

    /**
     * Callback function to set the visible div depending on timerEnableInput value.
     * If timerEnableInput is enabled, display the div of timeslots.
     * Else if disabled, display the manual relay div.
     */
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
