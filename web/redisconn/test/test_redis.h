//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_TEST_REDIS_H
#define WEBSERVER_TEST_REDIS_H

#include "redisconn/RedisCache.h"
#include "redisconn/RedisPool.h"
#include "redisconn/RedisConnRAII.h"
#include <iostream>
using namespace Tiny_muduo;

void test_redisCache() {
    RedisCache* rc = new RedisCache();
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
    RedisCache* rc = nullptr;
    RedisPool::getInstance()->init();
    RedisConnRAII(&rc, RedisPool::getInstance());

    bool res = rc->setKeyVal("yunfei", "22");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << "set success!" << std::endl;
};

void test_main() {
    test_redisCache();
    test_redisPool();
}

#endif //WEBSERVER_TEST_REDIS_H
