#include "EEPROMConfig.h"

/*
TimeSlot class methods
*/
// TimeSlot::TimeSlot(timeSlot& timeslot): _tS(timeslot) {
// }

TimeSlot::TimeSlot(timeSlot* timeslot, int index) {
    _tS = timeslot;
    this->setIndex(index);
}

TimeSlot::TimeSlot(timeSlot* timeslot, int index, DateTime now): TimeSlot(timeslot, index) {
    if (_tS->initialized != 0) {
        _tS->enabled = false;
        _tS->initialized=0;
    }
    if (_tS->onStartTime.hour() > 23 || _tS->onStartTime.hour() < 0) {
        this->setOnStartTime(DateTime(0,0,0,0,0,0), now);
        this->setOnEndTime(DateTime(0,0,0,0,0,0), now);
    }
    this->setOnOffFullDateTimes(now, true);
}

void TimeSlot::print() {
    Serial.printf("Timeslot\n");
    Serial.printf("initialized=%d\n", _tS->initialized);
    Serial.printf("index=%d\n", _tS->index);
    Serial.printf("enabled=%d\n", _tS->enabled);
    Serial.printf("onStartTime=%02d:%02d:%02d\n", _tS->onStartTime.hour(), _tS->onStartTime.minute(), _tS->onStartTime.second());
    Serial.printf("onEndTime=%02d:%02d:%02d\n", _tS->onEndTime.hour(), _tS->onEndTime.minute(), _tS->onEndTime.second());
    Serial.printf("durationInSeconds=%d\n", _tS->durationInSeconds);
}

int TimeSlot::getIndex() {
    return _tS->index;
}

void TimeSlot::setIndex(int index) {
    _tS->index=index;
}

bool TimeSlot::getEnabled() {
    return _tS->enabled;
}

void TimeSlot::setEnabled(bool enabled) {
    _tS->enabled=enabled;
}

DateTime TimeSlot::getOnStartTime() {
    return _tS->onStartTime;
}

String TimeSlot::getOnStartTimeISOString() {
    // 10:55:00Z
    char buff[11];
    snprintf(buff, 11, "%02u:%02u:%02uZ", _tS->onStartTime.hour(), 
        _tS->onStartTime.minute(), _tS->onStartTime.second());
    return buff;
}

void TimeSlot::setOnStartTime(int hour, int minute, int second, DateTime now) {
    this->setOnStartTime(DateTime(0,1,1,hour, minute, second), now);
}

void TimeSlot::setOnStartTime(DateTime onStartTime, DateTime now) {
    _tS->onStartTime=onStartTime;
    this->setOnOffFullDateTimes(now, true);
}

void TimeSlot::setOnStartTimeISOString(String tStr, DateTime now) {
    int tH, tMin, tS;
    sscanf(tStr.c_str(), "%02u:%02u:%02uZ", &tH, &tMin, &tS);
    this->setOnStartTime(tH, tMin, tS, now);
}

DateTime TimeSlot::getOnEndTime() {
    return _tS->onEndTime;
}

String TimeSlot::getOnEndTimeISOString() {
    // 10:55:00Z
    char buff[11];
    snprintf(buff, 11, "%02u:%02u:%02uZ", _tS->onEndTime.hour(), 
        _tS->onEndTime.minute(), _tS->onEndTime.second());
    return buff;
}

void TimeSlot::setOnEndTime(int hour, int minute, int second, DateTime now) {
    this->setOnEndTime(DateTime(0,1,1,hour, minute, second), now);
}

void TimeSlot::setOnEndTime(DateTime onEndTime, DateTime now) {
    _tS->onEndTime=onEndTime;
    // when onEndTime is changed, update durationInSeconds.
    this->setOnOffFullDateTimes(now, true);
    this->updateFromEndTimeToDuration();
}

void TimeSlot::setOnEndTimeISOString(String tStr, DateTime now) {
    int tH, tMin, tS;
    sscanf(tStr.c_str(), "%02u:%02u:%02uZ", &tH, &tMin, &tS);
    this->setOnEndTime(tH, tMin, tS, now);
}

void TimeSlot::updateFromEndTimeToDuration() {
    _tS->durationInSeconds = (_onEndFullTime - _onStartFullTime).totalseconds();
}

unsigned int TimeSlot::getDuration() {
    return _tS->durationInSeconds;
}

void TimeSlot::setDuration(unsigned int duration, DateTime now) {
    _tS->durationInSeconds=duration;
    this->setOnOffFullDateTimes(now, true);
    // when durationInSeconds is changed, update onEndTime.
    this->updateFromDurationToEndTime();    
}

void TimeSlot::updateFromDurationToEndTime() {
    _onEndFullTime = _onStartFullTime + TimeSpan(_tS->durationInSeconds);
    _tS->onEndTime = DateTime(0,1,1, _onEndFullTime.hour(), _onEndFullTime.minute(), _onEndFullTime.second());
}

/*
This method must be called upon initialization of the TimeSlot and every time this timeslot 
switches from ON to OFF. 
*/
void TimeSlot::setOnOffFullDateTimes(DateTime now, bool interrupt) {
    /*
    if interrupt is false, then the on and off datetimes will only be set
        if timeslot is off and DateTime now is greater than onEndFullTime.
    if interrupt is true, then the on and off datetimes will be reset whether or not the
        timeslot is on, which ay cause the timeslot state to suddenly toggle.
    */
    // Serial.println("set full on off date times");
    if (!interrupt) {
        //if interrupt is false then only proceed if the current state is off and 
        // the datetime now is after the onEndFullTime.
        if (_currentState || now <= _onEndFullTime)
            return;
    }
    // assign date now to the start time
    _onStartFullTime = DateTime(now.year(), now.month(), now.day(), 
        _tS->onStartTime.hour(), _tS->onStartTime.minute(), _tS->onStartTime.second());
    // assign date now to the end time 
    _onEndFullTime = DateTime(now.year(), now.month(), now.day(), 
    _tS->onEndTime.hour(), _tS->onEndTime.minute(), _tS->onEndTime.second());

    /* 
    compare the start and end times
    if the start datetime has a higher or equal time than the end datetime, 
        add one day to the end datetime.
    */
    if (_onStartFullTime >= _onEndFullTime) {
        _onEndFullTime = _onEndFullTime + TimeSpan(60*60*24);
    }

    // Serial.printf("start= %04d/%02d/%02d %02d:%02d:%02d\n", _onStartFullTime.year(), _onStartFullTime.month(),
    //     _onStartFullTime.day(), _onStartFullTime.hour(), _onStartFullTime.minute(), _onStartFullTime.second());
    // Serial.printf("end= %04d/%02d/%02d %02d:%02d:%02d\n", _onEndFullTime.year(), _onEndFullTime.month(),
    //     _onEndFullTime.day(), _onEndFullTime.hour(), _onEndFullTime.minute(), _onEndFullTime.second());
}

bool TimeSlot::checkIfOn(DateTime now) {
    this->setOnOffFullDateTimes(now);
    if (_tS->enabled) {
        if (now >= _onStartFullTime && now <= _onEndFullTime) {
            _currentState = true;
        }
        else {
            _currentState = false;
        }
        _previousState = _currentState;
        return _currentState;
    }
    else {
        return false;
    }
}



/*
EEPROMConfig class methods
*/
EEPROMConfig::EEPROMConfig(unsigned int eepromAddr) {
    _eepromAddr = eepromAddr;
}

void EEPROMConfig::print() {
    Serial.printf("EEPROMConfig\n");
    Serial.printf("ipAddrIndexSetting=%d\n", this->getIPAddressIndex());
    Serial.printf("portSetting=%d\n", this->getPort());
    Serial.printf("ssidSetting=%s\n", this->getSSID());
    Serial.printf("passwordSetting=%s\n", this->getPassword());
    Serial.println();
    Serial.printf("Device name = %s\n", this->getName());
    Serial.printf("ntpEnabledSetting=%d\n", this->getNTPEnabled());
    Serial.printf("gmtOffsetSetting=%d\n", this->getGMTOffset());
    Serial.printf("timerEnabledSetting=%d\n", this->getTimerEnabled());
    Serial.printf("ledSetting=%d\n", this->getLEDSetting());
    Serial.printf("relayManualSetting=%d\n", this->getRelayManualSetting());
    Serial.println();
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        _timeslots[i]->print();
        Serial.println();
    }
    Serial.println();
}

void EEPROMConfig::begin() {
    EEPROM.begin(sizeof(eepromConfig));
    // compute starting addresses for connection config and main config structs
    _connectionConfigAddr = _eepromAddr;
    _mainConfigAddr = _connectionConfigAddr + sizeof(connectionConfig);
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        _timeslots[i] = new TimeSlot(&_eC._mainConfig.timeSlots[i], i);
    }
}

void EEPROMConfig::load() {
    EEPROM.get(_eepromAddr, _eC);
    Serial.println("loaded _eC");
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        _timeslots[i] = new TimeSlot(&_eC._mainConfig.timeSlots[i], i);
    }
    Serial.println("Initialized TimeSlots");
}

void EEPROMConfig::load(DateTime now) {
    EEPROM.get(_eepromAddr, _eC);
    Serial.println("loaded _eC");
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        _timeslots[i] = new TimeSlot(&_eC._mainConfig.timeSlots[i], i, now);
    }
    Serial.println("Initialized TimeSlots");
}

void EEPROMConfig::save() {
    this->saveConnectionConfig();
    this->saveMainConfig();
}

void EEPROMConfig::saveConnectionConfig() {
    EEPROM.put(_connectionConfigAddr, _eC._connectionConfig);
    EEPROM.commit();
}

void EEPROMConfig::saveMainConfig() {
    EEPROM.put(_mainConfigAddr, _eC._mainConfig);
    EEPROM.commit();
}

int EEPROMConfig::getIPAddressIndex() {
    return _eC._connectionConfig.ipAddrIndexSetting;
}

void EEPROMConfig::setIPAddressIndex(int ipIndex) {
    _eC._connectionConfig.ipAddrIndexSetting=ipIndex;
}

int EEPROMConfig::getPort() {
    return _eC._connectionConfig.portSetting;
}

void EEPROMConfig::setPort(int port) {
    _eC._connectionConfig.portSetting = port;
}

String EEPROMConfig::getSSID() {
    return _eC._connectionConfig.ssidSetting;
}

void EEPROMConfig::setSSID(String ssid) {
    ssid.toCharArray(_eC._connectionConfig.ssidSetting, SSID_LENGTH);
}

String EEPROMConfig::getPassword() {
    return _eC._connectionConfig.passwordSetting;
}

void EEPROMConfig::setPassword(String password) {
    password.toCharArray(_eC._connectionConfig.passwordSetting, PASS_LENGTH);
}


String EEPROMConfig::getName() {
    return _eC._mainConfig.deviceName;
}

void EEPROMConfig::setName(String deviceName) {
    deviceName.toCharArray(_eC._mainConfig.deviceName, NAME_LENGTH);
}

bool EEPROMConfig::getNTPEnabled() {
    return _eC._mainConfig.ntpEnabledSetting;
}

void EEPROMConfig::setNTPEnabled(bool ntpEnabled) {
    _eC._mainConfig.ntpEnabledSetting = ntpEnabled;
}

short EEPROMConfig::getGMTOffset() {
    return _eC._mainConfig.gmtOffsetSetting;
}

void EEPROMConfig::setGMTOffset(short gmtOffset) {
    _eC._mainConfig.gmtOffsetSetting = gmtOffset;
}

bool EEPROMConfig::getTimerEnabled() {
    return _eC._mainConfig.timerEnabledSetting;
}

void EEPROMConfig::setTimerEnabled(bool timerEnabled) {
    _eC._mainConfig.timerEnabledSetting = timerEnabled;
}

short EEPROMConfig::getLEDSetting() {
    return _eC._mainConfig.ledSetting;
}

void EEPROMConfig::setLEDSetting(short ledSetting) {
    _eC._mainConfig.ledSetting = ledSetting;
}

bool EEPROMConfig::getRelayManualSetting() {
    return _eC._mainConfig.relayManualSetting;
}

void EEPROMConfig::setRelayManualSetting(bool relayManualSetting) {
    _eC._mainConfig.relayManualSetting = relayManualSetting;
}

TimeSlot* EEPROMConfig::getTimeSlot(int index) {
    return _timeslots[index];
}

bool EEPROMConfig::checkIfAnyTimeSlotOn(DateTime now) {
    for (int i=0;i<NUMBER_OF_TIMESLOTS;i++) {
        Serial.printf("checkIfAnyTimeSlotOn timeslot index %d returns\n", i);
        if (_timeslots[i]->checkIfOn(now)) {
            return true;
        }
    }
    return false;
}