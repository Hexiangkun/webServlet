//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H


#include <cstdint>
#include <memory>
#include <functional>
#include "base/Noncopyable.h"
#include "base/TimeStamp.h"


namespace Tiny_muduo
{
    namespace net
    {
        class EventLoop;

        /**
         * 管理fd
         * 注册fd在epoll上的事件
         * 事件发生后，执行回调函数，处理事件
         * */
        class Channel : public Noncopyable
        {
        public:
            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;

        public:
            using _ptr = std::shared_ptr<Channel>;
            using EventCallback = std::function<void()>;
            typedef std::function<void(TimeStamp)> ReadEventCallback;

            Channel(EventLoop* loop, int fd);
            ~Channel() ;

            // fd得到Poller通知以后 处理事件 handleEvent在EventLoop::loop()中调用
            void handleEvents(TimeStamp receiveTime);

            void setReadCallback(ReadEventCallback cb) { _readCallback = std::move(cb); }
            void setWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }
            void setCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }
            void setErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }

            // 设置fd相应的事件状态，update()其本质调用epoll_ctl
            void enableReading() { _events |= kReadEvent; updateChannel(); }
            void disableReading() { _events &= ~kReadEvent; updateChannel(); }
            void enableWriting() { _events |= kWriteEvent; updateChannel(); }
            void disableWriting() { _events &= ~kWriteEvent; updateChannel(); }
            void disableAll() { _events = kNoneEvent; updateChannel(); }

            // 返回fd当前的事件状态
            bool isWriting() const { return _events & kWriteEvent; }
            bool isReading() const { return _events & kReadEvent; }
            bool isNonEvent() const { return _events == kNoneEvent; }

            int index() { return _index; }
            void setIndex(int idx) { _index = idx; }

            EventLoop* ownerLoop() { return _loop; }
            int fd() const { return _fd; }

            int events() const { return _events; }
            void setEvents(int events) { _events = events; }
            void setRevents(int events) { _revents = events; }
            int revents() const { return _revents; }

            // for debug
            std::string reventsToString() const;
            std::string eventsToString() const;

            // TODO:防止当 channel 执行回调函数时被被手动 remove 掉
            void tie(const std::shared_ptr<void>&);

            void removeChannel();
        private:
            void updateChannel();
            void handleEventWithGuard(TimeStamp receiveTime);
            static std::string eventsToString(int fd, int ev);
        private:
            const int _fd;              //fd，Poller监听的对象
            int _events;                //注册fd感兴趣的事件
            int _revents;               //poll上发生的事件
            int _index;                 //在poll上注册的情况|knew\kadd\kdelete

            bool _eventHandling;

            std::weak_ptr<void> _tie;   //弱指针指向TcpConnection(必要时升级为shared_ptr)
            bool _tied;                 //标志此Channel是否调用过Channel::tie方法

            EventLoop* _loop;           //当前channel属于的EventLoop
            bool _addedToLoop;

            ReadEventCallback _readCallback;        //TcpConnection::handleRead
            EventCallback _writeCallback;           //TcpConnection::handleWrite
            EventCallback _closeCallback;           //TcpConnection::handleClose
            EventCallback _errorCallback;           //TcpConnection::handleError
        };
    }
}



#endif //WEBSERVER_CHANNEL_H
