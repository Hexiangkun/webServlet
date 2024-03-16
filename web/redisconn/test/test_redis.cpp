
#include "redisconn/RedisCache.h"
#include "redisconn/RedisPool.h"
#include "redisconn/RedisConnRAII.h"
#include <iostream>


void test_redisCache() {
    redis::RedisCache* rc = new redis::RedisCache();
    rc->init("127.0.0.1", 6379);
    bool res = rc->setKeyVal("yunfei", "22");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << "set success!" << std::endl;
    std::string str = rc->getKeyVal("yunfei");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << str << std::endl;

    rc->listPush("lan", "Java");
    rc->listPush("lan", "Go");
    rc->listPush("lan", "C++");

    auto v = rc->listRange("lan", 0, 100);
    for (auto i : v) {
        std::cout << i << std::endl;
    }

    res = rc->existKey("yunfei");

    res = rc->delKey("yunfei");
    if (!res) {
        std::cout << "fail del key!" << std::endl;
        return;
    }
    std::cout << "del key success!" << std::endl;

    rc->getKeyVal("y");

    res = rc->flushDB();
    if (!res) {
        std::cout << "flush fail!" << std::endl;
        return;
    }
    std::cout << "flush success!" << std::endl;
}


void test_redisPool()
{
    redis::RedisCache* rc = nullptr;
    redis::RedisPool::getInstance()->init();
    redis::RedisConnRAII(&rc, redis::RedisPool::getInstance());

    bool res = rc->setKeyVal("hxk", "22");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << "set success!" << std::endl;
};


int main() {
    test_redisCache();
    test_redisPool();
}

