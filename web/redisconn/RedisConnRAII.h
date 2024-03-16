//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_REDISCONNRAII_H
#define WEBSERVER_REDISCONNRAII_H

#include "RedisPool.h"

namespace redis
{
    class RedisConnRAII
    {
    public:
        RedisConnRAII(RedisCache** pRedisCache, RedisPool* redisPool) {
            assert(redisPool);
            *pRedisCache = redisPool->getRedisCache();
            _redisCache = *pRedisCache;
            _redisPool = redisPool;
        }

        ~RedisConnRAII()
        {
            if(_redisCache) {
                _redisPool->freeRedisCache(_redisCache);
            }
        }

    private:
        RedisCache* _redisCache;
        RedisPool* _redisPool;
    };
}

#endif //WEBSERVER_REDISCONNRAII_H
