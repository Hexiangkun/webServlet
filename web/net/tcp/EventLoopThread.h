//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_EVENTLOOPTHREAD_H
#define WEBSERVER_EVENTLOOPTHREAD_H


#include <condition_variable>
#include "net/tcp/EventLoop.h"
#include "base/Thread.h"

namespace Tiny_muduo
{
    namespace net
    {
        class EventLoopThread
        {
        public:
            using ThreadInitCallback = std::function<void(EventLoop*)>;

            EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                            const std::string& name = "");

            ~EventLoopThread();

            EventLoop* startLoop();
        private:
            void threadFunc();

            EventLoop* _loop;
            bool _existing;
            Thread _thread;
            std::mutex _mutex;
            std::condition_variable _cond;
            ThreadInitCallback _threadInitCallback;
        };

    }
}

#endif //WEBSERVER_EVENTLOOPTHREAD_H
