//
// Created by 37496 on 2024/2/24.
//

#include "RedisPool.h"
#include "log/Log.h"

namespace Tiny_muduo
{
    RedisPool *RedisPool::getInstance() {
        static RedisPool redisPool;
        return &redisPool;
    }

    RedisPool::RedisPool()
            :_port(config::GET_CONFIG<int>("redis.port", 6379)),
            _host(config::GET_CONFIG<std::string>("redis.host", "127.0.0.1")),
            _maxConnNum(config::GET_CONFIG<int>("redis.initSize", 8)),
            _sem(_maxConnNum)
    {
        init();
    }

    void RedisPool::init() {
        for(int i=0; i<_maxConnNum; i++) {
            RedisCache* redisCache = new RedisCache();
            redisCache->init(_host.data(), _port);
            _conn_que.push(redisCache);
        }
    }

    RedisCache *RedisPool::getRedisCache() {
        RedisCache* redisCache = nullptr;
        if(_conn_que.empty()) {
            LOG_INFO << "redis pool busy";
            return nullptr;
        }
        _sem.wait();
        {
            std::lock_guard<std::mutex> lockGuard(_mutex);
            redisCache = _conn_que.front();
            _conn_que.pop();
        }
        return redisCache;
    }

    void RedisPool::freeRedisCache(Tiny_muduo::RedisCache *redisCache) {
        assert(redisCache!= nullptr);
        std::lock_guard<std::mutex> lockGuard(_mutex);
        _conn_que.push(redisCache);
        _sem.notify();
    }
}