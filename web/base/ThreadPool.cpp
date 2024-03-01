//
// Created by 37496 on 2024/2/10.
//

#include "ThreadPool.h"
#include "log/Log.h"

namespace Tiny_muduo
{
    ThreadPool::ThreadPool(const std::string &name)
                : _mutex(),
                _cond(),
                _name(name),
                _running(false)
    {

    }

    ThreadPool::~ThreadPool() {
        stop();
        for(const auto& t : _threads) {
            t->join();              //等待直到线程结束
        }
    }

    void ThreadPool::start() {
        _running = true;
        _threads.reserve(_threadSize);
        for(int i=0; i<_threadSize; i++) {
            _threads.emplace_back(std::make_unique<Thread>(std::bind(&ThreadPool::runInThread, this), _name + std::to_string(i+1)));
            _threads[i]->start();
        }
        //不创建新线程，执行回调函数
        if(_threadSize == 0 && _threadInitCallback) {
            _threadInitCallback();
        }
    }

    void ThreadPool::stop() {
        std::lock_guard<std::mutex> lock(_mutex);
        _running = false;
        _cond.notify_all();         //唤醒所有线程
    }

    void ThreadPool::add(ThreadPool::ThreadFunc func) {
        std::unique_lock<std::mutex> lock(_mutex);
        _que_funcs.push_back(std::move(func));
        _cond.notify_one();
    }

    size_t ThreadPool::queueSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _que_funcs.size();
    }

    void ThreadPool::runInThread() {
        try {
            if(_threadInitCallback) {
                _threadInitCallback();
            }

            ThreadFunc  task;
            while (true) {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    while (_que_funcs.empty()) {
                        if(!_running) {
                            return;
                        }
                        _cond.wait(lock);
                    }
                    task = _que_funcs.front();
                    _que_funcs.pop_front();

                }
                if(task != nullptr) {
                    task();
                }
            }
        }
        catch (...) {
            LOG_WARN << "runInThread throw exception";
        }
    }
}