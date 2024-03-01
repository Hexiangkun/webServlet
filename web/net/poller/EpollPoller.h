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
        static constexpr auto kServerEvent = EPOLLRDHUP;
        static constexpr auto kConnectionEvent = EPOLLET | EPOLLONESHOT | EPOLLRDHUP;

        class EpollPoller : public Poller
        {
        private:
            using EventList = std::vector<epoll_event>;
        public:
            using ChannelList = std::vector<Channel*> ;

            EpollPoller(EventLoop* loop);
            ~EpollPoller() override;


            TimeStamp poll(int timeoutMS, ChannelList* activeChannel) override;

            //通过channel添加、修改、删除红黑树
            void addChannel(Channel*);
            void modChannel(Channel*);
            TimeStamp poll(ChannelList&, int timeoutMs = -1);

            void removeChannel(Channel*) override;
            void updateChannel(Channel*) override;

        private:
            void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
            void fillActiveChannels(int numEvents, ChannelList& activeChannels);   //填写活跃连接

            void update(int operation, Channel* channel);       //更新channel，本质调用epoll_ctl

        private:
            static const int kInitEventListSize = 16;       // 默认监听事件数量
            int _epollFd;
            EventList _activeEvents;                        //触发的事件描述符对应的集合
        };
    }
}



#endif //WEBSERVER_EPOLLPOLLER_H
