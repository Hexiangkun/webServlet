//
// Created by 37496 on 2024/2/11.
//

#include "EventLoop.h"
#include "Thread.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

namespace test
{
    using namespace Tiny_muduo;
    using namespace Tiny_muduo::net;

    EventLoop* g_loop;

    void callback()
    {
        printf("callback(): pid = %d, tid = %d\n", getpid(), currentThread::tid());
        EventLoop anotherLoop;
    }

    void threadFunc()
    {
        printf("threadFunc(): pid = %d, tid = %d\n", getpid(), currentThread::tid());

        assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
        EventLoop loop;
        assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
        loop.runAfter(1.0, callback);
        loop.loop();
    }

    void test_eventloop()
    {
        printf("main(): pid = %d, tid = %d\n", getpid(), currentThread::tid());

        assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
        EventLoop loop;
        assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

        Thread thread(threadFunc);
        thread.start();

        loop.loop();
    }
}

