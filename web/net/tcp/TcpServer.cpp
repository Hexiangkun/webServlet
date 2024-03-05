//
// Created by 37496 on 2024/2/11.
//

#include "net/tcp/TcpServer.h"
#include "net/tcp/TcpConnection.h"
#include "net/tcp/EventLoop.h"
#include "log/Log.h"


namespace Tiny_muduo::net
{
    static EventLoop *CheckLoopNotNull(EventLoop *loop)
    {
        if (loop == nullptr)
        {
            LOG_FATAL << "mainLoop is null!";
        }
        return loop;
    }
    TcpServer::TcpServer(Tiny_muduo::net::EventLoop *loop, const Tiny_muduo::net::InetAddress &listenAddr,
                         const std::string &name, Tiny_muduo::net::TcpServer::Option option)
                         :_mainLoop(CheckLoopNotNull(loop)),
                         _ip_port(listenAddr.toString()),
                         _name(name),
                         _acceptor(std::make_unique<Acceptor>(loop, listenAddr, option == kReusePort)),
                         _loopThreadPool(std::make_shared<EventLoopThreadPool>(loop, _name)),
                         _connectionCallback(defaultConnectionCallback),
                         _messageCallback(defaultMessageCallback),
                         _threadInitCallback(),
                         _writeCompleteCallback(),
                         _started(0), _nextConnId(1)
    {
        // 当有新用户连接时，Acceptor类中绑定的acceptChannel_会有读事件发生执行handleRead()调用TcpServer::newConnection回调
        _acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                      std::placeholders::_1, std::placeholders::_2));
    }

    TcpServer::~TcpServer() {
        _mainLoop->assertInLoopThread();
#ifdef USE_DEBUG
        LOG_TRACE << "TcpServer::~TcpServer [" << _name << "] destructing";
#endif
        for(auto& connection : _connections) {
            TcpConnectionPtr conn(connection.second);
            connection.second.reset();
            // 销毁连接
            conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
            conn.reset();
        }
    }

    // 开启服务器监听
    void TcpServer::start() {
        if(_started++ == 0) {    // 防止一个TcpServer对象被start多次
            _loopThreadPool->start(_threadInitCallback);
        }
        if(!_acceptor->listening()) {
            _mainLoop->runInLoop(std::bind(&Acceptor::listen, _acceptor.get()));
        }
    }

    void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
        _mainLoop->assertInLoopThread();
        EventLoop* ioLoop = _loopThreadPool->getNextLoop();
        // 提示信息
        char buf[64] = {0};
        snprintf(buf, sizeof buf, "-%s#%d", _ip_port.c_str(), _nextConnId);
        // 这里没有设置为原子类是因为其只在mainloop中执行 不涉及线程安全问题
        ++_nextConnId;
        // 新连接名字
        std::string connName = _name + buf;
#ifdef USE_DEBUG
        LOG_INFO << "TcpServer::newConnection [" << _name.c_str() << "] - new connection [" << connName.c_str() << "] from " << peerAddr.toString().c_str();
#endif
        InetAddress localAddr(sockops::getLocalAddr(sockfd));

        TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd,
                                                               localAddr, peerAddr));

        _connections[connName] = conn;

        conn->setConnectionCallback(_connectionCallback);
        conn->setMessageCallback(_messageCallback);
        conn->setWriteCompleteCallback(_writeCompleteCallback);

        conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

        //runInLoop函数会唤醒ioLoop所在的子线程，也就是让子线程不在阻塞在epoller_->poll上，
        // 解除阻塞后，子线程可以执行TcpConnection::connectEstablished函数
        ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    }

    void TcpServer::removeConnection(const TcpConnectionPtr &connection) {
        _mainLoop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, connection));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &connection) {
        _mainLoop->assertInLoopThread();
#ifdef USE_DEBUG
        LOG_INFO << "TcpServer::removeConnectionInLoop [" << _name.c_str() << "] - connection " << connection->name().c_str();
#endif
        _connections.erase(connection->name());
        EventLoop *ioLoop = connection->getLoop();
        ioLoop->queueInLoop(
                std::bind(&TcpConnection::connectDestroyed, connection));
    }
}