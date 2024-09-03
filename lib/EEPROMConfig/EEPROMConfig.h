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

/*
07:00-08:00
08:00-08:00
08:00-07:00
23:00-01:00
23:00-00:00

// same day
if (end > start) {
    end and start are combined with the date today
}
// next day
else {
    start is combined with the date today while end is combined with the date tomorrow
}
if now > start && now < end

*/
struct timeSlot {
    unsigned short index;
    short initialized;
    bool enabled;
    DateTime onStartTime, onEndTime; // nonvolatile time only values for on start and on end
    unsigned int durationInSeconds;
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
    bool timerEnabledSetting;
    short ledSetting;
    bool relayManualSetting;
    timeSlot timeSlots[NUMBER_OF_TIMESLOTS];
};

struct eepromConfig {
    connectionConfig _connectionConfig;
    mainConfig _mainConfig;
};

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
        bool checkIfOn(DateTime now);
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
        int getIPAddressIndex();
        void setIPAddressIndex(int ipIndex=2);
        int getPort();
        void setPort(int port);
        String getSSID();
        void setSSID(String ssid);
        String getPassword();
        void setPassword(String password);

        String getName();
        void setName(String deviceName);
        bool getNTPEnabled();
        void setNTPEnabled(bool ntpEnabled);
        short getGMTOffset();
        void setGMTOffset(short gmtOffset);
        bool getTimerEnabled();
        void setTimerEnabled(bool timerEnabled);
        short getLEDSetting();
        void setLEDSetting(short ledSetting);
        bool getRelayManualSetting();
        void setRelayManualSetting(bool relayManualSetting);
        TimeSlot* getTimeSlot(int index);
        bool checkIfAnyTimeSlotOn(DateTime now);

    private:
        unsigned int _eepromAddr, _connectionConfigAddr, _mainConfigAddr;
        eepromConfig _eC;
        TimeSlot* _timeslots[NUMBER_OF_TIMESLOTS];
};

#endif