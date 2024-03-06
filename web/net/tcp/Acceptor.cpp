//
// Created by 37496 on 2024/2/10.
//

#include <unistd.h>
#include <fcntl.h>
#include "net/tcp/Acceptor.h"
#include "log/Log.h"
#include "net/tcp/InetAddress.h"
#include "net/tcp/EventLoop.h"

namespace Tiny_muduo::net
{
    Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort)
            :_loop(loop),
            _acceptSocket(sockops::createNonBlockSocket()),
             _acceptChannel(_loop, _acceptSocket.sockfd()),
             _listening(false),
             _idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
    {
        assert(_idleFd >= 0);
#ifdef USE_DEBUG
        LOG_DEBUG << "Acceptor create nonblocking socket, [fd = " << _acceptChannel.fd() << "]";
#endif
        _acceptSocket.setReuseAddr(true);
        _acceptSocket.setReusePort(reusePort);
        _acceptSocket.bind(listenAddr);

        /**
         * TcpServer::start() => Acceptor.listen => 绑定到epoll上去，监听读事件的到来，也就是新的连接
         * 有新用户的连接，需要执行一个回调函数handleNewConnection => TcpServer::newConnection
         * (accept => connfd => 打包成Channel => 唤醒subloop)
         * 因此向封装了acceptSocket_的channel注册回调函数
         * baseloop监听到有事件发生 => acceptChannel_(listenfd) => 执行该回调函数
         */
        _acceptChannel.setReadCallback(std::bind(&Acceptor::handleNewConnection, this));
    }

    Acceptor::~Acceptor() {
        _acceptChannel.disableAll();         // 把从Poller中感兴趣的事件删除
        _acceptChannel.removeChannel();      // 调用EventLoop->delChannel => Poller->delChannel 把Poller的ChannelMap对应的部分删除
        ::close(_idleFd);
    }

    void Acceptor::listen() {
        _loop->assertInLoopThread();
        _listening = true;
        _acceptSocket.listen();
        _acceptChannel.enableReading();         // 将acceptChannel的读事件注册到poller
    }

    // 当epoll监听到listenfd有事件发生了，就是有新用户连接了
    void Acceptor::handleNewConnection() {
        _loop->assertInLoopThread();

        InetAddress peerAddr(0);

        int connfd = _acceptSocket.accept(&peerAddr);   // 接受新连接

        if(connfd >= 0) {       // 确实有新连接到来

            // _newConnectionCallback = TcpServer::newConnection
            if(_newConnectionCallback) {
                _newConnectionCallback(connfd, peerAddr);   // 轮询找到subLoop 唤醒并分发当前的新客户端的Channel
            }
            else {
                sockops::close(connfd);
            }
        }
        else {
            LOG_ERROR << "accept() failed";

            // 当前进程的fd已经用完了
            if (errno == EMFILE)//文件描述符太多了
            {
                ::close(_idleFd);
                _idleFd = ::accept(_acceptSocket.sockfd(), nullptr, nullptr);
                ::close(_idleFd);
                _idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
                LOG_ERROR << "sockfd reached limit";
            }
        }
    }
}