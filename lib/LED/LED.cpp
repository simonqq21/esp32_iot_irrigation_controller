#include "Arduino.h"
#include "LED.h" 

LED::LED(int pin) {
    _pin = pin;
}

void LED::begin() {
    pinMode(_pin, OUTPUT);
}

void LED::loop() {
    if (_timerOn && millis() - _lastTimeTimerSet >= _onDuration && 
        _ledMode > LED_OFF) {
        _ledMode = LED_OFF;
        _timerOn = false;
        _ledASet = false;
        if (_resumePrevLEDMode) {
            _ledMode = _previousLEDMode;
        }
    }

    switch (_ledMode)
    {
    case LED_ON:
        _nextLEDDigitalVal = true;
        break;
    case LED_BLINK:
        if (millis() - _previousMillis > _blinkOnPeriod && _curLEDDigitalVal == true) {
            _previousMillis = millis();
            _nextLEDDigitalVal = false;

        }
        else if (millis() - _previousMillis > _blinkOffPeriod && _curLEDDigitalVal == false) {
            _previousMillis = millis();
            _nextLEDDigitalVal = true;
        }
        break;
    case LED_ANALOGSET:
        if (!_ledASet) {
            analogWrite(_pin, _curLEDAnalogVal);
            _ledASet = true;
        }
        break;
    default:
        _nextLEDDigitalVal = false;
    }

    // write values to the LED pin only upon change to avoid flickering
    if (_ledMode != LED_ANALOGSET) {
        if (_curLEDDigitalVal != _nextLEDDigitalVal) {
            _curLEDDigitalVal = _nextLEDDigitalVal;
            digitalWrite(_pin, _curLEDDigitalVal);
        }
    }
}

void LED::on() {
    pinMode(_pin, OUTPUT);
    _ledMode = LED_ON;
}

void LED::off() {
    pinMode(_pin, OUTPUT);
    _ledMode = LED_OFF;
}

void LED::toggle() {
    switch (_ledMode)
    {
    case LED_OFF:
        this->on();
        break;
    
    default:
        this->off();
        break;
    }
}

void LED::set(bool state) {
    if (state)
        this->on();
    else 
        this->off();
}

void LED::blink(unsigned int period, double dutyCycle) {
    pinMode(_pin, OUTPUT);
    _ledMode = LED_BLINK;
    if (dutyCycle > 1.0) {
        dutyCycle = 1.0;
    }
    else if (dutyCycle < 0.0) {
        dutyCycle = 0.0;
    }
    _blinkOnPeriod = period * dutyCycle;
    _blinkOffPeriod = period * (1.0-dutyCycle);
}

void LED::aSet(int aValue) {
    _ledMode = LED_ANALOGSET;
    _curLEDAnalogVal = aValue;
    _ledASet = false;
}

void LED::startTimer(int milliseconds, bool resumePreviousMode) {
    _previousLEDMode = _ledMode;
    _resumePrevLEDMode = resumePreviousMode;

    if (milliseconds > 0) {
        _timerOn = true;
        _onDuration = milliseconds;
        _lastTimeTimerSet = millis();
    }
    else {
        _timerOn = false;
    }
}
