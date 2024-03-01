//
// Created by 37496 on 2024/2/10.
//

#include "net/timer/Timer.h"

namespace Tiny_muduo::net
{
    AtomicInt64 Timer::_numCreated;

    void Timer::restart(TimeStamp now) {
        // 如果是重复定时事件，则继续添加定时事件，得到新事件到期事件
        if(_repeat) {
            _expiration = addTime(now, _interval);
        }
        else {
            _expiration = TimeStamp::invalid();
        }
    }
}