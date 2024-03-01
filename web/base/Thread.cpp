//
// Created by 37496 on 2024/2/2.
//

#include <stdexcept>
#include <semaphore.h>
#include "base/Thread.h"
#include "base/CurrentThread.h"

namespace Tiny_muduo
{
    std::atomic_int32_t Thread::_numThread{0};

    Thread::Thread(Thread::ThreadFunc taskFunc, const std::string &name)
            : _joined(false),       //还未设置等待线程
            _started(false),        //还未开始
            _task(std::move(taskFunc)),
            _tid(0),                //初始置为空
            _name(name)
    {
        setDefaultName();   //设置线程索引和线程名
    }

    Thread::~Thread() {
        //线程启动 && 未设置等待线程
        if(_started && !_joined) {
            _thread->detach();  //线程分离，（不需要等待线程结束，不会产生孤儿线程）
        }
    }

    void Thread::start() {
        _started = true;
        sem_t sem;
        sem_init(&sem, false, 0);
        //开启线程
        _thread = std::make_shared<std::thread>([&]() {
            _tid = CurrentThread::tid();        //获取线程id

            sem_post(&sem);                     //v操作

            _task();                            //执行任务
        });

        sem_wait(&sem);
    }

    void Thread::join() {
        _joined = true;
        _thread->join();        //等待线程执行完毕
    }


    void Thread::setDefaultName() {
        int curThreadCount = ++_numThread;
        if(_name.empty()) {
            _name = "Thread_" + std::to_string(curThreadCount);
        }
    }
}