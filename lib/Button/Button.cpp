#include "Arduino.h"
#include "Button.h"

Button::Button(int pin) {
    _pin = pin;
}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

void Button::setShortPressFunc(void (*shortPressFunc)()) {
    _shortPressFunc = shortPressFunc;
}

void Button::setLongPressFunc(void (*longPressFunc)()) {
    _longPressFunc = longPressFunc;
}

void Button::setDoublePressFunc(void (*doubleShortPressFunc)()) {
    _doubleShortPressFunc = doubleShortPressFunc;
}

void Button::loop() {
    _btnState = digitalRead(_pin); // poll button state
    if (_btnState != _lastBtnState) { // if button state changes, start debouncing timer
                                    // and save button states.
        _lastDebounceTime = millis();
        _trigBtnState = !_btnState;
        _lastBtnState = _btnState;
    }
    
    if (millis() - _lastDebounceTime > DELAY_DEBOUNCE) { // if debounce timer is exceeded
        if (!_btnState && _trigBtnState) { // if button is still held down after the debounce 
                                        // delay, start the press timer.
            _longPressTimer = millis();
            // Serial.println("debounce timer");
            _btnSurePressed = true;
        }

        else if (_btnState && !_trigBtnState && _btnSurePressed) { // if the button is released before the long press, it is a short press.
            _btnSurePressed = false;
            // check for double press 
            if (_buttonPresses == 0) {
                _doublePressTimer = millis();
            }
            if (millis() - _doublePressTimer < DELAY_DBLPRESS) {
                _buttonPresses++;
                if (_buttonPresses >= 2) {
                    _btnPressed = BUTTON_DOUBLE_SHORT_PRESS;
                    Serial.println("button double short pressed ");
                    if (_doubleShortPressFunc != NULL) {
                        _doubleShortPressFunc(); // run short press callback
                    }
                    _buttonPresses = 0;
                    _doublePressTimer = millis();
                }
            }      
        }
        _trigBtnState = _btnState;
        _lastDebounceTime = millis();
    }

    if (millis() - _doublePressTimer >= DELAY_DBLPRESS && _buttonPresses > 0) {
        _btnPressed = BUTTON_SHORT_PRESS;
        Serial.println("button short pressed ");
        if (_shortPressFunc != NULL) {
            _shortPressFunc(); // run short press callback
        }
        _buttonPresses = 0;
        _doublePressTimer = millis();
    }

    _timePressed = millis() - DELAY_DEBOUNCE - _longPressTimer; // measure the time how long the button was held down to check if its a long press 
    if (_btnSurePressed && _timePressed > DELAY_LONG_PRESS) { // if button still held down, it is a long press.
        _btnPressed = BUTTON_LONG_PRESS;
        Serial.println("button long pressed ");
        if (_longPressFunc != NULL) {
            _longPressFunc(); // run long press callback
        }
        _btnSurePressed = false;
        _trigBtnState = _btnState;
    }
}