//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_REDISPOOL_H
#define WEBSERVER_REDISPOOL_H

#include <queue>
#include "RedisCache.h"
#include "base/Mutex.h"
#include "config/Config.h"

namespace Tiny_muduo
{
    class RedisPool
    {
    public:
        static RedisPool* getInstance();

        void init();
        RedisCache* getRedisCache();
        void freeRedisCache(RedisCache* redisCache);
    private:
        RedisPool();
        ~RedisPool() = default;
    private:
        std::queue<RedisCache*> _conn_que;
        int _maxConnNum;
        Semaphore _sem;
        std::mutex _mutex;
        std::string _host;
        int _port;
    };
}



#endif //WEBSERVER_REDISPOOL_H
