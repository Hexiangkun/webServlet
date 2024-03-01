//
// Created by 37496 on 2024/2/24.
//

#include "RedisCache.h"


namespace Tiny_muduo
{
    std::mutex RedisCache::_mutex;

    bool RedisCache::init(const char *host, int port) {
        _ctx = redisConnect(host, port);
        if(_ctx->err) {
            redisFree(_ctx);
            LOG_ERROR << "Connect to redisServer failed";
            return false;
        }
        LOG_INFO << "Connect to redisServer success";
        return true;
    }

    bool RedisCache::setKeyVal(const std::string& key, const std::string& val) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* r = (redisReply*) redisCommand(_ctx, "set %s %b", key.c_str(), val.c_str(), val.length());
        if(r == nullptr) {
            LOG_ERROR << "execute set " << key << "failure.";
            freeReplyObject(r);
            return false;
        }
        freeReplyObject(r);
        return true;
    }

    bool RedisCache::setKeyVal(const std::string& key, char *val, int sz) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* r = (redisReply*) redisCommand(_ctx, "set %s %b", key.c_str(), val, sz);
        if(r == nullptr) {
            LOG_ERROR << "execute set " << key << "failure.";
            freeReplyObject(r);
            return false;
        }
        freeReplyObject(r);
        return true;
    }

    std::string RedisCache::getKeyVal(const std::string &key) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* reply = (redisReply*) redisCommand(_ctx, "get %s", &key[0]);
        if(reply->type == REDIS_REPLY_NIL) {
            LOG_INFO << "key: " << key << " is nil.";
            freeReplyObject(reply);
            return noStr;
        }
        if(reply->type != REDIS_REPLY_STRING) {
            LOG_ERROR << "fail to get key : " << key;
            freeReplyObject(reply);
            return "";
        }
        std::string res(reply->str, reply->str + reply->len);
        freeReplyObject(reply);
        return res;
    }

    bool RedisCache::existKey(const std::string &key) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* reply = (redisReply*) redisCommand(_ctx, "exists %s", key.data());
        if(reply->type != REDIS_REPLY_INTEGER) {
            LOG_ERROR << "fail to execute exists " << key;
            freeReplyObject(reply);
            return false;
        }

        int res = reply->integer;
        freeReplyObject(reply);
        return res == 1;
    }

    bool RedisCache::delKey(const std::string &key) const {
        std::lock_guard<std::mutex> lk(_mutex);
        std::string command = "del ";
        command += key;
        redisReply* r = (redisReply*)redisCommand(_ctx, command.c_str());
        if (r->type != REDIS_REPLY_INTEGER) {
            LOG_ERROR << "Failed to execute command : "<< command;
            freeReplyObject(r);
            return false;
        }
        freeReplyObject(r);
        return true;
    }

    bool RedisCache::increase(const std::string &key) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        std::string command = "incr ";
        command += key;
        redisReply* reply = (redisReply*) redisCommand(_ctx, command.data());
        if(reply->type != REDIS_REPLY_INTEGER) {
            LOG_ERROR << "fail to execute command: " << command;
            freeReplyObject(reply);
            return false;
        }
        freeReplyObject(reply);
        return true;
    }

    bool RedisCache::listPush(const std::string& key, const std::string& val) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* reply = (redisReply*) redisCommand(_ctx, "lpush %s %s", key.c_str(), val.c_str(), val.length());
        if(reply->type == REDIS_REPLY_ERROR) {
            LOG_ERROR << "execute list push " << key << "failure.";
            freeReplyObject(reply);
            return false;
        }
        freeReplyObject(reply);
        return true;
    }

    std::vector<std::string> RedisCache::listRange(const std::string &key, int left, int right) const {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        redisReply* reply = (redisReply*) redisCommand(_ctx, "lrange %s %d %d", key.c_str(), left, right);
        if(reply->type == REDIS_REPLY_ERROR) {
            LOG_ERROR << "execute list push " << key << "failure.";
            freeReplyObject(reply);
            return {};
        }
        std::vector<std::string > res;
        for(size_t i = 0 ; i < reply->elements; i++) {
            res.push_back(std::move(reply->element[i]->str));
        }
        freeReplyObject(reply);
        return res;
    }

    bool RedisCache::flushDB() const {
        std::lock_guard<std::mutex> lk(_mutex);
        std::string command = "flushdb";
        redisReply* r = (redisReply*)redisCommand(_ctx, command.data());
        if (r->type != REDIS_REPLY_STATUS) {
            LOG_ERROR << "Failed to execute command : "<< command;
            freeReplyObject(r);
            return false;
        }
        freeReplyObject(r);
        return true;
    }
}