//
// Created by 37496 on 2024/3/16.
//

#include "ChatServer.h"
#include "chat/service/ChatService.h"

using namespace std::placeholders;
ChatServer::ChatServer(Tiny_muduo::net::EventLoop *loop, const Tiny_muduo::net::InetAddress &listenAddr,
                       const std::string &nameArg) : _server(loop, listenAddr, nameArg){
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

}

void ChatServer::setThreadNum(int num) {
    _server.setThreadNum(num);
}

void ChatServer::start() {
    _server.start();
}

void ChatServer::onConnection(const Tiny_muduo::net::TcpConnection::_ptr &connection) {
    if(connection->disconnected()) {
        ChatService::getInstance()->clientCloseException(connection);
        connection->shutdown();
    }
}

void ChatServer::onMessage(const Tiny_muduo::net::TcpConnectionPtr &connection, Tiny_muduo::Buffer *buffer,
                           Tiny_muduo::TimeStamp stamp) {
    std::string msg = buffer->retrieveAllToStr();
    LOG_INFO << connection->name() << " recived " << msg
             << " | at time: " << stamp.toFormatString();

    nlohmann::json js = nlohmann::json::parse(msg);
    auto handler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
    handler(connection, js, stamp);
}