//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_EVENTLOOPTHREADPOOL_H

#include "net/tcp/EventLoop.h"
#include "net/tcp/EventLoopThread.h"

namespace Tiny_muduo
{
    namespace net
    {
        class EventLoopThreadPool
        {
        public:
            using ThreadInitCallback = std::function<void(EventLoop*)>;

            EventLoopThreadPool(EventLoop* baseLoop, const std::string& name = "EventLoopThreadPool");
            ~EventLoopThreadPool() = default;

            void setThreadNum(int numThreads) { _numThreads = numThreads; }

            void start(const ThreadInitCallback& cb = ThreadInitCallback());

            EventLoop* getNextLoop();

            std::vector<EventLoop *> getAllLoops();

            bool started() const { return _started; }
            const std::string name() const { return _name; }
        private:
            EventLoop* _mainLoop;
            std::string _name;
            bool _started;
            int _numThreads;
            size_t _next;
            std::vector<std::unique_ptr<EventLoopThread>> _threads;
            std::vector<EventLoop*> _subLoops;
        };
    }
}


#endif //WEBSERVER_EVENTLOOPTHREADPOOL_H
