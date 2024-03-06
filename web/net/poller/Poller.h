//
// Created by 37496 on 2024/2/21.
//

#ifndef WEBSERVER_POLLER_H
#define WEBSERVER_POLLER_H

#include <vector>
#include <unordered_map>
#include "net/tcp/Channel.h"
#include "base/TimeStamp.h"
#include "base/Noncopyable.h"

namespace Tiny_muduo::net
{
    class Poller : public Noncopyable
    {
    public:
        using ChannelList = std::vector<Channel*>;

        Poller(EventLoop* loop);
        virtual ~Poller() = default;

        //返回epoll_wait的调用时间
        virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannel) = 0;

        //channel->updateChannel --> eventLoop->updateChannel --> poll->updateChannel
        virtual void updateChannel(Channel* channel) = 0;
        virtual void removeChannel(Channel* channel) = 0;

        bool hasChannel(Channel* channel) const;            //判断channel是否注册到poller中

        static Poller* newDefaultPoller(EventLoop* loop);

    protected:
        std::unordered_map<int, Channel*> _channels;        //sockfd -> Channel

    private:
        EventLoop* _ownerLoop;                              //定义poller所属的事件循环EventLoop
    };
}


#endif //WEBSERVER_POLLER_H
