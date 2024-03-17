//
// Created by 37496 on 2024/3/16.
//

#ifndef WEBSERVER_CHATSERVICE_H
#define WEBSERVER_CHATSERVICE_H

#include "base/TimeStamp.h"
#include "net/tcp/TcpConnection.h"
#include "nlohmann/json.h"
#include "redisconn/RedisCache.h"
#include "chat/control/UserModel.h"
#include "chat/control/OfflineMsgModel.h"
#include "chat/control/FriendModel.h"
#include "chat/control/GroupModel.h"
#include <functional>
#include <mutex>

typedef std::function<void(const Tiny_muduo::net::TcpConnection::_ptr& , nlohmann::json& , Tiny_muduo::TimeStamp)> msgHandler;


class ChatService
{
public:
    //单例模式
    static ChatService* getInstance();
    //暴露给chatServer接口，用int整型匹配对应的处理函数
    msgHandler getHandler(int msgId);

    //登录
    void login(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);
    //注册
    void regis(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    //处理客户端异常退出
    void clientCloseException(const Tiny_muduo::net::TcpConnection::_ptr& connection);
    //处理服务器宕机
    void serverCloseException();

    void oneChat(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    void addFriend(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    void createGroup(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    void addToGroup(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    void groupChat(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);

    void logout(const Tiny_muduo::net::TcpConnection::_ptr& connection, nlohmann::json& js, Tiny_muduo::TimeStamp stamp);


private:
    ChatService();

    std::unordered_map<int, msgHandler > _msgHandlerMap;

    std::unordered_map<int, Tiny_muduo::net::TcpConnectionPtr > _userConnMap;

    std::mutex _connMtx;

    UserModel _userModel;

    OfflineMsgModel _offlineMsgModel;

    FriendModel _friendModel;

    GroupModel _groupModel;

    redis::RedisCache _redisModel;

    //订阅频道有更新，获取消息
    void redisNotifyHandler(int ,std::string);
};


#endif //WEBSERVER_CHATSERVICE_H
