//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_TIMERQUEUE_H
#define WEBSERVER_TIMERQUEUE_H


#include <functional>
#include <set>
#include "base/TimeStamp.h"
#include "net/tcp/Channel.h"


namespace Tiny_muduo
{
    namespace net
    {
        class Timer;
        class TimerId;
        class EventLoop;
        class TimerQueue
        {
        private:
            using Entry = std::pair<TimeStamp, Timer*>;
            using ActiveTimer = std::pair<Timer*, int64_t >;
        public:
            using TimerCallback = std::function<void()>;

            explicit TimerQueue(EventLoop* loop);
            ~TimerQueue();

            //插入定时器
            TimerId addTimer(TimerCallback cb, TimeStamp when, double interval);

            void cancel(TimerId timerId);
            void adjust(TimerId timerId, TimeStamp now);
        private:
            void handleRead();                                              //定时器读事件触发函数

            void addTimerInLoop(Timer* timer);                              //在本loop中添加定时器
            void cancelInLoop(TimerId timerId);
            void adjustInLoop(TimerId timerId);

            bool insert(Timer* timer);                                      //插入定时器队列内部

            std::vector<Entry> getExpired(TimeStamp now);                   //移除到期计时器

            void reset(const std::vector<Entry>& expired, TimeStamp now);   //重置这些到期定时器

            EventLoop* _loop;                   //所属EventLoop
            const int _timerfd;                 //timerfd为Linux提供的定时器接口
            Channel _timerfdChannel;            //封装timerfd文件描述符

            std::set<Entry> _timers;            //定时器队列

            std::set<ActiveTimer> _activeTimers;
            std::set<ActiveTimer> _cancelingTimers;
            bool _callingExpiredTimers;         //表明正在获取超时定时器

        };
    }
}


#endif //WEBSERVER_TIMERQUEUE_H
