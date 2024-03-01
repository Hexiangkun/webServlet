//
// Created by 37496 on 2024/2/10.
//

#include "net/tcp/EventLoopThread.h"


namespace Tiny_muduo::net
{
    EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb,
                                     const std::string &name)
                                     :_loop(nullptr), _existing(false),
                                      _thread(std::bind(&EventLoopThread::threadFunc, this), name),
                                      _mutex(), _cond(), _threadInitCallback(cb)
    {

    }

    EventLoopThread::~EventLoopThread() {
        _existing = true;
        if(_loop != nullptr) {
            _loop->quit();
            _thread.join();
        }
    }

    //startLoop函数启动了子线程，子线程绑定的回调函数是EventLoopThread::threadFunc
    EventLoop *EventLoopThread::startLoop() {
        _thread.start();

        EventLoop* loop = nullptr;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (_loop == nullptr) {
                _cond.wait(lock);
            }
            loop = _loop;
        }
        return loop;
    }

    //startLoop需要返回子线程所属的EventLoop，所以就必须等待子线程成功创建出一个EventLoop对象。
    void EventLoopThread::threadFunc() {
        EventLoop loop;

        if(_threadInitCallback) {
            _threadInitCallback(&loop);
        }

        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loop = &loop;              // 等到生成EventLoop对象之后才唤醒
            _cond.notify_one();
        }

        /*
         *   执行EventLoop的loop() 开启了底层的Poller的poll()，这个是subLoop
         */
        loop.loop();

        /*
         *     loop是一个事件循环，如果往下执行说明停止了事件循环，需要关闭eventLoop
         *     此处是获取互斥锁再置loop_为空
         * */
        std::unique_lock<std::mutex> lock(_mutex);
        _loop = nullptr;
    }
}