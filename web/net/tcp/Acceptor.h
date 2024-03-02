//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_ACCEPTOR_H
#define WEBSERVER_ACCEPTOR_H

#include <functional>
#include "net/tcp/Socket.h"
#include "net/tcp/Channel.h"
#include "base/Noncopyable.h"

namespace Tiny_muduo
{
    namespace net
    {
        /**
         * 主要封装了listenfd对应的Channel，它的任务就是创建套接字，绑定地址，转变为监听状态，接受连接
         * 只负责接受连接，不负责tcp连接的分配
         * Acceptor用于接受客户端连接，通过设置回调函数通知使用者, 运行在mainLoop中
         * Acceptor在TcpServer内部使用，由TcpServer控制它的生命周期
         * TcpServer发现Acceptor有一个新连接，则将此channel分发给一个subLoop
        */
        class EventLoop;
        class InetAddress;
        class Acceptor : public Noncopyable
        {
        public:
            using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

            Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort = true);
            ~Acceptor();

            void listen();

            void setNewConnectionCallback(const NewConnectionCallback& cb) { _newConnectionCallback = cb; }
            bool listening() const { return _listening; }

        private:
            void handleNewConnection();

            EventLoop* _loop;
            Socket _acceptSocket;           //监听的套接字
            Channel _acceptChannel;         //监听的通道
            NewConnectionCallback _newConnectionCallback;   //accept之后回调
            bool _listening;                //是否正在监听标志
            int _idleFd;                    //占位fd,用于满fd的情况
        };
    }
}


#endif //WEBSERVER_ACCEPTOR_H
