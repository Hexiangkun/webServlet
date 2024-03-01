//
// Created by 37496 on 2024/2/27.
//

#ifndef WEBSERVER_TIMERID_H
#define WEBSERVER_TIMERID_H

#include <cstdint>

namespace Tiny_muduo::net
{
    class Timer;
    class TimerId
    {
    public:
        TimerId():_timer(nullptr), _sequence(0) {}

        TimerId(Timer* timer, int64_t seq) : _timer(timer), _sequence(seq) {}


        friend class TimerQueue;

    private:
        Timer* _timer;
        int64_t _sequence;
    };
}
#endif //WEBSERVER_TIMERID_H
