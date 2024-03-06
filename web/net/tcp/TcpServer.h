//
// Created by 37496 on 2024/2/11.
//

#ifndef WEBSERVER_TCPSERVER_H
#define WEBSERVER_TCPSERVER_H

#include <functional>
#include "base/Noncopyable.h"
#include "net/tcp/EventLoop.h"
#include "net/tcp/Acceptor.h"
#include "net/tcp/EventLoopThreadPool.h"
#include "net/tcp/Callback.h"

namespace Tiny_muduo
{
    namespace net
    {
        class TcpServer : public Noncopyable
        {
        public:
            using ThreadInitCallback = std::function<void(EventLoop*)>;
            enum Option
            {
                kNoReusePort,
                kReusePort
            };

            TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                      const std::string& name, Option option = kNoReusePort);
            ~TcpServer();

            void start();

            void setThreadInitCallback(const ThreadInitCallback &cb) { _threadInitCallback = cb; }
            void setConnectionCallback(const ConnectionCallback &cb) { _connectionCallback = cb; }
            void setMessageCallback(const MessageCallback &cb) { _messageCallback = cb; }
            void setWriteCompleteCallback(const WriteCompleteCallback &cb) { _writeCompleteCallback = cb; }

            void setThreadNum(int numThreads) { assert(0 <= numThreads); _loopThreadPool->setThreadNum(numThreads); }       //设置底层subLoop的个数

            EventLoop* getMainLoop() const { return _mainLoop; }
            const std::string& name() const { return _name; }
            const std::string& ipPort() const { return _ip_port; }

            std::shared_ptr<EventLoopThreadPool>& threadPool() { return _loopThreadPool; }

        private:
            void newConnection(int sockFd, const InetAddress& peerAddr);

            void removeConnection(const TcpConnectionPtr& connection);

            void removeConnectionInLoop(const TcpConnectionPtr& connection);

        private:
            using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr >;
            EventLoop* _mainLoop;                   //主事件循环
            const std::string _ip_port;             //保存传入的IP和Port
            const std::string _name;                //TcpServer名字

            std::unique_ptr<Acceptor> _acceptor;    //Acceptor负责监视新的连接

            std::shared_ptr<EventLoopThreadPool> _loopThreadPool;       //线程池

            ConnectionCallback _connectionCallback;
            MessageCallback _messageCallback;
            WriteCompleteCallback _writeCompleteCallback;
            ThreadInitCallback _threadInitCallback;                     //loop线程初始化的回调函数

            std::atomic_int _started;

            int _nextConnId;                            //连接索引
            ConnectionMap _connections;                 //保存所有连接
        };
    }
}

#endif //WEBSERVER_TCPSERVER_H
