#ifndef Relay_h
#define Relay_h 
#include "Arduino.h"

class Relay {
    public:
        Relay(int pin);
        void begin();
        void set(bool state);
        bool readState();
    private:
        int _pin;
        bool _curRelayState;
};

#endif