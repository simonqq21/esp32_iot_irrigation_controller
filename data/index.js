document.addEventListener("DOMContentLoaded", function() {
    let port = 5555;
    // let hostname = window.location.hostname;
    let hostname = "192.168.5.71";
    // let hostname = "192.168.4.1";
    let url = `ws://${hostname}:${port}/ws`;
    console.log(`hostname=${hostname}`);
    let ws = new WebSocket(url);

    // upon page
    let 

    let element = document.querySelector('.timeSlot[data-index="1"]');
    console.log(element);
    element.getElementsByClassName('tsEnabledInput')[0].checked = true;
    console.log(element.getElementsByClassName('tsEnabledInput')[0].checked);
    
});
