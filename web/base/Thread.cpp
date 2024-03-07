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

    //join，调用线程阻塞等待目标线程终止，然后回收目标线程的资源
    //detach使主线程不用等待子线程，可以继续往下执行，即使主线程结束，子线程也不一定结束

    void Thread::start() {
        _started = true;
        sem_t sem;
        sem_init(&sem, false, 0);
        //开启线程
        _thread = std::make_shared<std::thread>([&]() {
            _tid = CurrentThread::tid();        //获取线程id

            sem_post(&sem);                     //v操作，调用这个函数会使其中的一个线程不再阻塞+1

            _task();                            //执行任务
        });
        /**
         * 这里必须等待获取上面新创建的线程获取tid
         * 未获取到线程tid之前会阻塞
         * 如果不适用信号量操作，别的线程访问tid时候，可能当前线程还没有获取到tid
         * */
        sem_wait(&sem);                        //阻塞当前线程直到信号量的值大于0，-1
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