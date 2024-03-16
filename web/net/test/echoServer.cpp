#include "net/tcp/EventLoop.h"
#include "net/tcp/TcpServer.h"
#include "net/tcp/TcpConnection.h"
#include "log/Log.h"

using namespace Tiny_muduo::net;

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
            : server_(loop, addr, name)
            , loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, std::placeholders::_1));

        server_.setMessageCallback(
                std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的subloop线程数量
        // server_.setThreadNum(3);
    }

    void start()
    {
        server_.start();
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO << "Connection UP : " << conn->peerAddress().toString().c_str();
        }
        else
        {
            LOG_INFO << "Connection DOWN : " << conn->peerAddress().toString().c_str();
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Tiny_muduo::Buffer *buf, Tiny_muduo::TimeStamp time)
    {
        std::string msg = buf->retrieveAllToStr();
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data received at " << time.toFormatString();
        conn->send(msg);
        // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{

    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress addr(9999);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();

    return 0;
}