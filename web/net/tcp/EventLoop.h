//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H


#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <any>
#include "base/Noncopyable.h"
#include "base/CurrentThread.h"
#include "base/TimeStamp.h"
#include "net/timer/TimerId.h"

#ifdef USE_LOCKFREEQUEUE
#include "base/KLockFreeQueue.h"
#endif

#ifdef USE_SPINLOCK
#include ".base/KSpinLock.h"
#endif

namespace Tiny_muduo
{
    namespace net
    {
        class Channel;
        class Poller;
        class TimerQueue;

        // 事件循环类 负责 Channel 和 Poller(epoll的抽象)的沟通
        class EventLoop : public Noncopyable
        {
        public:
            using _ptr = std::shared_ptr<EventLoop>;
            using _wptr = std::weak_ptr<EventLoop>;
            using TaskFunc = std::function<void()>;
        public:
            //每个线程只能有一个EventLoop对象
            EventLoop();
            ~EventLoop();

            //EventLoop的对象生命周期通常和所属IO线程一样长，不必是堆上的对象
            void loop();                        //开启事件循环
            void quit();                        //退出事件循环

            void runInLoop(TaskFunc task);      //在当前loop中执行任务cb
            void queueInLoop(TaskFunc task);    //把cb放入队列中，唤醒loop所在线程，执行cb

            // 用来唤醒loop所在的线程的,向wakeupfd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
            void wakeup();

            // EventLoop的方法 => Poller的方法
            void removeChannel(Channel* channel);
            void updateChannel(Channel* channel);
            bool hasChannel(Channel *channel);

            /**  Timer Event
             * 允许跨线程使用，比方说我想在某个IO线程中执行超时回调。
             * 这就带来 线程安全性方面的问题，muduo的解决办法不是加锁，
             * 而是把对 TimerQueue的操作转移到IO线程来进行 -> runInLoop
             */
            TimerId runAt(TimeStamp timeStamp, TaskFunc&& cb);
            TimerId runAfter(double waitTime, TaskFunc&& cb);
            TimerId runEvery(double interval, TaskFunc&& cb);

            void cancel(TimerId timerId);

            static EventLoop* getEventLoopOfCurrentThread();
            /**
             * 每个EventLoop都保存创建自己的线程tid
             * 我们可以通过CurrentThread::tid()获取当前执行线程的tid然后和EventLoop保存的进行比较
             * */
            bool isInLoopThread() const { return _threadId == CurrentThread::tid(); }
            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }
        private:
            //就是读，写啥读啥无所谓，就是为了唤醒loop线程执行回调
            void handleWakeUp();
            void doPendingTask();
            void abortNotInLoopThread();
            void printActiveChannels() const;

        private:
            std::atomic_bool _looping;  //是否循环
            std::atomic_bool _quit;     //退出循环
            bool _eventHandling;

            std::unique_ptr<Poller> _epoller;

            std::unique_ptr<TimerQueue> _timerQueue;

            int _wakeupFd;
            std::unique_ptr<Channel> _weakUpChannel;

            const pid_t _threadId;      //记录loop所在线程id

            TimeStamp _pollReturnTime;              // poller返回发生事件的channels的返回时间

            std::atomic_bool _callPendingTasks;     // 标志当前loop是否有需要执行的回调操作

            std::vector<Channel*> _activeChannels;
            Channel* _currentActiveChannel;

            std::any _context;

#ifdef USE_LOCKFREEQUEUE
            LockFreeQueue<TaskFunc> _pendingTasks;
#else
    // 锁类型的选择
    #ifdef USE_SPINLOCK
                SpinLock spinlock;
    #else
                std::mutex _mutex;
    #endif
            std::vector<TaskFunc> _pendingTasks;// 存储loop跨线程需要执行的所有回调操作
#endif
        };
    }
}


#endif //WEBSERVER_EVENTLOOP_H
