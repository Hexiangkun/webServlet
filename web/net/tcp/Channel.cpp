//
// Created by 37496 on 2024/2/10.
//

#include <sys/epoll.h>
#include <sys/poll.h>
#include <sstream>
#include "net/tcp/Channel.h"
#include "log/Log.h"
#include "net/tcp/EventLoop.h"

namespace Tiny_muduo::net
{
    const int Channel::kNoneEvent = 0;
    const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;     // POLLPRI 有紧迫数据可读
    const int Channel::kWriteEvent = EPOLLOUT;
    const int Channel::kET = EPOLLET;

    Channel::Channel(EventLoop *loop, int fd)
            : _loop(loop),
            _fd(fd),
            _events(0),
            _revents(0),
            _index(-1),
            _tied(false),
            _eventHandling(false),
            _addedToLoop(false)
    {

    }

    Channel::~Channel() {
        assert(!_eventHandling);
        assert(!_addedToLoop);

    }

    //fd得到epoll通知后，处理事件
    void Channel::handleEvents(Tiny_muduo::TimeStamp receiveTime) {
        /*
         *  调用channel::tie会设置_tied=true
         *  而TcpConnection
         * */
        // 提升tie_为shared_ptr，如果提升成功，说明指向一个存在的对象
        if(_tied) {
            std::shared_ptr<void> guard = _tie.lock();      // 变成shared_ptr增加引用计数，防止误删
            if(guard) {
                handleEventWithGuard(receiveTime);
            }
            // guard为空情况，说明Channel的TcpConnection对象已经不存在了
        }
        else {
            handleEventWithGuard(receiveTime);
        }
    }

    //根据对应的事件执行回调操作
    void Channel::handleEventWithGuard(Tiny_muduo::TimeStamp receiveTime) {
        _eventHandling = true;
        //对端关闭事件
        //当TcpConnection对应channel，通过shutdown关闭写端，epoll触发EPOLLHUP
        // 当事件为挂起并没有可读事件时
        // POLLHUP 对方描述符挂起
        // 当客户端调用 close，服务器端接收到 POLLHUP 和 POLLIN
        if (_revents & POLLNVAL) // invalid polling request
        {
#ifdef USE_DEBUG
            LOG_WARN<< "Channel::handle_event() POLLNVAL";
#endif
        }

        if((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
            if(_closeCallback) {
                _closeCallback();
            }
        }

        // POLLRDHUP 对等方关闭连接
        // 发生错误或者描述符不可打开
        if(_revents & (POLLNVAL | POLLERR)) {
            LOG_ERROR << "the fd = " << fd() << "happended error thing";
            if(_errorCallback) {
                _errorCallback();
            }
        }
        //读事件
        if(_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
#ifdef USE_DEBUG
            LOG_DEBUG << "channel have read events, fd = " << fd();
#endif
            if(_readCallback) {
                _readCallback(receiveTime);
            }
        }
        //写事件
        if(_revents & EPOLLOUT) {
            if(_writeCallback) {
                _writeCallback();
            }
        }
        _eventHandling = false;
    }

    //在tcpConnection建立时会调用
    /**
     * TcpConnection中注册了Channel对应的回调函数，传入的回调函数均为TcpConnection
     * 对象的成员方法，因此可以说明一点就是：Channel的结束一定早于TcpConnection对象！
     * 此处用tie去解决TcpConnection和Channel的生命周期时长问题，从而保证了Channel对
     * 象能够在TcpConnection销毁前销毁。
     **/
    void Channel::tie(const std::shared_ptr<void> &obj) {
        _tie = obj;         // weak_ptr 指向 obj
        _tied = true;
    }

    // 调用这个函数之前确保调用disableAll
    void Channel::removeChannel() {
        assert(isNonEvent());
        _addedToLoop = false;
        _loop->removeChannel(this);
    }

    void Channel::updateChannel() {
        _addedToLoop = true;
        _loop->updateChannel(this);
    }


    std::string Channel::eventsToString(int fd, int ev)
    {
        std::ostringstream oss;
        oss << fd << ": ";
        if (ev & POLLIN)
            oss << "IN ";
        if (ev & POLLPRI)
            oss << "PRI ";
        if (ev & POLLOUT)
            oss << "OUT ";
        if (ev & POLLHUP)
            oss << "HUP ";
        if (ev & POLLRDHUP)
            oss << "RDHUP ";
        if (ev & POLLERR)
            oss << "ERR ";
        if (ev & POLLNVAL)
            oss << "NVAL ";
        return oss.str();
    }
}