//
// Created by 37496 on 2024/2/21.
//

#include <thread>
#include "MySqlConnectionPool.h"
#include "config/Config.h"

namespace Tiny_muduo
{
    MySqlConnectionPool *MySqlConnectionPool::getInstance() {
        static MySqlConnectionPool pool;
        return &pool;
    }

    void MySqlConnectionPool::loadConfig() {
        _ip = Tiny_muduo::config::GET_CONFIG<std::string>("mysql.ip", "127.0.0.1");
        _user = Tiny_muduo::config::GET_CONFIG<std::string>("mysql.username", "root");
        _password = Tiny_muduo::config::GET_CONFIG<std::string>("mysql.password", "root");
        _dbName = Tiny_muduo::config::GET_CONFIG<std::string>("mysql.dbname", "default");
        _port = Tiny_muduo::config::GET_CONFIG<unsigned int>("mysql.port", 3306);
        _initSize = Tiny_muduo::config::GET_CONFIG<int>("mysql.initSize", 10);
        _maxSize = Tiny_muduo::config::GET_CONFIG<int>("mysql.maxSize", 512);
        _maxIdleTime = Tiny_muduo::config::GET_CONFIG<int>("mysql.maxIdleTime", 30);
        _timeout = Tiny_muduo::config::GET_CONFIG<int>("mysql.connectionTimeout", 100);
    }

    MySqlConnectionPool::MySqlConnectionPool() :_connectionCount(0){
        loadConfig();
        for(int i = 0; i < _initSize; i++) {
            MySqlConnection* connection = new MySqlConnection();
            connection->connect(_ip, _user,  _password, _dbName);
            connection->refreshAliveTime();
            _connections.push(connection);
            _connectionCount++;
        }

        std::thread produce(std::bind(&MySqlConnectionPool::produceConnection, this));
        produce.detach();
        // 不能使得主线程阻塞，要使得这两个线程分离
        std::thread scannerConnectionTime(std::bind(&MySqlConnectionPool::scanConnectionTime, this));
        scannerConnectionTime.detach();
    }

    MySqlConnectionPool::~MySqlConnectionPool() {
        while(!_connections.empty()) {
            MySqlConnection* conn = _connections.front();
            _connections.pop();
            delete conn;
        }
    }

    void MySqlConnectionPool::produceConnection() {
        while(true) {
            std::unique_lock<std::mutex> lock(_mutex);

            while(!_connections.empty()) {
                _cond.wait(lock);
            }

            if(_connectionCount < _maxSize) {
                MySqlConnection* connection = new MySqlConnection();
                connection->connect(_ip, _user, _password, _dbName);
                connection->refreshAliveTime();
                _connections.push(connection);
                ++_connectionCount;
            }

            _cond.notify_all();         // 生产完毕，通知可以使用
        }
    }

    void MySqlConnectionPool::scanConnectionTime() {
        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
            std::unique_lock<std::mutex> lock(_mutex);
            // 扫描队列里全部的节点，将超过最大允许存活时间的去掉
            while(_connectionCount > _initSize ) {
                MySqlConnection* connection = _connections.front();

                if(connection->getAliveTime() >= _maxIdleTime * 1000) {
                    _connections.pop();
                    --_connectionCount;
                    delete connection;
                }
                else {  //头的连接没有超时则后面的肯定不会超时
                    break;
                }
            }
        }
    }

    std::shared_ptr<MySqlConnection> MySqlConnectionPool::getConnection() {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_connections.empty()) {       // 等待队列为空

            // 唤醒条件：1. notify_one 等唤醒；2. 等待时长结束，返回一个状态，名称为 timeout
            if(std::cv_status::timeout == _cond.wait_for(lock, std::chrono::milliseconds(_timeout))) {
                if(_connections.empty()) {
                    return nullptr;
                }
            }
        }

        // 使用智能指针，并重写删除器，将原始指针归还到队列里面而不是析构
        std::shared_ptr<MySqlConnection> p(_connections.front(), [this](MySqlConnection* tmp){
            std::unique_lock<std::mutex> lock(_mutex);
            tmp->refreshAliveTime();
            _connections.push(tmp);
        });

        _connections.pop();
        _cond.notify_all();
        return p;
    }
}