import * as wsMod from './wsMod.js';

document.addEventListener("DOMContentLoaded", function() {
    let port = 5555;
    // let hostname = window.location.hostname;
    let hostname = "192.168.5.71";
    // let hostname = "192.168.4.1";
    let url = `ws://${hostname}:${port}/ws`;
    console.log(`hostname=${hostname}`);
    let ws = new WebSocket(url);
    /*
    TODO:
    .load all configuration from ESP32 on startup 
    .update system date and time display
    .load all timeslots
    .create variables to hold the local configuration
    */
    
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
            {   
                index: 1,
                enabled: false, 
                onStartTime: new Date(2024, 1, 1, 0, 1, 0),
                onEndTime: new Date(2024, 1, 1, 0, 2, 0),
            },
            {   
                index: 2,
                enabled: false, 
                onStartTime: new Date(2024, 1, 1, 0, 2, 0),
                onEndTime: new Date(2024, 1, 1, 0, 4, 0),
            }
        ]
    }
    /**
     * 
     */
     

    let element = document.querySelector('.timeSlot[data-index="1"]');
    console.log(element);
    element.getElementsByClassName('tsEnabledInput')[0].checked = true;
    console.log(element.getElementsByClassName('tsEnabledInput')[0].checked);
    
});
