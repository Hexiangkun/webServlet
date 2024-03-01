//
// Created by 37496 on 2024/2/21.
//

#ifndef WEBSERVER_MYSQLCONNECTIONPOOL_H
#define WEBSERVER_MYSQLCONNECTIONPOOL_H

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "base/Noncopyable.h"
#include "MySqlConnection.h"

namespace Tiny_muduo
{
    class MySqlConnectionPool : public Noncopyable
    {
    public:
        static MySqlConnectionPool* getInstance();   //单例模式

        std::shared_ptr<MySqlConnection> getConnection();       //消费者获取一个连接

        ~MySqlConnectionPool();

    private:
        MySqlConnectionPool();
        void produceConnection();       //生产者生产一个连接
        void scanConnectionTime();      //定时器处理函数

        void loadConfig();

    private:
        std::string _ip;                //数据库的ip
        std::string _user;              //数据库的用户名
        std::string _password;          //数据库的密码
        std::string _dbName;            //数据库使用的库名
        unsigned int _port;             //数据库使用的端口号
        int _initSize;                  //数据库连接池的初始线程数量
        int _maxSize;                   //数据库连接池的最大线程数量
        int _maxIdleTime;               //数据库连接池各线程最大空闲时间
        int _timeout;                   //数据库连接池获取连接的超时时间
        std::atomic<int> _connectionCount;          //保存现有连接数量
        std::queue<MySqlConnection*> _connections;  //用于存放连接池中所有连接

        std::mutex _mutex;                          //队列锁，维护线程安全
        std::condition_variable _cond;              //条件变量，用于连接生产者和消费者
    };
}


#endif //WEBSERVER_MYSQLCONNECTIONPOOL_H
