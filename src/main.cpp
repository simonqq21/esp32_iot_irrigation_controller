#include <Arduino.h>
#include "LED.h"
#include "Button.h"
#include "Relay.h"
#include "RTCNTPlib.h"
#include "EEPROMConfig.h"
#include "Webserver_module.h"

#include "esp_websocket_client.h"

/*
TODO for v3:
add method to LED to play a sequence of bools over a certain span of time in a cycle
Use that method to blink out the LED state on the status LEDs
make the UI more user friendly, such as greying out the save button until a 
setting has been changed.
*/

/*
hardware pins
*/ 
const int wifiResetBtnPin = 27;
const int statusLED1Pin = LED_BUILTIN;
const int ledPins[NUMBER_OF_RELAYS] = {18,19,5};
const int buttonPins[NUMBER_OF_RELAYS] = {4,23,15};
const int relayPins[NUMBER_OF_RELAYS] = {13,12,14};
/*
hardware components
*/
LED mainStatusLED(statusLED1Pin);
Button wifiResetBtn(wifiResetBtnPin);
LED statusLEDs[NUMBER_OF_RELAYS] = {LED(ledPins[0]), LED(ledPins[1]), LED(ledPins[2])};
Button buttons[NUMBER_OF_RELAYS] = {Button(buttonPins[0]), Button(buttonPins[1]), Button(buttonPins[2])};
Relay relays[NUMBER_OF_RELAYS] = {Relay(relayPins[0]), Relay(relayPins[1]), Relay(relayPins[2])};

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
unsigned long lastTimesTimeSlotsChecked[NUMBER_OF_RELAYS];
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
bool newRelayStates[NUMBER_OF_RELAYS];

/*
function in the loop to control the relay.
*/
void checkRelaysIfOn() {
  for (int i=0;i<NUMBER_OF_RELAYS;i++) {
    switch (eC.getOperationMode(i)) {
      // manual mode
      case RELAY_MANUAL:
        newRelayStates[i] = eC.getRelayManualSetting(i);
        // Serial.printf("newRelayState=%d\n", newRelayState);
        break;
      // if automatic timer is enabled, check all timeslots if any one of them is on
      // in the current time.
      case RELAY_TIMESLOTS:
        if (millis() - lastTimesTimeSlotsChecked[i] >= 500) {
          lastTimesTimeSlotsChecked[i] = millis();
          dtnow = rtcntp.getRTCTime();
          newRelayStates[i] = eC.checkIfAnyTimeSlotOn(i, dtnow);
        }
        break;
      // countdown timer mode
      case RELAY_COUNTDOWN:
        if (millis() - lastTimesTimeSlotsChecked[i] >= 50) {
          lastTimesTimeSlotsChecked[i] = millis();
          newRelayStates[i] = eC.checkCountdownTimer(i);
        }
        break;
      default:
        newRelayStates[i] = false;
    }
    // only update the relay if the state has changed, minimizing flickering.
    if (newRelayStates[i] != relays[i].readState()) {
      // Serial.printf("before set %d, %d",newRelayState, relay.readState());
      relays[i].set(newRelayStates[i]);
      // Serial.printf("after set %d, %d",newRelayState, relay.readState());
      // update relay state to client
      wsMod.sendCurrentRelayStates(newRelayStates);
    } 
  }
}

/**
 * Flash n times on statusLED(ledIndex)
 */
void flashOpModeOnLED(LED led, int numFlashes) {
  led.setLoopUnitDuration(100);
  bool loopseq[MAX_SEQUENCE_LENGTH];
  int i=0;
  for (int j=0;j<3;j++) {
    if (j<numFlashes) loopseq[i] = 1;
    else loopseq[i] = 0; 
    i++;
    for (int k=0;k<3;k++) {
      loopseq[i] = 0;
      i++;
    }
  }
  for (int j=0;j<38;j++) { 
    loopseq[i] = 0;
    i++;
  }
  led.setLoopSequence(loopseq, i);
  led.startLoop();
}

// function in loop to set the status LED mode.
void setStatusLEDs() {
  switch (eC.getMainLEDSetting()) {
    case LED_ON:
      mainStatusLED.on();
      break;
    case LED_BLINK:
      mainStatusLED.blink(3000, 0.08);
      break;
    default:
      mainStatusLED.off();
      break;
  }
  for (int i=0;i<NUMBER_OF_RELAYS;i++) {
    switch (eC.getLEDSetting(i))
    {
    case LED_ON:
      statusLEDs[i].on();
      break;
    case LED_BLINK:
      if (statusLEDs[i].getMode() != LED_LOOP)
        flashOpModeOnLED(statusLEDs[i], eC.getOperationMode(i));
      // switch (eC.getOperationMode(i)) {
      //   case (RELAY_MANUAL):
          
      //     break;
      //   case (RELAY_TIMESLOTS): 

      //     break;
      //   case (RELAY_COUNTDOWN): 

      //     break;
      //   default: 

      // }
      // statusLEDs[i].blink(3000, 0.08);
      break;
    default:
      statusLEDs[i].off();
      break;
    }
  }
}

/*
callback function to toggle relay state when button is short pressed 
*/
void buttonToggleRelay(int index) {
  bool newState;
  switch (eC.getOperationMode(index)) {
    case 1:
      if (relays[index].readState()) {
        newState = false;
      }
      else {
        newState = true;
      }
      Serial.printf("newstate=%d\n", newState);
      eC.setRelayManualSetting(index, newState);
      Serial.printf("button set relay to %d\n", eC.getRelayManualSetting(index));
      break;
    case 3:
      if (eC.checkCountdownTimer(index)) {
        eC.stopCountdownTimer(index);
      }
      else {
        eC.startCountdownTimer(index);
      }
      break;
    default:
      break;
  }
  eC.saveRelayConfig(index);
}

void buttonToggleMainStatusLED(int i) {
  if (eC.getMainLEDSetting() < 2) {
    eC.setMainLEDSetting(eC.getMainLEDSetting() + 1);
  } else {
    eC.setMainLEDSetting(0);
  }
  eC.saveMainConfig();
  Serial.printf("button set main status LED to %d\n", eC.getMainLEDSetting());
}

/*
callback function to toggle the automatic timer when button is double pressed
*/
void buttonToggleOperationMode(int index) {
  if (eC.getOperationMode(index) < 3) {
    eC.setOperationMode(index, eC.getOperationMode(index)+1);
  } else {
    eC.setOperationMode(index, 0);
  }
  eC.save();
  // Serial.printf("button set operation mode to %d\n", eC.getOperationMode(index));
  statusLEDs[index].startTimer(1000*eC.getOperationMode(index), true);
  statusLEDs[index].blink(1000, 0.5);
  // eC.load();
  Serial.printf("button set operation mode to %d\n", eC.getOperationMode(index));
}

/*
callback function to write default wifi credential values to the EEPROM when 
the button is long pressed.
*/
void buttonResetWiFi(int i) {
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
    mainStatusLED.startTimer(2000, true);
    mainStatusLED.blink(800, 0.5);
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

  mainStatusLED.begin();
  wifiResetBtn.begin();
  wifiResetBtn.setIndex(4);
  wifiResetBtn.setShortPressFunc(buttonToggleMainStatusLED);
  wifiResetBtn.setLongPressFunc(buttonResetWiFi);
  for (int i=0;i<NUMBER_OF_RELAYS;i++) {
    // init status LED
    statusLEDs[i].begin();
    // init button
    buttons[i].setIndex(i);
    buttons[i].begin();
    buttons[i].setShortPressFunc(buttonToggleRelay);
    buttons[i].setDoublePressFunc(buttonToggleOperationMode);
    // button.setLongPressFunc();
    // init relay
    relays[i].begin();
    relays[i].set(LOW); // turn off relay
  }

  // init eeprom config
  eC.begin();
  eC.load();

  // init wsmod
  // scan wifi
  wsMod.scanWiFi();
  wsMod.begin(&eC, &rtcntp, relays);
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
  mainStatusLED.loop();
  wifiResetBtn.loop();
  // start concurrent loops
  for (int i=0;i<NUMBER_OF_RELAYS;i++) {
    statusLEDs[i].loop();
  }
  for (int i=0;i<NUMBER_OF_RELAYS;i++) {
    buttons[i].loop();
  }
  wsMod.cleanupClients();
  wsMod.checkWiFiStatusLoop();

  checkRelaysIfOn();
  setStatusLEDs();
  checkToResetWiFi();
  sendTime();

  updateTimeInLoop();
}
