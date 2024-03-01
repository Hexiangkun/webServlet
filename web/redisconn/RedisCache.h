//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_REDISCACHE_H
#define WEBSERVER_REDISCACHE_H

#include "log/Log.h"
#include <hiredis/hiredis.h>
#include <string>
#include <mutex>

namespace Tiny_muduo
{
    const std::string noStr = "nil";
    class RedisCache
    {
    public:
        RedisCache() = default;
        ~RedisCache() = default;

        bool init(const char* host, int port);

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
    private:
        redisContext* _ctx;
        static std::mutex _mutex;
    };
}


#endif //WEBSERVER_REDISCACHE_H
