//
// Created by 37496 on 2024/2/1.
//

#ifndef WEBSERVER_EPOLLPOLLER_H
#define WEBSERVER_EPOLLPOLLER_H

#include <sys/epoll.h>
#include "net/poller/Poller.h"

namespace Tiny_muduo
{
    namespace net
    {
        class EpollPoller : public Poller
        {
        private:
            using EventList = std::vector<epoll_event>;
        public:
            using ChannelList = std::vector<Channel*> ;

            EpollPoller(EventLoop* loop);
            ~EpollPoller() override;

            TimeStamp poll(int timeoutMS, ChannelList* activeChannel) override;
            void removeChannel(Channel*) override;
            void updateChannel(Channel*) override;

        private:
            void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
            void update(int operation, Channel* channel);       //更新channel，本质调用epoll_ctl

        private:
            static const int kInitEventListSize = 16;       // 默认监听事件数量
            int _epollFd;                                   //epoll_create在内核创建的fd
            EventList _activeEvents;                        //触发的事件描述符对应的集合
        };
    }
}



#endif //WEBSERVER_EPOLLPOLLER_H
