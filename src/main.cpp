#include <Arduino.h>
#include "LED.h"
#include "Button.h"
#include "Relay.h"
#include "RTCNTPlib.h"
#include "EEPROMConfig.h"
#include "Webserver_module.h"

/*
feature list:
. initialize all settings for wifi and rtcntp from the eeprom config
. when timer is enabled, check all timeslots and switch the relay accordingly.
. when button is short pressed, toggle relay state manually if timer is disabled.
. when button is long pressed, reset wifi credentials and restart ESP32.
. when button is double pressed, toggle timer enabled.
. when relay is set via websockets, switch relay state.
. when status LED mode is set via websockets, switch LED mode.
. when gmt offset is set via websockets, update rtcntp websocket.
. when config is set via websockets, check all timeslots again.
*/

const int ledPin = 18;
const int buttonPin = 4;
const int relayPin = 13;

LED statusLED(ledPin);
Button button(buttonPin);
Relay relay(relayPin);

RTCNTP rtcntp;
DateTime dtnow;
EEPROMConfig eC;
WebserverModule wsMod;
unsigned long lastTimeTimeSlotsChecked;
int timeSlotCheckInterval = 1000;
bool resetWiFi = false;
bool resetWiFiBlinkLED = false;
unsigned long resetWiFiTime;

/*
function run in loop to switch on the relay when any timeslot is on and if 
timer is enabled.
*/
void checkTimeSlots() {
  if (eC.getTimerEnabled()) {
    if (millis() - lastTimeTimeSlotsChecked >= timeSlotCheckInterval) {
      lastTimeTimeSlotsChecked = millis();
      dtnow = rtcntp.getRTCTime();
      relay.set(eC.checkIfAnyTimeSlotOn(dtnow));
    }
  }
}

/*
toggle relay state when button is short pressed 
*/
void buttonToggleRelay() {
  if (!eC.getTimerEnabled()) {
    relay.set(!relay.readState());
    Serial.printf("button set relay to %d\n", relay.readState());
  }
}

void buttonToggleTimerEnable() {
  eC.setTimerEnabled(!eC.getTimerEnabled());
  eC.saveMainConfig();
  if (eC.getTimerEnabled()) {
    statusLED.startTimer(1000, true);
    statusLED.blink(100, 0.5);
  }
  else {
    statusLED.startTimer(1000, true);
    statusLED.blink(800, 0.5);
  }
  Serial.printf("button set timer enable to %d\n", eC.getTimerEnabled());
}

void buttonResetWiFi() {
  resetWiFi = true;
  resetWiFiBlinkLED = true;
  resetWiFiTime = millis();
  Serial.println("button trigger reset wifi");
}

void setup() {
  Serial.begin(115200);

  // init status LED
  statusLED.begin();

  // init button
  button.begin();
  button.setShortPressFunc(buttonToggleRelay);
  button.setLongPressFunc(buttonResetWiFi);
  button.setDoublePressFunc(buttonToggleTimerEnable);
  
  // init relay
  relay.begin();
  relay.set(LOW); // turn off relay

  // init eeprom config
  eC.begin();
  eC.load();

  // init wsmod
  // scan wifi
  wsMod.scanWiFi();
  wsMod.begin(&eC, &rtcntp);
  wsMod.setSendConnectionCallback();
  wsMod.setSendDateTimeCallback();
  wsMod.setSendRelayStateCallback();
  wsMod.setSendConfigCallback();
  wsMod.setReceiveConnectionCallback();
  wsMod.setReceiveDateTimeCallback();
  wsMod.setReceiveRelayStateCallback();
  wsMod.setReceiveConfigCallback();
  wsMod.connect();

  // init rtcntp
  rtcntp.begin();
  rtcntp.setGMTOffset(eC.getGMTOffset());
  // update RTC with NTP only if NTP is enabled
  if (eC.getNTPEnabled()) {
    rtcntp.updateRTCWithNTP();
  }
  dtnow = rtcntp.getRTCTime();

  // initialize ec timeslots
  eC.load(dtnow);
}

void loop() {
  // start concurrent loops
  statusLED.loop();
  button.loop();
  wsMod.checkWiFiStatusLoop();

  checkTimeSlots();

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
    resetWiFi["port"] = 5555;
    wsMod.receiveConnection(resetWiFi);
  }
}
