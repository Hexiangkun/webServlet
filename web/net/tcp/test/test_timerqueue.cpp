//
// Created by 37496 on 2024/2/11.
//

#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Thread.h"

#include <stdio.h>
#include <unistd.h>


namespace test_timerQueue
{
    using namespace Tiny_muduo;
    using namespace Tiny_muduo::net;
    int cnt = 0;
    EventLoop* g_loop;

    void printTid()
    {
        printf("pid = %d, tid = %d\n", getpid(), currentThread::tid());
        printf("now %s\n", TimeStamp::now().toFormatString().c_str());
    }

    void print(const char* msg)
    {
        printf("msg %s %s\n", TimeStamp::now().toFormatString().c_str(), msg);
        if (++cnt == 20)
        {
            g_loop->quit();
        }
    }

    void test_timerqueue()
    {
        printTid();

        sleep(1);
        {
            EventLoopThread loopThread;
            EventLoop* loop = loopThread.startLoop();
            loop->runAfter(2, printTid);
            sleep(3);
            print("thread loop exits");
        }
    }

}