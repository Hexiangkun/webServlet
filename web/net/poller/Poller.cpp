//
// Created by 37496 on 2024/2/21.
//

#include "net/poller/Poller.h"


namespace Tiny_muduo::net
{
    Poller::Poller(Tiny_muduo::net::EventLoop *loop)
                :_ownerLoop(loop)
    {

    }

    bool Poller::hasChannel(Tiny_muduo::net::Channel *channel) const {
        auto it = _channels.find(channel->fd());
        return it != _channels.end() && it->second == channel;
    }
}