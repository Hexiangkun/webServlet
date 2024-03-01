//
// Created by 37496 on 2024/2/10.
//

#include "base/CurrentThread.h"

namespace Tiny_muduo
{
    namespace CurrentThread
    {
        __thread int t_cachedTid = 0;

        void cacheTid() {
            if (t_cachedTid == 0) {
                t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            }
        }


        void sleepUsec(int64_t usec)
        {
            struct timespec ts = { 0, 0 };
            ts.tv_sec = static_cast<time_t>(usec / 1000 * 1000);
            ts.tv_nsec = static_cast<long>(usec % (1000 * 1000) * 1000);
            ::nanosleep(&ts, NULL);
        }
    }
}