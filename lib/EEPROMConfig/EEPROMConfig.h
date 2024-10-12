#ifndef EEPROMCONFIG_H
#define EEPROMCONFIG_H
#include "Arduino.h"
#include "EEPROM.h"
#include <RTClib.h>
#include <time.h>
#include <SPI.h>

#define NUMBER_OF_TIMESLOTS 8
#define SSID_LENGTH 32
#define PASS_LENGTH 32 
#define NAME_LENGTH 32 
#define NUMBER_OF_RELAYS 3
const int MAGIC_NUMBER = 0x88;
// structs

struct timeSlot {
    unsigned short index;
    short initialized;
    bool enabled;
    DateTime onStartTime, onEndTime; // nonvolatile time only values for on start and on end
    unsigned int durationInSeconds;
};

struct countdownTimer {
    long timeRemaining;
    unsigned long lastTimeChecked;
    bool pause;
};

struct connectionConfig {
    int ipAddrIndexSetting;
    int portSetting;
    char ssidSetting[SSID_LENGTH];
    char passwordSetting[PASS_LENGTH];
};

struct mainConfig {
    char deviceName[NAME_LENGTH];
    bool ntpEnabledSetting;
    short gmtOffsetSetting;
};

struct relayConfig {
    int index;
    short ledSetting;
    int operationModeSetting; // 0 for disabled, 1 for manual, 2 for daily timer, and 3 for countdown timer
    bool relayManualSetting;
    timeSlot timeSlots[NUMBER_OF_TIMESLOTS];
    long countdownDurationSetting;
};

struct eepromConfig {
    connectionConfig _connectionConfig;
    mainConfig _mainConfig;
    relayConfig _relayConfigs[NUMBER_OF_RELAYS];
    int magicNumber;
};

// classes

class TimeSlot {
    public:
        TimeSlot(timeSlot* timeslot, int index);
        TimeSlot(timeSlot *timeslot, int index,  DateTime now);
        void print();
        // TimeSlot(timeSlot &timeslot);
        // void setTimeSlot(timeSlot& timeslot);
        int getIndex();
        void setIndex(int index);
        bool getEnabled();
        void setEnabled(bool enabled);
        DateTime getOnStartTime();
        String getOnStartTimeISOString();
        void setOnStartTime(DateTime onStartTime, DateTime now);
        void setOnStartTime(int hour, int minute, int second, DateTime now);
        void setOnStartTimeISOString(String tStr, DateTime now);
        DateTime getOnEndTime();
        String getOnEndTimeISOString();
        void setOnEndTime(int hour, int minute, int second, DateTime now);
        void setOnEndTime(DateTime onEndTime, DateTime now);
        void setOnEndTimeISOString(String tStr, DateTime now);
        void updateFromEndTimeToDuration();
        unsigned int getDuration();
        void setDuration(unsigned int duration, DateTime now);
        void updateFromDurationToEndTime();
        void setOnOffFullDateTimes(DateTime now, bool interrupt=false);
        bool checkIfOn(DateTime now, bool interrupt=false);
    private:
        timeSlot* _tS;
        // variables stored in memory, not in EEPROM
        DateTime _onStartFullTime, _onEndFullTime; // volatile time with date values for on start and on end
        bool _previousState, _currentState;
        bool initialized = false;
};

class EEPROMConfig {
    public:
        EEPROMConfig(unsigned int eepromAddr = 0);
        void print();
        void begin();
        void load();
        void load(DateTime now);
        void save();
        void saveConnectionConfig();
        void saveMainConfig();
        void saveRelayConfig(int rIndex);

        // connection config methods
        int getIPAddressIndex();
        void setIPAddressIndex(int ipIndex=2);
        int getPort();
        void setPort(int port);
        String getSSID();
        void setSSID(String ssid);
        String getPassword();
        void setPassword(String password);

        // main config methods
        String getName();
        void setName(String deviceName);
        bool getNTPEnabled();
        void setNTPEnabled(bool ntpEnabled);
        short getGMTOffset();
        void setGMTOffset(short gmtOffset);
        
        // relay config methods
        short getLEDSetting(int rIndex);
        void setLEDSetting(int rIndex, short ledSetting);
        int getOperationMode(int rIndex);
        void setOperationMode(int rIndex, int operationMode);
        bool getRelayManualSetting(int rIndex);
        void setRelayManualSetting(int rIndex, bool relayManualSetting);
        TimeSlot* getTimeSlot(int rIndex, int tsIndex);
        bool checkIfAnyTimeSlotOn(int rIndex, DateTime now, bool interrupt=false);
        unsigned long getCountdownDuration(int rIndex);
        void setCountdownDuration(int rIndex, unsigned long countdownDuration);
        void startCountdownTimer(int rIndex);
        void stopCountdownTimer(int rIndex);
        bool checkCountdownTimer(int rIndex, unsigned long min_ms = 100);
        void pauseCountdownTimer(int rIndex);
        void unpauseCountdownTimer(int rIndex);

    private:
        unsigned int _eepromAddr, _connectionConfigAddr, _mainConfigAddr, _relayConfigAddrs[NUMBER_OF_RELAYS], _magicNumberAddr;
        eepromConfig _eC;
        TimeSlot* _timeslots[NUMBER_OF_RELAYS][NUMBER_OF_TIMESLOTS];
        countdownTimer countdownTimerVars[NUMBER_OF_RELAYS];
};

#endif