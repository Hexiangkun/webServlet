//
// Created by 37496 on 2024/2/24.
//

#include "RedisCache.h"
#include <utility>


namespace redis
{
    std::mutex RedisCache::_mutex;

    RedisCache::~RedisCache() {
        if(_ctx) {
            redisFree(_ctx);
        }
        if(_publish_context) {
            redisFree(_publish_context);
        }
        if(_subscribe_context) {
            redisFree(_subscribe_context);
        }
    }

    bool RedisCache::init(const char *host, int port) {
        _ctx = redisConnect(host, port);
        if(_ctx->err) {
            redisFree(_ctx);
            LOG_ERROR << "Connect to redisServer failed";
            return false;
        }
        return true;
    }

    bool RedisCache::connect(const char *host, int port) {
        _publish_context = redisConnect(host, port);
        if(!_publish_context) {
            LOG_ERROR << "connect to redis failed!";
            return false;
        }
        _subscribe_context = redisConnect(host, port);
        if(!_subscribe_context) {
            LOG_ERROR <<  "connect to redis failed!";
            return false;
        }

        //在单独的线程中，监听通道上的时间，有消息给业务层进行上报
        std::thread t([&](){
           observer_channel_message();
        });
        t.detach();

        return true;
    }

    bool RedisCache::publish(int channel, const std::string& message) {
        redisReply* reply = (redisReply*) redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
        if(reply == nullptr) {
            LOG_ERROR << "publish " << message << " failed!";
            return false;
        }
        freeReplyObject(reply);
        return true;
    }

    bool RedisCache::subscribe(int channel) {
        //subscribe命令本身会造成线程阻塞，等待通道里面发生消息，这里只做订阅通道，不接收通道消息
        //通道消息的接收专门在observer_channel_message函数中独立线程中运行
        //只负责发送命令，不阻塞接收redis server响应消息，否则和notify msg线程响应资源
        if(REDIS_ERR == redisAppendCommand(_subscribe_context, "SUBSCRIBE %d", channel)) {
            LOG_ERROR << "subscribe " << channel << " failed!";
            return false;
        }
        int done = 0;
        while (!done) {
            if(REDIS_ERR == redisBufferWrite(_subscribe_context, &done)) {
                LOG_ERROR << "redisBufferWrite error";
                return false;
            }
        }
        return true;
    }

    bool RedisCache::unsubscribe(int channel) {
        if(REDIS_ERR == redisAppendCommand(_subscribe_context, "UNSUBSCRIBE %d", channel)) {
            LOG_ERROR << "ubsubscribe " << channel << " failed";
            return false;
        }
        int done = 0;
        while(!done) {
            if(REDIS_ERR == redisBufferWrite(_subscribe_context, &done)) {
                LOG_ERROR << "redisBufferWrite " << channel << " failed" ;
                return false;
            }
        }
        return true;
    }

    void RedisCache::observer_channel_message() {
        redisReply* reply = nullptr;
        while(REDIS_OK == redisGetReply(_subscribe_context, (void**)&reply)) {
            if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
                // 订阅收到的消息是一个带三元素的数组
                _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
            }
            freeReplyObject(reply);
        }
    }

    void RedisCache::init_notify_handler(std::function<void(int, std::string)> fn) {
        this->_notify_message_handler = std::move(fn);
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