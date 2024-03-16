//
// Created by 37496 on 2024/1/31.
//

#ifndef WEBSERVER_LOG_TIMESTAMP_H
#define WEBSERVER_LOG_TIMESTAMP_H

#include <cstdint>
#include <string>
#include <limits>

namespace HLog
{
    /*
     * Timestamp类记录的就是从1970年1月1号00:00开始到现在的毫秒数，毫秒是int64_t类型，
     * 这个毫秒数就是通过调用gettimeofday获得的，
     * 所以，Timestamp类可以简单看作就是一个int64_t，只不过多了一些成员方法。
     * */
    class TimeStamp
    {
    public:
        explicit TimeStamp(int64_t micro_seconds = 0);
        ~TimeStamp() = default;

        void swap(TimeStamp& that)
        {
            std::swap(_microSecondsSinceEpoch, that._microSecondsSinceEpoch);
        }


        int64_t second() const noexcept { return _microSecondsSinceEpoch / kMicroSecondsPerSecond; }
        int64_t millSecond() const noexcept { return _microSecondsSinceEpoch / kMicroSecondsPerMillSecond; }
        int64_t microSecond() const noexcept { return _microSecondsSinceEpoch; }
        int64_t nanosecond() const noexcept { return _microSecondsSinceEpoch * 1000; }

        TimeStamp& operator+(const TimeStamp& timeStamp) ;
        TimeStamp& operator-(const TimeStamp& timeStamp) ;
        TimeStamp& operator+(int second);
        TimeStamp& operator-(int second);
        bool operator<(const TimeStamp& rhs) const;
        bool operator<=(const TimeStamp& rhs) const;
        bool operator==(const TimeStamp& rhs) const;
        bool operator>(const TimeStamp& rhs) const;
        bool operator>=(const TimeStamp& rhs) const;

        static TimeStamp second(int sec) { return TimeStamp(sec * 1000000); }
        static TimeStamp millsecond(int msec) { return TimeStamp(msec * 1000); }
        static TimeStamp microsecond(int microsec) { return TimeStamp(microsec); }
        static TimeStamp nanosecond(int nsec) { return TimeStamp(nsec / 1000); }
        static TimeStamp max() noexcept { return TimeStamp(std::numeric_limits<int64_t>::max()); }
        static TimeStamp nowSecond(int sec) { return TimeStamp::now() + second(sec); }
        static TimeStamp nowMsecond(int msec) { return TimeStamp::now() + millsecond(msec); }
        static TimeStamp nowNanosecond(int nsec) { return TimeStamp::now() + nanosecond(nsec); }

        //格式, "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
        std::string toFormatString(bool showMicroseconds = true) const;

        bool valid() const { return _microSecondsSinceEpoch > 0; }  //判断当前时间戳是否有效

        static TimeStamp invalid() { return TimeStamp(); }// 失效的时间戳，返回一个值为0的Timestamp
        static TimeStamp now();        //返回当前时间戳

        int64_t microSecondsSinceEpoch() const { return _microSecondsSinceEpoch; }          //返回当前时间戳的微妙
        time_t secondsSinceEpoch() const { return static_cast<time_t>(_microSecondsSinceEpoch / kMicroSecondsPerSecond); }         //返回当前时间戳的秒数

    public:
        static const int kMicroSecondsPerSecond = 1000 * 1000;  // 1秒=1000*1000微妙
        static const int kMicroSecondsPerMillSecond = 1000;

    private:
        int64_t _microSecondsSinceEpoch;        // 表示时间戳的微秒数(自epoch开始经历的微妙数)
    };

    inline double timeDifference(TimeStamp high, TimeStamp low) {
        int64_t diff = high.microSecond() - low.microSecond();
        return static_cast<double>(diff / TimeStamp::kMicroSecondsPerSecond);
    }

    // 如果是重复定时任务就会对此时间戳进行增加。
    inline TimeStamp addTime(TimeStamp timeStamp, double seconds) {
        int64_t delta =  static_cast<int64_t >(seconds * TimeStamp::kMicroSecondsPerSecond);
        return TimeStamp(timeStamp.microSecond() + delta);
    }
}


#endif //WEBSERVER_TIMESTAMP_H
