#ifndef Button_h
#define Button_h
#include "Arduino.h" 

#define BUTTON_OPEN 0
#define BUTTON_SHORT_PRESS 1
#define BUTTON_LONG_PRESS 2 
#define BUTTON_DOUBLE_SHORT_PRESS 3
#define DELAY_DEBOUNCE 60
#define DELAY_LONG_PRESS 3000
#define DELAY_DBLPRESS 400

class Button {
    public:
        Button(int pin);
        void begin();
        void setShortPressFunc(void (*shortPressFunc)() = NULL);
        void setLongPressFunc(void (*longPressFunc)() = NULL);
        void setDoublePressFunc(void (*doubleShortPressFunc)() = NULL);
        void loop();
    private:
        int _pin;
        bool _btnState, _lastBtnState, _trigBtnState, _btnSurePressed;
        unsigned long _lastDebounceTime, _longPressTimer, _doublePressTimer;
        int _btnPressed, _timePressed, _buttonPresses;
        void (*_shortPressFunc)();
        void (*_longPressFunc)();
        void (*_doubleShortPressFunc)();
};

#endif