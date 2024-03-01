//
// Created by 37496 on 2024/2/10.
//

#include <cassert>
#include "net/tcp/EventLoopThreadPool.h"


namespace Tiny_muduo::net
{
    EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
            :_mainLoop(baseLoop), _name(name), _started(false),
            _numThreads(0), _next(0)
    {

    }

    void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
        assert(!_started);
        _mainLoop->assertInLoopThread();
        _started = true;

        for(int i=0 ;i < _numThreads; i++) {
            EventLoopThread* t= new EventLoopThread(cb, _name +"_"+ std::to_string(i+1));
            EventLoop* loop = t->startLoop();

            _threads.push_back(std::unique_ptr<EventLoopThread>(t));
            _subLoops.push_back(loop);
        }

        if(_numThreads == 0 && cb) {
            cb(_mainLoop);
        }
    }

    EventLoop *EventLoopThreadPool::getNextLoop() {
        EventLoop* loop = _mainLoop;

        if(!_subLoops.empty()) {
            loop = _subLoops[_next];
            ++_next;
            if(_next >= _subLoops.size()) {
                _next = 0;
            }
        }
        return loop;
    }

    std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
        if(_subLoops.empty()) {
            return std::vector<EventLoop*>(1, _mainLoop);
        }
        else {
            return _subLoops;
        }
    }
}