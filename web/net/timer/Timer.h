//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include <functional>
#include "base/TimeStamp.h"
#include "base/Atomic.h"

namespace Tiny_muduo
{
    namespace net
    {
        class Timer
        {
        public:
            using TimerCallback = std::function<void()>;

            Timer(TimerCallback cb, TimeStamp when, double interval)
                :_timerCB(std::move(cb)),
                _expiration(when),
                _interval(interval),
                _repeat(interval > 0),
                _sequence(_numCreated.incrementAndGet())
            {

            }

            void run() const {
                _timerCB();
            }

            TimeStamp expiration() const { return _expiration; }
            bool repeat() const { return _repeat; }
            int64_t sequence() const { return _sequence; }

            void restart(TimeStamp now);           //重启计时器（如果是非重复事件则到期事件置为0）

            static int64_t numCreated() { return _numCreated.get(); }
        private:
            const TimerCallback _timerCB;   //定时器回调函数
            TimeStamp _expiration;          //下一次超时时刻
            const double _interval;         //超时时间间隔，如果时一次性定时器，该值为0
            const bool _repeat;             //是否重复，false为一次性定时器

            const int64_t _sequence;
            static AtomicInt64 _numCreated;
        };
    }
}



#endif //WEBSERVER_TIMER_H
