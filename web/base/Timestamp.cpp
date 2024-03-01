//
// Created by 37496 on 2024/1/31.
//

#include <sys/time.h>
#include "TimeStamp.h"

namespace Tiny_muduo
{

    TimeStamp::TimeStamp(int64_t micro_seconds) : _microSecondsSinceEpoch(micro_seconds){

    }

    TimeStamp& TimeStamp::operator+(const TimeStamp& timeStamp) {
        _microSecondsSinceEpoch += timeStamp._microSecondsSinceEpoch;
        return *this;
    }

    TimeStamp& TimeStamp::operator-(const TimeStamp& timeStamp) {
        _microSecondsSinceEpoch -= timeStamp._microSecondsSinceEpoch;
        return *this;
    }

    TimeStamp& TimeStamp::operator+(int microsecond) {
        _microSecondsSinceEpoch += microsecond;
        return *this;
    }

    TimeStamp& TimeStamp::operator-(int microsecond) {
        _microSecondsSinceEpoch -= microsecond;
        return *this;
    }

    bool TimeStamp::operator<(const Tiny_muduo::TimeStamp &rhs) const {
        return _microSecondsSinceEpoch < rhs._microSecondsSinceEpoch;
    }
    bool TimeStamp::operator<=(const Tiny_muduo::TimeStamp &rhs) const {
        return _microSecondsSinceEpoch <= rhs._microSecondsSinceEpoch;
    }

    bool TimeStamp::operator==(const Tiny_muduo::TimeStamp &rhs) const {
        return _microSecondsSinceEpoch == rhs._microSecondsSinceEpoch;
    }

    bool TimeStamp::operator>(const Tiny_muduo::TimeStamp &rhs) const {
        return _microSecondsSinceEpoch > rhs._microSecondsSinceEpoch;
    }
    bool TimeStamp::operator>=(const Tiny_muduo::TimeStamp &rhs) const {
        return _microSecondsSinceEpoch >= rhs._microSecondsSinceEpoch;
    }

    std::string TimeStamp::toFormatString(bool showMicroseconds) const {
        char buf[64]={0};
        time_t seconds = static_cast<time_t>(_microSecondsSinceEpoch / kMicroSecondsPerSecond);
        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);

        if(showMicroseconds) {
            int microseconds = static_cast<int>(_microSecondsSinceEpoch % kMicroSecondsPerSecond);
            snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                     microseconds);
        }
        else {
            snprintf(buf, sizeof(buf), "%4d%02d%02d_%02d%02d%02d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        }
        return buf;
    }

    TimeStamp TimeStamp::now() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
    }
}