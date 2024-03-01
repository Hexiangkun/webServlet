
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Thread.h"

#include <stdio.h>
#include <unistd.h>

using namespace Tiny_muduo;
using namespace Tiny_muduo::net;

void print(EventLoop* p = NULL)
{
    printf("print: pid = %d, tid = %d, loop = %p\n",
           getpid(), currentThread::tid(), p);
}

void quit(EventLoop* p)
{
    print(p);
    p->quit();
}

void test_eventLoopThread()
{
    print();

    {
        EventLoopThread thr1;  // never start
    }

    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
        sleep(25);
    }

    {
        // quit() before dtor
        EventLoopThread thr3;
        EventLoop* loop = thr3.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        sleep(25);
    }
}