//
// Created by 37496 on 2024/2/2.
//

#ifndef WEBSERVER_THREAD_H
#define WEBSERVER_THREAD_H

#include <memory>
#include <functional>
#include <string>
#include <atomic>
#include <thread>
#include "base/Noncopyable.h"

namespace Tiny_muduo
{
    class Thread : public Noncopyable
    {
    public:
        using _ptr = std::shared_ptr<Thread>;
        using ThreadFunc = std::function<void()>;

        explicit Thread(ThreadFunc taskFunc, const std::string& name = "");
        ~Thread();

        void start();       //开启线程
        void join();        //等待线程

        bool joinable() { return !_joined && _thread; }

        const std::string& name() const { return _name; }

        bool started() const { return _started; }

        const pid_t tid() { return _tid; }

        static int numCreated() { return _numThread; }

    private:
        void setDefaultName();          //设置线程名

    private:
        volatile bool _started;                 //是否启动线程
        bool _joined;                           //是否等待该线程

        std::shared_ptr<std::thread> _thread;   //线程
        pid_t _tid;                             //线程号

        ThreadFunc _task;                       //要执行的任务, 其实保存的是EventLoopThread::threadFunc()
        std::string _name;                      //线程名

        static std::atomic_int32_t _numThread;     //线程数量/索引

    };
}



#endif //WEBSERVER_THREAD_H
