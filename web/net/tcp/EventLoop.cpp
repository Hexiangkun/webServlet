//
// Created by 37496 on 2024/2/10.
//

#include <sys/eventfd.h>
#include "log/Log.h"
#include "net/tcp/EventLoop.h"
#include "net/tcp/Channel.h"
#include "net/poller/Poller.h"
#include "net/timer/TimerQueue.h"

namespace
{
#pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
            // LOG_TRACE << "Ignore SIGPIPE";
        }
    };
#pragma GCC diagnostic error "-Wold-style-cast"
    IgnoreSigPipe initObj;
}

namespace Tiny_muduo::net
{
    __thread EventLoop* t_loopInThisThread = nullptr;
    const int kPollTimeMs = 10000;      // 定义默认的Poller IO复用接口的超时时间

    namespace detail
    {
        int createEventFd() {
            int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            if(evfd < 0) {
                LOG_ERROR << "eventfd error: " << errno;
            }
            return evfd;
        }
    }

    EventLoop* EventLoop::getEventLoopOfCurrentThread()
    {
        return t_loopInThisThread;
    }


    EventLoop::EventLoop()
            :_looping(false),
            _quit(false),
            _callPendingTasks(false),
            _eventHandling(false),
            _threadId(CurrentThread::tid()),
            _epoller(Poller::newDefaultPoller(this)),
            _timerQueue(std::make_unique<TimerQueue>(this)),
            _wakeupFd(detail::createEventFd()),
            _weakUpChannel(std::make_unique<Channel>(this, _wakeupFd)),
            _currentActiveChannel(nullptr)
    {
#ifdef DEBUG
        LOG_DEBUG << "EventLoop created " << this << " the index is " << _threadId;
        LOG_DEBUG << "EventLoop created wakeupFd " << _weakUpChannel->fd();
#endif
        if (t_loopInThisThread)
        {
            LOG_FATAL << "Another EventLoop" << t_loopInThisThread << " exists in this thread " << _threadId;
        }
        else
        {
            t_loopInThisThread = this;
        }

        _weakUpChannel->setReadCallback(std::bind(&EventLoop::handleWakeUp, this));
        _weakUpChannel->enableReading();    //每一个EventLoop都将监听wakeupChannel的EPOLLIN事件
    }

    EventLoop::~EventLoop() {
#ifdef DEBUG
        LOG_DEBUG << "EventLoop " << this << " of thread " << _threadId
                  << " destructs in thread " << CurrentThread::tid();
#endif
        _weakUpChannel->disableAll();
        _weakUpChannel->removeChannel();

        ::close(_wakeupFd);
        t_loopInThisThread = nullptr;
    }

    void EventLoop::loop() {
        assert(!_looping);
        assertInLoopThread();
        _looping = true;
        _quit = false;
#ifdef DEBUG
        LOG_INFO << "EventLoop " << this << " start looping";
#endif
        while (!_quit) {
            _activeChannels.clear();
            _pollReturnTime = _epoller->poll(kPollTimeMs, &_activeChannels);

            if (log::Logger::get_logLevel() <= log::LogLevel::INFO)
            {
                printActiveChannels();
            }

            _eventHandling = true;
            for(Channel* channel : _activeChannels) {
                _currentActiveChannel = channel;
                // Poller监听哪些channel发生了事件 然后上报给EventLoop 通知channel处理相应的事件
                channel->handleEvents(_pollReturnTime);
            }
            _currentActiveChannel = nullptr;
            _eventHandling = false;
            // 执行当前EventLoop事件循环需要处理的回调操作
            /**
             * IO thread：mainLoop accept fd 打包成 chennel 分发给 subLoop
             * mainLoop实现注册一个回调，交给subLoop来执行，wakeup subLoop 之后，让其执行注册的回调操作
             * 这些回调函数在 std::vector<Functor> pendingFunctors_; 之中
             */
            /**
            * 执行当前EventLoop事件循环需要处理的回调操作 对于线程数 >=2 的情况 IO线程 mainloop(mainReactor) 主要工作：
            * accept接收连接 => 将accept返回的connfd打包为Channel => TcpServer::newConnection通过轮询将TcpConnection对象分配给subloop处理
            *
            * mainloop调用queueInLoop将回调加入subloop（该回调需要subloop执行 但subloop还在poller_->poll处阻塞） queueInLoop通过wakeup将subloop唤醒
            **/
            doPendingTask();
        }
#ifdef DEBUG
        LOG_INFO << "EventLoop " << this << " stop looping";
#endif
        _looping = false;
    }

    /**
     * 退出事件循环
     * 1. 如果loop在自己的线程中调用quit成功了 说明当前线程已经执行完毕了loop()函数的poller_->poll并退出
     * 2. 如果不是当前EventLoop所属线程中调用quit退出EventLoop 需要唤醒EventLoop所属线程的epoll_wait
     *
     * 比如在一个subloop(worker)中调用mainloop(IO)的quit时 需要唤醒mainloop(IO)的poller_->poll 让其执行完loop()函数
     *
     * ！！！ 注意： 正常情况下 mainloop负责请求连接 将回调写入subloop中 通过生产者消费者模型即可实现线程安全的队列
     * ！！！       但是muduo通过wakeup()机制 使用eventfd创建的wakeupFd_ notify 使得mainloop和subloop之间能够进行通信
     **/
    void EventLoop::quit() {
        _quit = true;
        /**
         * TODO:生产者消费者队列派发方式和muduo的派发方式
         * 有可能是别的线程调用quit(调用线程不是生成EventLoop对象的那个线程)
         * 比如在工作线程(subLoop)中调用了IO线程(mainLoop)
         * 这种情况会唤醒主线程
        */
        if (isInLoopThread())
        {
            wakeup();
        }
    }

    // 在当前loop中执行cb
    void EventLoop::runInLoop(EventLoop::TaskFunc task) {
        if(isInLoopThread()) {
            task();
        }
        else {
            // 在非当前eventLoop线程中执行回调函数，需要唤醒eventLoop所在线程
            queueInLoop(std::move(task));
        }
    }

    // 把cb放入队列中 唤醒loop所在的线程执行cb
    void EventLoop::queueInLoop(EventLoop::TaskFunc task) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _pendingTasks.emplace_back(std::move(task));
        }
        // 唤醒相应的，需要执行上面回调操作的loop线程
        /** 
         * TODO:
         * std::atomic_bool callingPendingFunctors_; 标志当前loop是否有需要执行的回调操作
         * 这个 || callingPendingFunctors_ 比较有必要，因为在执行回调的过程可能会加入新的回调
         * 则这个时候也需要唤醒，否则就会发生有事件到来但是仍被阻塞住的情况
         */
        if(!isInLoopThread() || _callPendingTasks) {
            wakeup();
        }
    }

    // 用来唤醒loop所在线程 向wakeupFd_写一个数据 wakeupChannel就发生读事件 当前loop线程就会被唤醒
    void EventLoop::wakeup() {
        uint64_t  one = 1;
        ssize_t n = ::write(_wakeupFd, &one, sizeof(one));
        if(n != sizeof(one)) {
            LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes instead of 8";
        }
    }

    void EventLoop::handleWakeUp() {
        uint64_t  one = 1;
        ssize_t n = ::read(_wakeupFd, &one, sizeof(one));
        if(n != sizeof(one)) {
            LOG_ERROR << "EventLoop::wakeup read " << n << " bytes instead of 8";
        }
    }

    void EventLoop::updateChannel(Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        _epoller->updateChannel(channel);
    }

    void EventLoop::removeChannel(Tiny_muduo::net::Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        if(_eventHandling) {
            assert(_currentActiveChannel == channel || std::find(_activeChannels.begin(), _activeChannels.end(), channel) == _activeChannels.end());
        }
        _epoller->removeChannel(channel);
    }

    bool EventLoop::hasChannel(Tiny_muduo::net::Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        return _epoller->hasChannel(channel);
    }

    TimerId EventLoop::runAt(Tiny_muduo::TimeStamp timeStamp, Tiny_muduo::net::EventLoop::TaskFunc &&cb) {
        return _timerQueue->addTimer(std::move(cb), timeStamp, 0.0);
    }

    TimerId EventLoop::runAfter(double waitTime, EventLoop::TaskFunc &&cb) {
        TimeStamp time(addTime(TimeStamp::now(), waitTime));
        return runAt(time, std::move(cb));
    }

    TimerId EventLoop::runEvery(double interval, EventLoop::TaskFunc &&cb) {
        TimeStamp time(addTime(TimeStamp::now(), interval));
        return _timerQueue->addTimer(std::move(cb), time, interval);
    }

    void EventLoop::cancel(Tiny_muduo::net::TimerId timerId) {
        _timerQueue->cancel(timerId);
    }

    void EventLoop::adjust(Tiny_muduo::net::TimerId timerId , TimeStamp now) {
        _timerQueue->adjust(timerId, now);
    }

    void EventLoop::doPendingTask() {
        std::vector<TaskFunc > funcs;
        _callPendingTasks = true;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            funcs.swap(_pendingTasks);
        }

        for(const auto& func : funcs) {
            func();
        }

        _callPendingTasks = false;
    }

    void EventLoop::abortNotInLoopThread()
    {
    #ifdef DEBUG
            LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
                      << " was created in threadId_ = " << _threadId
                      << ", current thread id = " <<  CurrentThread::tid();
    #endif
    }

    void EventLoop::printActiveChannels() const
    {
        for (const auto& channel : _activeChannels)
        {
            const Channel* ch = channel;
#ifdef DEBUG
            LOG_TRACE << "{" << ch->reventsToString() << "} ";
#endif
        }
    }
}
