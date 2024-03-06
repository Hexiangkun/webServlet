//
// Created by 37496 on 2024/2/10.
//
#include <sys/timerfd.h>
#include "net/timer/TimerQueue.h"
#include "net/timer/Timer.h"
#include "net/timer/TimerId.h"
#include "net/tcp/EventLoop.h"
#include "log/Log.h"

namespace Tiny_muduo::net
{
    namespace detail
    {
        //用于创建并返回timerfd
        /*
         * clockid：常用取值为CLOCK_REALTIME和CLOCK_MONOTONIC，CLOCK_REALTIME表示相对时间，
         * 即从从1970.1.1到现在的时间，如果更改系统时间，从1970.1.1到现在时间就会发生变化。
         * CLOCK_MONOTONIC表示绝对时间，获取的时间为系统最近一次重启到现在的时间，更该系统时间对其没影响
         * flag：TFD_NONBLOCK(设置timerfd非阻塞)，TFD_CLOEXEC（fork子进程后在子进程中关掉父进程打开的文件描述符
         * **/
        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

            if(timerfd < 0) {
                LOG_ERROR << "timer_create failed";
            }
            return timerfd;
        }

        void readTimerfd(int timerfd) {
            uint64_t read_bytes;
            ssize_t readn = ::read(timerfd, &read_bytes, sizeof(read_bytes));
            if(readn != sizeof(read_bytes)) {
                LOG_ERROR << "TimerQueue::readTimerfd read_size < 0";
            }
        }

        struct timespec howMuchTimeFromNow(TimeStamp when)
        {
            int64_t microseconds = when.microSecondsSinceEpoch() - TimeStamp::now().microSecondsSinceEpoch();
            if(microseconds < 100)
            {
                microseconds = 100;
            }
            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(microseconds / TimeStamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long>((microseconds % TimeStamp::kMicroSecondsPerSecond) * 1000);

            return ts;
        }

        void resetTimerfd(int timerfd, TimeStamp expiration) {
            struct itimerspec newValue;
            struct itimerspec oldValue;
            memset(&newValue, '\0', sizeof(newValue));
            memset(&oldValue, '\0', sizeof(oldValue));

            newValue.it_value = howMuchTimeFromNow(expiration);

            /*
             * 开启或者停止定时器
             * fd：timerfd_create()返回的文件描述符
             * flags：1代表设置的是绝对时间；为0代表相对时间
             * new_value：指定新的超时信息（超时时刻和超时间隔），设定new_value.it_value非零则会在内核启动定时器，否则关闭定时器，如果new_value.it_interval为0，则定时器只触发一次
             * old_value：返回原来的超时信息，如果不需要知道之前的超时信息，可以设置为NULL
             * */
            if(::timerfd_settime(timerfd, 0, &newValue, &oldValue)) {
                LOG_ERROR << "timerfd_settime failed";
            }
        }
    }


    TimerQueue::TimerQueue(EventLoop *loop)
            :_timerfd(detail::createTimerfd()),
            _loop(loop),
            _timerfdChannel(loop, _timerfd),
            _timers(),
            _callingExpiredTimers(false)
    {
        _timerfdChannel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
        _timerfdChannel.enableReading();
    }

    TimerQueue::~TimerQueue() {
        _timerfdChannel.disableAll();
        _timerfdChannel.removeChannel();
        ::close(_timerfd);
        for(const Entry& timer : _timers) {
            delete timer.second;
        }
    }

    void TimerQueue::handleRead() {
        _loop->assertInLoopThread();

        TimeStamp now = TimeStamp::now();
        detail::readTimerfd(_timerfd);

        std::vector<Entry> expired = getExpired(now);

        _callingExpiredTimers = true;
        _cancelingTimers.clear();
        for(const Entry& entry : expired) {     //遍历到期定时器，调用回调函数
            entry.second->run();
        }
        _callingExpiredTimers = false;
        reset(expired, now);                    //重新设定到期定时器
    }

    TimerId TimerQueue::addTimer(TimerQueue::TimerCallback cb, TimeStamp when, double interval) {
        Timer* timer = new Timer(std::move(cb), when, interval);
        _loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
        return TimerId(timer, timer->sequence());
    }

    void TimerQueue::cancel(Tiny_muduo::net::TimerId timerId) {
        _loop->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
    }

    void TimerQueue::addTimerInLoop(Timer *timer) {
        _loop->assertInLoopThread();
        bool earliestChanged = insert(timer);       //是否取代了最早的定时触发时间
        if(earliestChanged) {
            detail::resetTimerfd(_timerfd, timer->expiration());
        }
    }

    void TimerQueue::cancelInLoop(Tiny_muduo::net::TimerId timerId) {
        _loop->assertInLoopThread();
        assert(_timers.size() == _activeTimers.size());

        ActiveTimer timer(timerId._timer, timerId._sequence);
        auto it = _activeTimers.find(timer);

        if(it != _activeTimers.end()) {
            size_t n = _timers.erase(Entry(it->first->expiration(), it->first));
            assert(n == 1);
            delete it->first;
            _activeTimers.erase(it);
        }
        else if(_callingExpiredTimers) {
            _cancelingTimers.insert(timer);
        }
        assert(_timers.size() == _activeTimers.size());
    }

    void TimerQueue::adjustInLoop(Tiny_muduo::net::TimerId timerId) {

    }

    bool TimerQueue::insert(Timer *timer) {
        _loop->assertInLoopThread();

        bool earliestChanged = false;

        TimeStamp when = timer->expiration();
        auto it = _timers.begin();

        if(it == _timers.end() || when < it->first) {
            earliestChanged = true;
        }

        _timers.insert(Entry(when, timer));
        _activeTimers.insert(ActiveTimer(timer, timer->sequence()));
        return earliestChanged;
    }


    std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now) {
        assert(_timers.size() == _activeTimers.size());

        std::vector<Entry> expired;

        Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
        auto end = _timers.lower_bound(sentry);

        std::copy(_timers.begin(), end, back_inserter(expired));
        _timers.erase(_timers.begin(), end);

        for(auto& it : expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            size_t n = _activeTimers.erase(timer);
            assert(n == 1);
        }
        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, Tiny_muduo::TimeStamp now) {
        TimeStamp nextExpired;
        for(const Entry& it : expired) {
            ActiveTimer activeTimer(it.second, it.second->sequence());
            if(it.second->repeat() && _cancelingTimers.find(activeTimer) == _cancelingTimers.end())  {   // 重复任务则继续执行
                auto timer = it.second;
                timer->restart(TimeStamp::now());
                insert(timer);
            }
            else {
                delete it.second;
            }
        }

        if(!_timers.empty()) {  // 如果重新插入了定时器，需要继续重置timerfd
            nextExpired = (_timers.begin()->second)->expiration();
        }
        if(nextExpired.valid()) {
            detail::resetTimerfd(_timerfd, nextExpired);
        }
    }
}