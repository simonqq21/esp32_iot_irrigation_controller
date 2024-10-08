#include <Arduino.h>
#include "LED.h"
#include "Button.h"
#include "Relay.h"
#include "RTCNTPlib.h"
#include "EEPROMConfig.h"
#include "Webserver_module.h"

/*
TODO:
add a third mode: timer mode.
  The relay is manually closed, and stays closed until either manually opened 
  or the set timer runs out. 
make the UI more user friendly, such as greying out the save button until a 
  setting has been changed.
Add a one click button to turn the relay on and off in one click.
*/

/*
hardware pins
*/ 
const int ledPin = 18;
const int buttonPin = 4;
const int relayPin = 13;
/*
hardware components
*/
LED statusLED(ledPin);
Button button(buttonPin);
Relay relay(relayPin);
/*
Major modules
*/
RTCNTP rtcntp;
DateTime dtnow;
EEPROMConfig eC;
WebserverModule wsMod;
/*
misc variables
*/
// check timeslots once a second
unsigned long lastTimeTimeSlotsChecked;
int timeSlotCheckInterval = 1000;
// for resetting wifi credentials by long pressing the button
bool resetWiFi = false;
bool resetWiFiBlinkLED = false;
unsigned long resetWiFiTime;
// send the time once a second
unsigned long sendTimeInterval = 1000;
unsigned long lastTimeTimeSent;
/*
If NTP update is called at the same time as the websocket sending data to the ESP32 when there is
no access to the NTP server, such as when the configuration is saved with NTP enabled through a 
WiFi connection that isn't connected to the internet, it will cause the watchdog to reset the ESP32. 
Instead, there is a delay of around 3 seconds so that the 8-second delay caused by the NTP client 
trying to connect to the NTP service simultaneously with the websocket code will not cause the 
watchdog to reset.
*/
bool updateNTPFlag = false;
unsigned long lastTimeUpdateTime;
// relay state
bool newRelayState;

/*
function in the loop to control the relay.
*/
void checkRelayIfOn() {
  switch (eC.getOperationMode()) {
    // manual mode
    case 1:
      newRelayState = eC.getRelayManualSetting();
      // Serial.printf("newRelayState=%d\n", newRelayState);
      break;
    // if automatic timer is enabled, check all timeslots if any one of them is on
    // in the current time.
    case 2:
      if (millis() - lastTimeTimeSlotsChecked >= timeSlotCheckInterval) {
        lastTimeTimeSlotsChecked = millis();
        dtnow = rtcntp.getRTCTime();
        newRelayState = eC.checkIfAnyTimeSlotOn(dtnow);
      }
      break;
    // countdown timer mode
    case 3:
      if (millis() - lastTimeTimeSlotsChecked >= 50) {
        lastTimeTimeSlotsChecked = millis();
        newRelayState = eC.checkCountdownTimer(100);
      }
      break;
    default:
      newRelayState = false;
    
  }
  // only update the relay if the state has changed, minimizing flickering.
  if (newRelayState != relay.readState()) {
    // Serial.printf("before set %d, %d",newRelayState, relay.readState());
    relay.set(newRelayState);
    // Serial.printf("after set %d, %d",newRelayState, relay.readState());
    // update relay state to client
    wsMod.sendCurrentRelayState(newRelayState);
  }
}

// function in loop to set the status LED mode.
void setStatusLED() {
  switch (eC.getLEDSetting())
  {
  case LED_ON:
    statusLED.on();
    break;
  case LED_BLINK:
    statusLED.blink(3000, 0.08);
    break;
  default:
    statusLED.off();
    break;
  }
}

/*
callback function to toggle relay state when button is short pressed 
*/
void buttonToggleRelay() {
  switch (eC.getOperationMode()) {
    case 1:
      eC.setRelayManualSetting(!relay.readState());
      Serial.printf("button set relay to %d\n", eC.getRelayManualSetting());
      break;
    case 3:
      if (eC.checkCountdownTimer()) {
        eC.stopCountdownTimer();
      }
      else {
        eC.startCountdownTimer();
      }
      break;
    default:
      break;
  }
}

/*
callback function to toggle the automatic timer when button is double pressed
*/
void buttonToggleOperationMode() {
  if (eC.getOperationMode() < 3) {
    eC.setOperationMode(eC.getOperationMode()+1);
  } else {
    eC.setOperationMode(0);
  }
  eC.saveMainConfig();
  statusLED.startTimer(1000*eC.getOperationMode(), true);
  statusLED.blink(1000, 0.5);
  Serial.printf("button set operation mode to %d\n", eC.getOperationMode());
}

/*
callback function to write default wifi credential values to the EEPROM when 
the button is long pressed.
*/
void buttonResetWiFi() {
  resetWiFi = true;
  resetWiFiBlinkLED = true;
  resetWiFiTime = millis();
  Serial.println("button trigger reset wifi");
}

/*
Loop function to reset the wifi credential values when requested.
*/
void checkToResetWiFi() {
  if (resetWiFiBlinkLED) {
    statusLED.startTimer(2000, true);
    statusLED.blink(800, 0.5);
    resetWiFiBlinkLED = false;
  }

  if (resetWiFi && millis() - resetWiFiTime > 2100) {
    Serial.println("resetting wifi...");
    JsonDocument resetWiFi;
    resetWiFi["ssid"] = "default-SSID";
    resetWiFi["pass"] = "password";
    resetWiFi["ipIndex"] = 2;
    resetWiFi["port"] = 7777;
    wsMod.receiveConnection(resetWiFi);
  }
}

/*
callback function to update NTP time when it is enabled.
*/
void updateTime() {
  Serial.print("updateTIme: ");
  Serial.println(eC.getNTPEnabled());
  if (eC.getNTPEnabled()) {
    updateNTPFlag = true;
    lastTimeUpdateTime = millis();
    Serial.println("updateTime");
  }
}

/*
function in loop to update NTP time 3 seconds after the websocket has completed
its current operation. Read the comments at the top of the file. */
void updateTimeInLoop() {
  if (updateNTPFlag && millis() - lastTimeUpdateTime > 3000) {
    updateNTPFlag = false;
    rtcntp.setGMTOffset(eC.getGMTOffset());
    rtcntp.updateRTCWithNTP();
    dtnow = rtcntp.getRTCTime();
    Serial.println("updateTimeInLoop");
  }
}

/*
function in loop to send time once a second to the client.
*/
void sendTime() {
  if (millis() - lastTimeTimeSent > sendTimeInterval) {
    lastTimeTimeSent = millis();
    wsMod.sendDateTime();
    rtcntp.printTime();
  }
}

void setup() {
  Serial.begin(115200);
  // littleFS 
  if (!LittleFS.begin()) {
    Serial.println("An error occured while mounting LittleFS.");
  }

  // init status LED
  statusLED.begin();

  // init button
  button.begin();
  button.setShortPressFunc(buttonToggleRelay);
  button.setLongPressFunc(buttonResetWiFi);
  button.setDoublePressFunc(buttonToggleOperationMode);
  
  // init relay
  relay.begin();
  relay.set(LOW); // turn off relay

  // init eeprom config
  eC.begin();
  eC.load();

  // init wsmod
  // scan wifi
  wsMod.scanWiFi();
  wsMod.begin(&eC, &rtcntp, &relay);
  // set websocket callback functions
  wsMod.setSendConnectionCallback();
  wsMod.setSendDateTimeCallback();
  wsMod.setSendRelayStateCallback();
  wsMod.setSendConfigCallback();
  wsMod.setReceiveConnectionCallback();
  wsMod.setReceiveDateTimeCallback(updateTime);
  wsMod.setReceiveRelayStateCallback();
  wsMod.setReceiveConfigCallback(updateTime);
  wsMod.connect();

  // init rtcntp
  rtcntp.begin();
  // update RTC with NTP only if NTP is enabled
  updateTime();
  dtnow = rtcntp.getRTCTime();

  // initialize ec timeslots
  eC.load(dtnow);

  eC.print();
  rtcntp.printTime();
}

void loop() {
  // start concurrent loops
  statusLED.loop();
  button.loop();
  wsMod.cleanupClients();
  wsMod.checkWiFiStatusLoop();

  checkRelayIfOn();
  setStatusLED();
  checkToResetWiFi();
  sendTime();

  updateTimeInLoop();
}
