document.addEventListener("DOMContentLoaded", function() {
    let element = document.querySelector('.timeSlot[data-index="1"]');
    console.log(element);
    element.getElementsByClassName('tsEnabledInput')[0].checked = true;
    console.log(element.getElementsByClassName('tsEnabledInput')[0].checked);
    
});
