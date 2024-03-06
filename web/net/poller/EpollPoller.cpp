//
// Created by 37496 on 2024/2/1.
//

#include <unistd.h>
#include <cstring>
#include "net/poller/EpollPoller.h"
#include "log/Log.h"

namespace Tiny_muduo::net
{
    const int kNew = -1;        //channel还未添加至poller
    const int kAdded = 1;       //channel已经添加至poller
    const int kDeleted = 2;     //channel从poller删除

    EpollPoller::EpollPoller(EventLoop* loop)
            : Poller(loop),
            _epollFd(::epoll_create1(EPOLL_CLOEXEC)) ,
            _activeEvents(kInitEventListSize)
    {
        if(_epollFd < 0) {
            LOG_FATAL << "epoll_create() error: " << errno;
        }
    }

    EpollPoller::~EpollPoller() {
        if(_epollFd < 0) {
            ::close(_epollFd);
        }
    }


    TimeStamp EpollPoller::poll(int timeoutMS, ChannelList* activeChannel) {
        //监听红黑树，将满足事件的文件描述符添加至activeEvents数组中，10秒内没有事件满足，返回0
        int numEvents = ::epoll_wait(_epollFd, &(*_activeEvents.begin()),
                               static_cast<int>(_activeEvents.size()), timeoutMS);
        int savedErrno = errno;
        TimeStamp now(TimeStamp::now());
        //有事件发生
        if (numEvents > 0)
        {
            fillActiveChannels(numEvents, activeChannel);
            if (static_cast<size_t>(numEvents) == _activeEvents.size())     //扩容
            {
                _activeEvents.resize(_activeEvents.size()*2);
            }
        }       //超时
        else if (numEvents == 0)
        {
#ifdef USE_DEBUG
            LOG_DEBUG << "epoll_wait() timeout";
#endif
        }
        else    //出错
        {
            if (savedErrno != EINTR)    //不是终端错误
            {
                errno = savedErrno;
                LOG_ERROR << "EPollPoller::poll() failed!";
            }
        }
        return now;
    }


    void EpollPoller::removeChannel(Tiny_muduo::net::Channel *channel) {

        int fd = channel->fd();

        assert(_channels.find(fd) != _channels.end());
        assert(_channels[fd] == channel);
        assert(channel->isNonEvent());

        const int index = channel->index();
        assert(index == kAdded || index == kDeleted);
        size_t n = _channels.erase(fd);
        (void)n;
        assert(n == 1);
        if(index == kAdded) {
            if(channel->isNonEvent()) { //删除
                update(EPOLL_CTL_DEL, channel);
            }
        }
        channel->setIndex(kNew);
    }

    void EpollPoller::updateChannel(Channel* channel) {
        const int index = channel->index();     //获取当前channel在epoll的状态
        //未添加和已删除状态都有可能会被再次添加
        if(index == kNew || index == kDeleted) {
            int fd = channel->fd();
            if(index == kNew) { //新添加
                assert(_channels.find(fd) == _channels.end());
                _channels[fd] = channel;
            }
            else {  //重新添加
                assert(_channels.find(fd) != _channels.end());
                assert(_channels[fd] == channel);
            }
            channel->setIndex(kAdded);          //修改channel状态，此时是已添加的状态
            update(EPOLL_CTL_ADD, channel);     //向epoll对象添加channel
        }
        else {              //更新channel对应文件描述符的监听事件
            int fd = channel->fd();
            assert(_channels.find(fd) != _channels.end());
            assert(_channels[fd] == channel);
            assert(index == kAdded);
            if(channel->isNonEvent()) { //删除
                update(EPOLL_CTL_DEL, channel);
                channel->setIndex(kDeleted);
            }
            else {  //修改
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::fillActiveChannels(int numEvents, EpollPoller::ChannelList *activeChannels) const{
        assert(static_cast<size_t>(numEvents) <= _activeEvents.size());
        for(size_t i=0; i<numEvents; i++) {
            Channel* channel = static_cast<Channel*>(_activeEvents[i].data.ptr);
            channel->setRevents(static_cast<int>(_activeEvents[i].events));
            activeChannels->push_back(channel);
        }
    }


    void EpollPoller::update(int operation, Channel *channel) {
        struct epoll_event ev;
        ::memset(&ev, 0, sizeof(ev));

        int fd = channel->fd();
        ev.events = static_cast<uint32_t>(channel->events());
        ev.data.ptr = channel;

        if(::epoll_ctl(_epollFd, operation, fd, &ev) < 0) {
            if(operation == EPOLL_CTL_ADD) {
                LOG_ERROR << "epoll_ctl() add error:" << errno;
            }
            else if(operation == EPOLL_CTL_MOD) {
                LOG_ERROR << "epoll_ctl() mod error:" << errno;
            }
            else if(operation == EPOLL_CTL_DEL) {
                LOG_ERROR << "epoll_ctl() del error:" << errno;
            }
        }
    }
}