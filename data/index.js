import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    /*
    websocket configuration
    */
    // let port = window.location.port;
    // let hostname = window.location.hostname;
    // let port = 7778;
    // let hostname = "192.168.57.70";
    // let hostname = "192.168.5.70";
    let hostname = "192.168.4.1";
    let port = 7777;

    /*
    DOM elements
    */
    // relay status indicator and text
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
        const saveTimeBtn = document.getElementById("saveTimeBtn");
    // status LED mode input
    const ledModeInput = document.getElementById("ledModeInput");
    // operation mode selector input
    const operationModeInput = document.getElementById("operationModeInput");
    // manual relay state input div, which is only shown when automatic timer is disabled.
    const manualRelayDiv = document.getElementById("manualRelayDiv");
        // manual relay state input
        const manualRelayInput = document.getElementById("manualRelayInput");
    // time slots relay input div, which is only shown when automatic timer is enabled.
    const timeSlotsRelayDiv = document.getElementById("timeSlotsRelayDiv"); 
        // invisible timeSlot html template
        let timeSlotTemplate = document.getElementById("timeSlotTemplate");
        timeSlotTemplate.style.display = "none";
        // list of all timeslots
        const timeSlotsInputs = timeSlotsRelayDiv.getElementsByClassName("timeSlot");
        let timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        let timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        let timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        let timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
    // countdown timer div
    const countdownTimerDiv = document.getElementById("countdownTimerDiv");
        // countdown duration input
        const countdownDurationInput = document.getElementById("countdownDurationInput");
        const countdownStartStopButton = document.getElementById("startStopCountdownButton");
    // save config button
    const saveConfigBtn = document.getElementById("saveConfigBtn");

    /*
    Input eventListeners
    */
    // change eventListener for deviceNameInput
    deviceNameInput.addEventListener("change", () => {
        config.name = deviceNameInput.value;
    });

    // change eventListener for ntpEnableInput
    ntpEnableInput.addEventListener("change", () => {
        // set visible div when ntpEnableInput is changed
        updateNtpEnableDivDisplay();
        config.ntpEnabledSetting = ntpEnableInput.checked;
    });

    // change eventListener for gmtOffsetInput
    gmtOffsetInput.addEventListener("change", () => {
        config.gmtOffsetSetting = gmtOffsetInput.value;
    });

    // change eventListener for manualTimeInput
    manualTimeInput.addEventListener("click", () => {
        manualDateTimeVal.datetime = manualTimeInput.value;
        // console.log(`manualtimeInput = ${JSON.stringify(manualDateTimeVal)}`);
        // console.log(`manualtimeInput = ${manualDateTimeVal.datetime}`);
    });
    // change eventListener for manualTimeInput
    manualTimeInput.addEventListener("change", () => {
        manualDateTimeVal.datetime = manualTimeInput.value;
    });

    // click eventListener for saveTimeBtn
    saveTimeBtn.addEventListener("click", (event) => {
        event.preventDefault();
        if (!config.ntpEnabledSetting) {
            console.log(`save datetime ${JSON.stringify(manualDateTimeVal)}`);
            wsMod.saveDateTime(ws, manualDateTimeVal);
        }
    });

    // change eventListener for ledModeInput
    ledModeInput.addEventListener("change", () => {
        config.ledSetting = ledModeInput.value;
    });

    // change eventListener for timerEnableInput
    operationModeInput.addEventListener("change", () => {
        // set visible div when timerEnableInput is changed
        console.log(`operation mode ${operationModeInput.value}`);
        updateOperationModeDivDisplay();
        config.operationModeSetting = operationModeInput.value;
    });

    // change eventListener for manualRelayInput
    manualRelayInput.addEventListener("change", () => {
        config.relayManualSetting = manualRelayInput.checked;
        curRelayState.relay_state = manualRelayInput.checked;
        wsMod.switchRelayState(ws, curRelayState);
    });

    // Set the callbacks for the input elements inside each timeslot.
    function setTimeSlotsCallbacks() {
        // get the enabled, startTIme, endTime, and duration inputs for all timeSlots
        // from the DOM
        timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
        // set the change eventListeners for enabled input of each timeSlot
        // console.log(timeSlotsEnableds.length);
        timeSlotsEnableds.forEach(element => {
            element.addEventListener("change", () => {
                let tsIndex = element.dataset.tsIndex;
                config.timeSlots[tsIndex].enabled = element.checked;
                // console.log(JSON.stringify(config)); 
            });
        });
        /*
        if startTime is updated, calculate for duration.
        if endTime is updated, calculate for duration.
        if duration is updated, calculate for endTime.
        */
       // set the change eventListeners for startTime of each timeSlot
        timeSlotsStartTimes.forEach(element => {
            element.addEventListener("change", () => {
                let tsIndex = element.dataset.tsIndex;
                config.timeSlots[tsIndex].onStartTime = element.value + "Z";
                config.timeSlots[tsIndex].duration = calculateDuration(
                    config.timeSlots[tsIndex].onStartTime,
                    config.timeSlots[tsIndex].onEndTime
                );
                console.log(config.timeSlots[tsIndex].duration);
                element.parentElement.parentElement.getElementsByClassName("timeSlotDuration")[0]
                    .value = config.timeSlots[tsIndex].duration;
                // console.log(JSON.stringify(config)); 
            });
        });
        // set the change eventListeners for endTime of each timeSlot
        timeSlotsEndTimes.forEach(element => {
            element.addEventListener("change", () => {
                let tsIndex = element.dataset.tsIndex;
                config.timeSlots[tsIndex].onEndTime = element.value + "Z";
                config.timeSlots[tsIndex].duration = calculateDuration(
                    config.timeSlots[tsIndex].onStartTime,
                    config.timeSlots[tsIndex].onEndTime
                );
                console.log(config.timeSlots[tsIndex].duration);
                element.parentElement.parentElement.getElementsByClassName("timeSlotDuration")[0]
                    .value = config.timeSlots[tsIndex].duration;
                // console.log(JSON.stringify(config)); 
            });
        });
        // set the change eventListeners for duration of each timeSlot
        timeSlotsDurations.forEach(element => {
            element.addEventListener("change", () => {
                let tsIndex = element.dataset.tsIndex;
                if (element.value >=0 && element.value < 24*60*60-1) {
                    config.timeSlots[tsIndex].duration = element.value;
                }
                else if (element.value < 0) {
                    config.timeSlots[tsIndex].duration = 0;
                }
                else {
                    config.timeSlots[tsIndex].duration = 24*60*60-1;
                }
                element.value = config.timeSlots[tsIndex].duration;
                let endTimeString = calculateEndTime(config.timeSlots[tsIndex].onStartTime, config.timeSlots[tsIndex].duration);
                config.timeSlots[tsIndex].onEndTime = endTimeString + 'Z';
                element.parentElement.parentElement.getElementsByClassName("timeSlotEndTime")[0].value = endTimeString;              
                // 2024-09-27T04:01:00.981Z
                // console.log(config.timeSlots[tsIndex].duration);
            });
        });
    }
    
    countdownDurationInput.addEventListener('change', () => {
        config.countdownDurationSetting = countdownDurationInput.value;
    });

    countdownStartStopButton.addEventListener('click', () => {
        let countdowntimercmd = structuredClone(curRelayState);
        countdowntimercmd.relay_state = !countdowntimercmd.relay_state;
        wsMod.switchRelayState(ws, countdowntimercmd);
    });

    // click eventListener for saveConfigBtn
    saveConfigBtn.addEventListener("click", () => {
        wsMod.saveConfig(ws, config);
        console.log("save config");
    });



    /*
    variables for configuration and data
    */
    // current system date and time
    let curDateTimeVal = new Date(Date.UTC(2024, 0, 1, 0, 0, 0)).toISOString();
    // date and time to be saved when NTP is not enabled
    let manualDateTimeVal = {
        datetime: new Date(Date.UTC(2024,1,1,0,0,0)).toISOString(),
    };
    // current relay state 
    let curRelayState = {relay_state: false};
    // EEPROM configuration
    let config = {
        name: "",
        ntpEnabledSetting: false, 
        gmtOffsetSetting: 0,
        operationModeSetting: 0,
        ledSetting: 0,
        timeSlots: [
            {   
                index: 0,
                enabled: false, 
                onStartTime: new Date(2024, 1, 1, 0, 0, 0),
                onEndTime: new Date(2024, 1, 1, 1, 0, 0),
            },
        ],
        countdownDurationSetting: 0,
    };

    
    /*
    Websocket 
    */
    let ws;
    function initWS() {
        let url = `ws://${hostname}:${port}/ws`;
        console.log(`url=${url}`);
        ws = new WebSocket(url);

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
            setTimeout(initWS, 100);
        });
    }

    /*
    Websocket callback functions
    */
    /**
     * Callback function to update displayed system date and time
     */
    function updateDisplayedTime(payload) {
        curDateTimeVal = payload["datetime"]
        document.getElementById("systemTime").textContent = curDateTimeVal;
    }

    /**
     * Callback function to update current relay state display and update the text of the start/stop countdown timer button
     */
    function updateDisplayedRelayState(payload) {
        // console.log(JSON.stringify(payload));
        curRelayState.relay_state = payload["relay_state"];
        if (payload["relay_state"]) {
            relayStatusTextOutput.textContent = "On";
            relayStatusIndicatorOutput.classList.add("on");
            relayStatusIndicatorOutput.classList.remove("off");
            countdownStartStopButton.textContent = "Stop";
        }
        else {
            relayStatusTextOutput.textContent = "Off";
            relayStatusIndicatorOutput.classList.add("off");
            relayStatusIndicatorOutput.classList.remove("on");
            countdownStartStopButton.textContent = "Start";
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
        // update operation mode setting
        operationModeInput.value = config["operationModeSetting"];
        // set visible div based on timer enabled value
        updateOperationModeDivDisplay();
        manualRelayInput.checked = config["relayManualSetting"];
        // refresh all timeslots
        // remove all timeslots
        while (timeSlotsInputs.length > 1) {
            timeSlotsRelayDiv.removeChild(timeSlotsInputs[0]);
        }
        // refresh all timeslots
        for (let i in config["timeSlots"]) {
            // clone timeslot template, make it visible, and change the classname
            let newTimeSlot = timeSlotTemplate.cloneNode(true);
            newTimeSlot.classList.add("timeSlot");
            // get the elements of the new timeslot element from the DOM
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
            // console.log(`dataset.tsIndex = ${timeSlotIndex.dataset.tsIndex}`);
            // set the values from the config into the fields of the new timeslot element
            timeSlotIndex.textContent = config["timeSlots"][i]["index"];
            timeSlotEnabled.checked = config["timeSlots"][i]["enabled"];
            // get time ISO strings of start and end times
            let startTimeString = config["timeSlots"][i]["onStartTime"].slice(0,8);
            let endTimeString = config["timeSlots"][i]["onEndTime"].slice(0,8);
            // set values for onStartTime and onEndTime
            timeSlotStartTime.value = startTimeString;
            timeSlotEndTime.value = endTimeString;
            // set duration value in timeslots
            timeSlotDuration.value = calculateDuration(startTimeString, endTimeString);            
            // display new timeslot and append it to the timeslots div
            newTimeSlot.style.display = "block";
            timeSlotsRelayDiv.appendChild(newTimeSlot);
        }
        // set callbacks for all timeslots
        setTimeSlotsCallbacks();
        // get countdown duration value 
        countdownDurationInput.value = config["countdownDurationSetting"];
    }

    /**
     * Calculate duration of a timeslot in seconds given the start and end times 
     * of the day.
     * @param {String} startTimeString - starting time of the day in ISO time format.
     * @param {String} endTimeString - ending time of the day in ISO time format.
     * @returns {Number} duration - number of seconds between the starting time and ending time.
     */
    function calculateDuration(startTimeString, endTimeString) {
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
        // console.log(startTimeInDate.toISOString());
        // console.log(endTimeInDate.toISOString());
        // console.log(duration);
        if (duration < 0) {
            endTimeInDate.setUTCDate(endTimeInDate.getUTCDate() + 1);
        }
        duration = (endTimeInDate-startTimeInDate)/1000;
        return duration;
    }
    
    /**
     * Calculate end time given the start time and duration.
     * @param {String} startTimeString - starting time of the day in ISO time format.
     * @param {Number} duration - number of seconds to add to the starting time.
     * @returns {String} endTimeString - ending time of the day in ISO time format.
     */
    function calculateEndTime(startTimeString, duration) {
        let startTimeInDate = new Date();
        startTimeInDate.setUTCHours(startTimeString.slice(0,2));
        startTimeInDate.setUTCMinutes(startTimeString.slice(3,5));
        startTimeInDate.setUTCSeconds(startTimeString.slice(6,8));
        let endTimeInDate = new Date(startTimeInDate);
        endTimeInDate.setUTCSeconds(endTimeInDate.getUTCSeconds() + duration);
        // console.log(`start = ${startTimeInDate.toISOString()}`);
        // console.log(`end = ${endTimeInDate.toISOString().slice(11,19)}`);
        let endTimeString = endTimeInDate.toISOString().slice(11,19);
        return endTimeString;
    }

    /**
     * Callback function to set the visible div depending on ntpEnableInput value.
     * If NTP is enabled, display the div with the input to set GMT offset. 
     * Else, display the div to set the system date and time manually. 
     */
    function updateNtpEnableDivDisplay() {
        manualTimeInput.value = curDateTimeVal.slice(0,19);
        manualDateTimeVal.datetime = curDateTimeVal.slice(0,19);
        // console.log(`manualTimeInput.value = ${manualTimeInput.value}`);
        if (ntpEnableInput.checked) {
            manualTimeInputDiv.style.display = "none";
            gmtOffsetInputDiv.style.display = "block";
        } else {
            manualTimeInputDiv.style.display = "block";
            gmtOffsetInputDiv.style.display = "none";
            // const now = new Date();
            // const year = now.getFullYear();
            // const month = String(now.getMonth() + 1).padStart(2, '0'); // Months are 0-indexed, so +1
            // const day = String(now.getDate()).padStart(2, '0');
            // const hours = String(now.getHours()).padStart(2, '0');
            // const minutes = String(now.getMinutes()).padStart(2, '0');
            // manualTimeInput.value = `${year}-${month}-${day}T${hours}:${minutes}`;
            // 2024-09-25T01:08:34Z
        }
    }

    /**
     * Callback function to set the visible div depending on timerEnableInput value.
     * If timerEnableInput is enabled, display the div of timeslots.
     * Else if disabled, display the manual relay div.
     */
    function updateOperationModeDivDisplay() {
        manualRelayDiv.style.display = "none";
        timeSlotsRelayDiv.style.display = "none";
        countdownTimerDiv.style.display = "none";
        switch (operationModeInput.value) {
            case '1':
                manualRelayDiv.style.display = "block";
                break;
            case '2':
                timeSlotsRelayDiv.style.display = "block";
                break;
            case '3':
                countdownTimerDiv.style.display = "block";
                break;
            default:
        }
    }

    // let element = document.querySelector('.timeSlot[data-index="1"]');
    // console.log(element);
    // element.getElementsByClassName('tsEnabledInput')[0].checked = true;
    // console.log(element.getElementsByClassName('tsEnabledInput')[0].checked);

    // initws
    initWS();
    // update all visible divs on page load
    updateNtpEnableDivDisplay();
    updateOperationModeDivDisplay();
});
