//
// Created by 37496 on 2024/3/16.
//

#ifndef WEBSERVER_CHATSERVER_H
#define WEBSERVER_CHATSERVER_H

#include "net/tcp/EventLoop.h"
#include "net/tcp/InetAddress.h"
#include "net/tcp/TcpConnection.h"
#include "net/tcp/TcpServer.h"

class ChatServer {
public:
    ChatServer(Tiny_muduo::net::EventLoop* loop, const Tiny_muduo::net::InetAddress& listenAddr, const std::string& nameArg);

    ~ChatServer() = default;

    void start();

    void setThreadNum(int num);

private:
    void onConnection(const Tiny_muduo::net::TcpConnection::_ptr& connection);
    void onMessage(const Tiny_muduo::net::TcpConnectionPtr& connection, Tiny_muduo::Buffer* buffer, Tiny_muduo::TimeStamp stamp);

    Tiny_muduo::net::TcpServer _server;
};


#endif //WEBSERVER_CHATSERVER_H
