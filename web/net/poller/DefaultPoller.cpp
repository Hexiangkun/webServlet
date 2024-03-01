//
// Created by 37496 on 2024/2/21.
//

#include <cstdlib>
#include "net/poller/Poller.h"
#include "net/poller/EpollPoller.h"

namespace Tiny_muduo::net
{
    Poller *Poller::newDefaultPoller(Tiny_muduo::net::EventLoop *loop) {
        if(::getenv("USE_POLL")) {      // 生成poll实例
            return nullptr;
        }
        else {
            return new EpollPoller(loop); // 生成epoll实例
        }
    }
}