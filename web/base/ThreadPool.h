//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <functional>
#include <deque>
#include "base/Noncopyable.h"
#include "base/Thread.h"

namespace Tiny_muduo
{
    class ThreadPool : public Noncopyable
    {
    public:
        using ThreadFunc = std::function<void()>;

        ThreadPool(const std::string& name = "ThreadPool");
        ~ThreadPool();

        void start();
        void stop();
        void add(ThreadFunc func);

        void setThreadInitCallback(const ThreadFunc& func) { _threadInitCallback = func; }
        void setThreadPoolSize(const int& num) { _threadSize = num; }

        const std::string& name() const { return _name; }
        size_t queueSize() const;
    private:
        void runInThread();
    private:
        mutable std::mutex _mutex;
        std::condition_variable _cond;
        std::string _name;
        ThreadFunc _threadInitCallback;

        std::vector<std::unique_ptr<Thread>> _threads;
        std::deque<ThreadFunc > _que_funcs;
        bool _running;
        size_t _threadSize;
    };
}





#endif //WEBSERVER_THREADPOOL_H
