//
// Created by 37496 on 2024/2/21.
//

#include <thread>
#include "MySqlConnectionPool.h"
#include "config/Config.h"

namespace SqlConn
{
    MySqlConnectionPool *MySqlConnectionPool::getInstance() {
        static MySqlConnectionPool pool;
        return &pool;
    }

    void MySqlConnectionPool::loadConfig() {
        _ip = config::GET_CONFIG<std::string>("mysql.ip", "127.0.0.1");
        _user = config::GET_CONFIG<std::string>("mysql.username", "root");
        _password = config::GET_CONFIG<std::string>("mysql.password", "root");
        _dbName = config::GET_CONFIG<std::string>("mysql.dbname", "default");
        _port = config::GET_CONFIG<unsigned int>("mysql.port", 3306);
        _initSize = config::GET_CONFIG<int>("mysql.initSize", 10);
        _maxSize = config::GET_CONFIG<int>("mysql.maxSize", 512);
        _maxIdleTime = config::GET_CONFIG<int>("mysql.maxIdleTime", 30);
        _timeout = config::GET_CONFIG<int>("mysql.connectionTimeout", 100);
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
/**
 *  程序执行到wait时会先在此阻塞，然后解锁互斥量，那么其他线程在拿到锁之后就会往下运行
 *  当运行到notify_one函数的时候，就会唤醒wait函数来不断尝试获取互斥锁，如果获取不到，就等待获取
 *  获取到锁之后，向下执行
 * */
            while(!_connections.empty()) {
                _cond.wait(lock);
            }

            //没有到达上限，继续创建新的连接
            if(_connectionCount < _maxSize) {
                MySqlConnection* connection = new MySqlConnection();
                connection->connect(_ip, _user, _password, _dbName);
                connection->refreshAliveTime();
                _connections.push(connection);
                ++_connectionCount;
            }

            _cond.notify_all();         // 通知消费者线程，可以连接
        }
    }

    void MySqlConnectionPool::scanConnectionTime() {
        while(true) {
            //模拟定时效果
            std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
            std::unique_lock<std::mutex> lock(_mutex);
            // 扫描队列里全部的节点，将超过最大允许存活时间的去掉
            while(_connectionCount >= _initSize ) {
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
        while(_connections.empty()) {       // 等待队列为空,阻塞等待

            // 唤醒条件：1. notify_one 等唤醒；2. 等待时长结束，返回一个状态，名称为 timeout
            if(std::cv_status::timeout == _cond.wait_for(lock, std::chrono::milliseconds(_timeout))) {
                if(_connections.empty()) {
                    return nullptr;
                }
            }
        }
        //阻塞途中被唤醒或队列不为空，直接获取连接

        // 使用智能指针，并重写删除器，将原始指针归还到队列里面而不是析构
        std::shared_ptr<MySqlConnection> p(_connections.front(), [this](MySqlConnection* tmp){
            std::unique_lock<std::mutex> lock(_mutex);
            tmp->refreshAliveTime();
            _connections.push(tmp);
        });

        _connections.pop();
        _cond.notify_all();     //消费完连接以后，通知生产者线程检查一下，若队列为空，赶紧生产
        return p;
    }
}