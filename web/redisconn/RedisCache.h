//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_REDISCACHE_H
#define WEBSERVER_REDISCACHE_H

#include "log/Log.h"
#include <hiredis/hiredis.h>
#include <string>
#include <mutex>

namespace redis
{
    const std::string noStr = "nil";
    class RedisCache
    {
    public:
        RedisCache() = default;
        ~RedisCache() ;

        bool init(const char* host, int port);
        bool connect(const char* host, int port);

        bool setKeyVal(const std::string& key, const std::string& val) const;
        bool setKeyVal(const std::string& key, char* val, int sz) const;

        std::string getKeyVal(const std::string& key) const;

        bool existKey(const std::string& key) const;

        bool delKey(const std::string& key) const;

        bool increase(const std::string& key) const;

        //将指定值推入 Redis 列表（List）的左侧
        bool listPush(const std::string& key, const std::string& val) const;

        std::vector<std::string> listRange(const std::string& key, int left, int right) const;

        bool flushDB() const;

        //向redis指定的通道channel发布消息
        bool publish(int channel, const std::string& message);

        //向redis指定通道订阅消息
        bool subscribe(int channel);

        //向redis指定通道取消订阅消息
        bool unsubscribe(int channel);

        //在独立线程中接受订阅通道中的消息
        void observer_channel_message();

        //初始化向业务层上报通道消息的回调函数
        void init_notify_handler(std::function<void(int, std::string)> fn);


    private:
        redisContext* _ctx;

        redisContext* _publish_context;
        redisContext* _subscribe_context;
        std::function<void(int, std::string)> _notify_message_handler;
        static std::mutex _mutex;
    };
}


#endif //WEBSERVER_REDISCACHE_H
