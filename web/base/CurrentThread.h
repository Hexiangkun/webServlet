//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_CURRENTTHREAD_H
#define WEBSERVER_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>
#include <ctime>

namespace Tiny_muduo
{
    namespace CurrentThread
    {
        extern __thread int t_cachedTid;    //保存tid缓冲，避免多次系统调用

        void cacheTid();


        inline int tid() {
            if(__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }


        void sleepUsec(int64_t usec);
    }
}


#endif //WEBSERVER_CURRENTTHREAD_H
