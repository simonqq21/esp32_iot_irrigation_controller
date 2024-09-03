#include "Arduino.h"
#include "Relay.h"

Relay::Relay(int pin) {
    _pin = pin;
}

void Relay::begin() {
    pinMode(_pin, OUTPUT);
}

void Relay::set(bool state) {
    _curRelayState = state;
    digitalWrite(_pin, _curRelayState);
}

bool Relay::readState() {
    return _curRelayState;
}