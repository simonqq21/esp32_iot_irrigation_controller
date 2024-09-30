#include <Arduino.h>
#include "RTCNTPlib.h"

// constructor
RTCNTP::RTCNTP() : _ntpClient(_ntpUDP, "ntp.pagasa.dost.gov.ph") {
}

// initialize RTC and NTP
void RTCNTP::begin() {
    Wire.begin();
    while (!_rtc.begin()) {
        Serial.println("no RTC.");
        delay(500);
    }
    _ntpClient.begin();
    this->refreshNTPTime();
}

// print the current time in the class
void RTCNTP::printTime() {
//    Serial.printf("%4u/%02u/%02u %02u:%02u:%02u", _dtnow.year(), _dtnow.month(), 
//         _dtnow.day(), _dtnow.hour(), 
//         _dtnow.minute(), _dtnow.second());
    Serial.println(this->getISODateTime());
}

String RTCNTP::getISODateTime() {
    // 2024-08-12T22:57:00Z
    char buffer[23];
    snprintf(buffer, 23, "%04u-%02u-%02uT%02u:%02u:%02uZ",  
        _dtnow.year(), _dtnow.month(), 
        _dtnow.day(), _dtnow.hour(), 
        _dtnow.minute(), _dtnow.second());
    return buffer;
}

void RTCNTP::setISODateTime(String dtStr) {
    int tY, tMon, tD, tH, tMin, tS;
    sscanf(dtStr.c_str(), "%04u-%02u-%02uT%02u:%02u:%02uZ", 
        &tY, &tMon, &tD, &tH, &tMin, &tS);
    this->setRTCTime(DateTime(tY, tMon, tD, tH, tMin, tS));
}

// get the time from the RTC and return it as a DateTime struct
DateTime RTCNTP::getRTCTime() {
    _dtnow = _rtc.now();
    return _dtnow; 
}

// get the NTP time and return it as a DateTime struct
DateTime RTCNTP::getNTPTime() {
    this->refreshNTPTime();
    // if NTP successfully refreshed, get time from NTP.
    if (_ntpUpdateStatus) {
        _epochTime = _ntpClient.getEpochTime();
        _dtnow = DateTime(year(_epochTime), month(_epochTime), day(_epochTime), 
            _ntpClient.getHours(), _ntpClient.getMinutes(), _ntpClient.getSeconds());
    }
    // else NTP failed to connect, get time from RTC.
    else {
        this->getRTCTime();
    }
    return _dtnow;
}

// set RTC time with a DateTime struct
void RTCNTP::setRTCTime(DateTime newDT) {
    _rtc.adjust(newDT);
    this->getRTCTime();
    // Serial.print("rtc adjusted to ");
    // Serial.println(this->getISODateTime());
}

// refresh NTP time from the internet
bool RTCNTP::refreshNTPTime() {
    _ntpClient.setTimeOffset(_gmtOffsetInHours * 3600); // UTC+8 for Philippines
    // when there is no NTP connection, update returns 0 and isTimeSet returns 0
    // when there is NTP connection, update returns 0 and isTimeSet returns 1
    _ntpClient.update();
    _ntpUpdateStatus = _ntpClient.isTimeSet();
    Serial.printf("ntpUpdateStatus=%d, isTimeSet=%d\n", _ntpUpdateStatus, _ntpClient.isTimeSet());
    /*
    ntpUpdateStatus=0, isTimeSet=0 when wifi has no access to NTP server or when it is in STA mode
        (which obviously has no connection to the NTP server.)
    ntpUpdateStatus=1, isTimeSet=1 when wifi has access to NTP server.
    */

    return _ntpUpdateStatus;
}

// update the RTC with NTP time
void RTCNTP::updateRTCWithNTP() {
    // refresh NTP time
    this->refreshNTPTime();
    if (_ntpUpdateStatus) {
        // update RTC time to NTP time
        this->setRTCTime(this->getNTPTime());
    }
    // get the current time
    this->getRTCTime();
}

// set the NTP GMT offset
void RTCNTP::setGMTOffset(int gmtOffset) {
    _gmtOffsetInHours = gmtOffset;
    _ntpClient.setTimeOffset(_gmtOffsetInHours * 3600);
}