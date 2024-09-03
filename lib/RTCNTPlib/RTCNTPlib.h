#ifndef RTCNTPLIB_H
#define RTCNTPLIB_H
#include <Arduino.h>
#include <RTClib.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>

// This library combines a DS1307 RTC with NTP time. NTP time can be used to update the RTC 
// periodically, while the RTC can store the time when NTP connection is not available.
class RTCNTP {
    public:
        RTCNTP();
        void begin();
        void printTime();
        String getISODateTime();
        void setISODateTime(String dtStr);
        DateTime getRTCTime();
        DateTime getNTPTime();
        void setRTCTime(DateTime newDT);
        bool refreshNTPTime();
        void updateRTCWithNTP();
        void setGMTOffset(int gmtOffset);
    private: 
        RTC_DS1307 _rtc;
        bool _ntpEnabled;
        bool _ntpUpdateStatus;
        DateTime _dtnow;
        long _UTCOffsetInSeconds;
        WiFiUDP _ntpUDP;
        NTPClient _ntpClient;
        int _gmtOffsetInHours;
        unsigned long _epochTime;
};

#endif