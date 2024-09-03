#ifndef LED_h
#define LED_h
#include "Arduino.h" 

#define LED_OFF 0
#define LED_ON 1 
#define LED_BLINK 2 
#define LED_ANALOGSET 3

class LED {
    public:
        LED(int pin);
        void begin(); 
        void loop();
        void on();
        void off();
        void toggle();
        void set(bool state);
        void blink(unsigned int period, double dutyCycle);
        void aSet(int aValue);
        void startTimer(int seconds, bool persistent=false);
    private:
        int _pin;
        int _ledMode;
        int _previousLEDMode;
        bool _resumePrevLEDMode;
        bool _curLEDDigitalVal, _nextLEDDigitalVal;
        int _curLEDAnalogVal;
        bool _ledASet;
        unsigned long _previousMillis;
        unsigned int _blinkOnPeriod;
        unsigned int _blinkOffPeriod;
        bool _timerOn;
        unsigned long _lastTimeTimerSet;
        unsigned long _onDuration;
};

#endif