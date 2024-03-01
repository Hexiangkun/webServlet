#include "base/ThreadPool.h"
#include "base/CurrentThread.h"
#include "log/Log.h"

#include <stdio.h>
#include <unistd.h>  // usleep

void print()
{
    printf("tid=%d\n", Tiny_muduo::CurrentThread::tid());
}

void printString(const std::string& str)
{
    LOG_INFO << str;
    usleep(100*1000);
}

void test(int maxSize)
{
    LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    Tiny_muduo::ThreadPool pool("MainThreadPool");
    pool.setThreadPoolSize(5);
    pool.start();

    LOG_WARN << "Adding";
    pool.add(print);
    pool.add(print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.add(std::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";
    pool.stop();
}


void test2()
{
    LOG_WARN << "Test ThreadPool by stoping early.";
    Tiny_muduo::ThreadPool pool("ThreadPool");
    pool.setThreadPoolSize(3);
    pool.start();

    Tiny_muduo::Thread thread1([&pool]()
                               {
                                   for (int i = 0; i < 20; ++i)
                                   {
                                       pool.add([]{sleep(5);});
                                   }
                               }, "thread1");
    thread1.start();

    sleep(5000000);
    LOG_WARN << "stop pool";
    pool.stop();  // early stop

    thread1.join();
    // run() after stop()
    pool.add(print);
    LOG_WARN << "test2 Done";
}

void test_threadpool()
{
    test(0);
    test(1);
    test(5);
    test(10);
    test(50);
    test2();
}
