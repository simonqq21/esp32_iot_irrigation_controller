import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    /*
    websocket configuration
    */
    let hostname;
    let port;
    if (window.location.hostname == "localhost") {
        // hostname = "192.168.57.70";
        // hostname = "192.168.5.70";
        hostname = "192.168.4.1";
        port = 7777;
    }
    else {
        port = window.location.port;
        hostname = window.location.hostname;
    }
    console.log(hostname);
    /*
    DOM elements
    */
    // relay states elements
    // invisible relay state template
    let relayStateTemplate = document.getElementById("relayStateTemplate");
    let relayStatesTable = document.getElementById("relayStatesTable");
    let relayStates = relayStatesTable.getElementsByClassName('relayState');
    // relay status indicator and text
    // const relayStatusTextOutput = document.getElementById("relayStatusText");
    // const relayStatusIndicator = document.getElementById("relayStatusIndicator");
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
    const mainStatusLEDModeInput = document.getElementById("mainStatusLEDModeInput");
    // save main config button
    const saveMainConfigBtn = document.getElementById("saveMainConfigBtn");
    
    // relay index selector
    const relayIndexSelector = document.getElementById("relayIndexSelector");
    // status LED mode input
    const ledModeInput = document.getElementById("ledModeInput");
    // operation mode selector input
    const operationModeInput = document.getElementById("operationModeInput");
    // manual relay state input div, which is only shown when operation mode of the selected relay index is manual.
    const manualRelayDiv = document.getElementById("manualRelayDiv");
        // manual relay state input
        const manualRelayInput = document.getElementById("manualRelayInput");
    // time slots relay input div, which is only shown when operation mode of the selected relay index is daily timer.
    const timeSlotsRelayDiv = document.getElementById("timeSlotsRelayDiv"); 
        // invisible timeSlot html template
        let timeSlotTemplate = document.getElementById("timeSlotTemplate");
        // list of all timeslots
        const timeSlotsInputs = timeSlotsRelayDiv.getElementsByClassName("timeSlot");
        let timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        let timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        let timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        let timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
    // countdown timer div, which is only shown if operation mode of the selected relay index is countdown mode.
    const countdownTimerDiv = document.getElementById("countdownTimerDiv");
        // countdown duration input
        const countdownDurationInput = document.getElementById("countdownDurationInput");
        // countdown start/stop button
        const countdownStartStopButton = document.getElementById("startStopCountdownButton");
    // save relay config for the selected relay index
    const saveRelayConfigBtn = document.getElementById('saveRelayConfigBtn');

    /*
    Input eventListeners
    */
    // change eventListener for deviceNameInput
    deviceNameInput.addEventListener("change", () => {
        mainConfig.name = deviceNameInput.value;
    });

    // change eventListener for ntpEnableInput
    ntpEnableInput.addEventListener("change", () => {
        // set visible div when ntpEnableInput is changed
        updateNtpEnableDivDisplay();
        mainConfig.ntpEnabledSetting = ntpEnableInput.checked;
    });

    // change eventListener for gmtOffsetInput
    gmtOffsetInput.addEventListener("change", () => {
        mainConfig.gmtOffsetSetting = gmtOffsetInput.value;
    });

    // click eventListener for manualTimeInput
    manualTimeInput.addEventListener("click", () => {
        manualDateTimeVal.datetime = manualTimeInput.value;
    });
    // change eventListener for manualTimeInput
    manualTimeInput.addEventListener("change", () => {
        manualDateTimeVal.datetime = manualTimeInput.value;
    });

    // click eventListener for saveTimeBtn
    saveTimeBtn.addEventListener("click", (event) => {
        event.preventDefault();
        if (!mainConfig.ntpEnabledSetting) {
            console.log(`save datetime ${JSON.stringify(manualDateTimeVal)}`);
            wsMod.saveDateTime(ws, manualDateTimeVal);
        }
    });

    // change eventListener for mainStatusLEDModeInput
    mainStatusLEDModeInput.addEventListener("change", () => {
        mainConfig.ledSetting = mainStatusLEDModeInput.value;
    });

    // click eventlistener for saveMainConfigBtn
    saveMainConfigBtn.addEventListener('click', () => {
        wsMod.saveMainConfig(ws, mainConfig);
        console.log(JSON.stringify(mainConfig));
        console.log("saved main config");
    });

    // change eventlistener for relay index selector
    relayIndexSelector.addEventListener("change", () => {
        getCurRelayIndex();
        updateRelayConfigsDisplay(curIndex);
    });

    // get the current relay index from the relay index selector
    function getCurRelayIndex() {
        curIndex = relayIndexSelector.value;
    }

    // change eventListener for ledModeInput
    ledModeInput.addEventListener("change", () => {
        relayConfigs[curIndex].ledSetting = ledModeInput.value;
    });

    // change eventListener for timerEnableInput
    operationModeInput.addEventListener("change", () => {
        // set visible div when timerEnableInput is changed
        console.log(`operation mode ${operationModeInput.value}`);
        updateOperationModeDivDisplay();
        relayConfigs[curIndex].operationModeSetting = operationModeInput.value;
    });

    // change eventListener for manualRelayInput
    manualRelayInput.addEventListener("change", () => {
        relayConfigs[curIndex].relayManualSetting = manualRelayInput.checked;
        curRelayStates.relay_states[curIndex] = manualRelayInput.checked;
        let payload = {
            index: curIndex,
            relay_state: curRelayStates.relay_states[curIndex],
        };
        wsMod.switchRelayStates(ws, payload);
    });

    // Set the callbacks for the input elements inside each timeslot.
    function setTimeSlotsCallbacks() {
        // get the enabled, startTIme, endTime, and duration inputs for all timeSlots
        // from the DOM
        timeSlotsEnableds = document.querySelectorAll(".timeSlotEnabled");
        timeSlotsStartTimes = document.querySelectorAll(".timeSlotStartTime"); 
        timeSlotsEndTimes = document.querySelectorAll(".timeSlotEndTime");
        timeSlotsDurations = document.querySelectorAll(".timeSlotDuration");
        
        /*
        if startTime is updated, calculate for duration.
        if endTime is updated, calculate for duration.
        if duration is updated, calculate for endTime.
        */
       let events = ["change", "click"];
       for (const event of events) {
            // set the change eventListeners for enabled input of each timeSlot
            // console.log(timeSlotsEnableds.length);
            timeSlotsEnableds.forEach(element => {
                element.addEventListener(event, () => {
                    let tsIndex = element.dataset.tsIndex;
                    relayConfigs[curIndex].timeSlots[tsIndex].enabled = element.checked;
                    // console.log(JSON.stringify(config)); 
                });
            });
            // set the change eventListeners for startTime of each timeSlot
            timeSlotsStartTimes.forEach(element => {
                element.addEventListener(event, () => {
                let tsIndex = element.dataset.tsIndex;
                relayConfigs[curIndex].timeSlots[tsIndex].onStartTime = element.value + "Z";
                relayConfigs[curIndex].timeSlots[tsIndex].duration = calculateDuration(
                    relayConfigs[curIndex].timeSlots[tsIndex].onStartTime,
                    relayConfigs[curIndex].timeSlots[tsIndex].onEndTime
                );
                console.log(relayConfigs[curIndex].timeSlots[tsIndex].duration);
                element.parentElement.parentElement.getElementsByClassName("timeSlotDuration")[0]
                    .value = relayConfigs[curIndex].timeSlots[tsIndex].duration;
                // console.log(JSON.stringify(config));
                });
            });
            // set the change eventListeners for endTime of each timeSlot
            timeSlotsEndTimes.forEach(element => {
                element.addEventListener(event, () => {
                    let tsIndex = element.dataset.tsIndex;
                    relayConfigs[curIndex].timeSlots[tsIndex].onEndTime = element.value + "Z";
                    relayConfigs[curIndex].timeSlots[tsIndex].duration = calculateDuration(
                        relayConfigs[curIndex].timeSlots[tsIndex].onStartTime,
                        relayConfigs[curIndex].timeSlots[tsIndex].onEndTime
                    );
                    console.log(relayConfigs[curIndex].timeSlots[tsIndex].duration);
                    element.parentElement.parentElement.getElementsByClassName("timeSlotDuration")[0]
                        .value = relayConfigs[curIndex].timeSlots[tsIndex].duration;
                    // console.log(JSON.stringify(config)); 
                });
            });
            // set the change eventListeners for duration of each timeSlot
            timeSlotsDurations.forEach(element => {
                element.addEventListener(event, () => {
                    let tsIndex = element.dataset.tsIndex;
                    if (element.value >=0 && element.value < 24*60*60-1) {
                        relayConfigs[curIndex].timeSlots[tsIndex].duration = element.value;
                    }
                    else if (element.value < 0) {
                        relayConfigs[curIndex].timeSlots[tsIndex].duration = 0;
                    }
                    else {
                        relayConfigs[curIndex].timeSlots[tsIndex].duration = 24*60*60-1;
                    }
                    element.value = relayConfigs[curIndex].timeSlots[tsIndex].duration;
                    let endTimeString = calculateEndTime(relayConfigs[curIndex].timeSlots[tsIndex].onStartTime, relayConfigs[curIndex].timeSlots[tsIndex].duration);
                    relayConfigs[curIndex].timeSlots[tsIndex].onEndTime = endTimeString + 'Z';
                    element.parentElement.parentElement.getElementsByClassName("timeSlotEndTime")[0].value = endTimeString;              
                    // 2024-09-27T04:01:00.981Z
                    // console.log(config.timeSlots[tsIndex].duration);
                });
            });
        }
    }
    
    // change eventListener for countdown duration input
    countdownDurationInput.addEventListener('change', () => {
        relayConfigs[curIndex].countdownDurationSetting = countdownDurationInput.value;
    });

    // click eventListener for countdown start/stop button
    countdownStartStopButton.addEventListener('click', () => {
        curRelayStates.relay_states[curIndex] = !curRelayStates.relay_states[curIndex];
        let payload = {
            index: curIndex,
            relay_state: curRelayStates.relay_states[curIndex],
        };
        wsMod.switchRelayStates(ws, payload);
    });

    // click eventListener for saveRelayConfigBtn
    saveRelayConfigBtn.addEventListener("click", () => {
        // current relay index value
        console.log(`curindex=${curIndex}`);
        wsMod.saveRelayConfigs(ws, relayConfigs[curIndex]);
        console.log("save relay config");
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
    let curRelayStates = {relay_states: [false, false, false]};
    // main configuration
    let mainConfig = {
        name: "",
        ntpEnabledSetting: false, 
        gmtOffsetSetting: 0,
        ledSetting: 0,
    };
    // current relay index
    let curIndex;
    // list of relay configurations
    let relayConfigs = [
        {
            ledSetting: 0,
            operationModeSetting: 0,
            timeSlots: [
                {   
                    index: 0,
                    enabled: false, 
                    onStartTime: "07:00:00Z",
                    onEndTime: "08:00:00Z",
                },
            ],
            countdownDurationSetting: 0,
        }
    ];

    
    /*
    Websocket 
    */
    // initialize websocket
    let ws;
    function initWS() {
        let url = `ws://${hostname}:${port}/ws`;
        console.log(`url=${url}`);
        ws = new WebSocket(url);

        // request for the date and time, relay state, and configuration upon websocket open
        ws.addEventListener("open", (event) => {
            wsMod.requestDateTime(ws);
            wsMod.requestRelayState(ws);
            wsMod.requestMainConfig(ws);
            wsMod.requestRelayConfigs(ws);
        })
        // websocket message handler
        ws.addEventListener("message", (event) => {
            // define callbacks for receiving different types of data
            // callbacks[cmd][type] = function(payload)
            let callbacks = {
                "datetime": updateDisplayedTime,
                "relay_states": updateDisplayedRelayStates,
                "main_config": receiveMainConfigCallback,
                "relay_configs": receiveRelayConfigsCallback,
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
     * callback function to create relay state indicators
     */
    function createRelayStates() {
        while (relayStates.length) {
            relayStatesTable.removeChild(relayStates[0]);
        }
        
        for (let i=0;i<3;i++) {
            console.log(i);
            let newRelayState = relayStateTemplate.cloneNode(true);
            console.log(`newrelayState = ${newRelayState}`);
            newRelayState.classList.add('relayState');
            newRelayState.dataset.index = i;
            newRelayState.style.display = "block";
            let relayIndex = newRelayState.getElementsByClassName('relayIndex')[0];
            relayIndex.textContent = i;
            relayStatesTable.appendChild(newRelayState);
        }
    }

    /**
     * Callback function to update current relay state display and update the text of the start/stop countdown timer button
     */
    function updateDisplayedRelayStates(payload) {
        curRelayStates = payload;
        console.log(`new currelays=${JSON.stringify(curRelayStates)}`);
        for (let i=0;i<3;i++) {
            let relayStatusIndicator = relayStates[i].getElementsByClassName('relayStatusIndicator')[0];
            let relayStatusText = relayStates[i].getElementsByClassName("relayStatusText")[0];

            if (payload["relay_states"][i]) {
                relayStatusText.textContent = "On";
                relayStatusIndicator.classList.add("on");
                relayStatusIndicator.classList.remove("off");
                countdownStartStopButton.textContent = "Stop";
            }
            else {
                relayStatusText.textContent = "Off";
                relayStatusIndicator.classList.add("off");
                relayStatusIndicator.classList.remove("on");
                countdownStartStopButton.textContent = "Start";
            } 
        }
    }

    /**
     * Callback function to update main configuration
     */
    function receiveMainConfigCallback(payload) {
        // save payload into config
        mainConfig = payload;
        updateMainConfigDisplay();
    }

    /**
     * update main configuration display
     */
    function updateMainConfigDisplay() {
        // update name input value
        deviceNameInput.value = mainConfig["name"];
        // update NTP enable value
        ntpEnableInput.checked = mainConfig["ntpEnabledSetting"];
        // update GMT offset value 
        gmtOffsetInput.value = mainConfig["gmtOffsetSetting"];
        // set visible div based on NTP enabled value
        updateNtpEnableDivDisplay();
        mainStatusLEDModeInput.value = mainConfig["ledSetting"];
    }

    /**
     * callback function to update relay configuration
     * @param {Object} payload - object with relay configuration for one relay
     */
    function receiveRelayConfigsCallback(payload) {
        let index = payload["index"];
        console.log(`relay index = ${index}`);
        relayConfigs[index] = payload;
        updateRelayConfigsDisplay(curIndex);
    }

    /**
     * update current relay display
     * @param {Number} index - current selected relay index curIndex
     */
    function updateRelayConfigsDisplay(index) {
        // update status LED value
        ledModeInput.value = relayConfigs[index]["ledSetting"];
        // update operation mode setting
        operationModeInput.value = relayConfigs[index]["operationModeSetting"];
        // set visible div based on timer enabled value
        updateOperationModeDivDisplay();
        manualRelayInput.checked = relayConfigs[index]["relayManualSetting"];
        // refresh all timeslots
        // remove all timeslots
        while (timeSlotsInputs.length > 0) {
            timeSlotsRelayDiv.removeChild(timeSlotsInputs[0]);
        }
        // refresh all timeslots
        for (let i in relayConfigs[index]["timeSlots"]) {
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
            timeSlotIndex.textContent = i;
            timeSlotEnabled.checked = relayConfigs[index]["timeSlots"][i]["enabled"];
            // get time ISO strings of start and end times
            let startTimeString = relayConfigs[index]["timeSlots"][i]["onStartTime"].slice(0,8);
            let endTimeString = relayConfigs[index]["timeSlots"][i]["onEndTime"].slice(0,8);
            // set values for onStartTime and onEndTime
            timeSlotStartTime.value = startTimeString;
            timeSlotEndTime.value = endTimeString;
            // set duration value in timeslots
            timeSlotDuration.value = calculateDuration(startTimeString, endTimeString);            
            // display new timeslot and append it to the timeslots div
            newTimeSlot.style.display = "block";
            timeSlotsRelayDiv.appendChild(newTimeSlot);
        }
        // set callbacks for all timeslot elements
        setTimeSlotsCallbacks();
        // get countdown duration value 
        countdownDurationInput.value = relayConfigs[index]["countdownDurationSetting"];
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
     * Callback function to set the visible div depending on operationModeInput value.
     * If operationmodeinput is manual, display manual div
     * Else if operationmodeinput is daily timer, display daily timer div
     * Else if operationmodeinput is countdown, display countdown div
     * Else disable the relay
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

    // initws
    initWS();
    // update all visible divs on page load
    createRelayStates();
    getCurRelayIndex();
    updateOperationModeDivDisplay();
    updateRelayConfigsDisplay(curIndex);
    updateNtpEnableDivDisplay();
});
