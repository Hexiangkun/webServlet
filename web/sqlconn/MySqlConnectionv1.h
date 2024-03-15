//
// Created by 37496 on 2024/2/21.
//

#ifndef WEBSERVER_MYSQLCONNECTIONV1_H
#define WEBSERVER_MYSQLCONNECTIONV1_H

#include <mysql/mysql.h>
#include <ctime>
#include <string>

namespace SqlConn
{
    class MySqlConnectionv1
    {
    public:
        MySqlConnectionv1();
        ~MySqlConnectionv1();
        
        //连接数据库
        bool connect(std::string ip, std::string user, std::string password,
                     std::string dbName, unsigned int port = 3306);
        
        //更新操作 insert | delete | update
        bool update(const std::string& sql);
        
        //查询操作
        MYSQL_RES* query(const std::string& sql);

        MYSQL* getConnection();
        
        void refreshAliveTime() { _aliveTime = clock(); }
        clock_t getAliveTime() const { return clock() - _aliveTime; }   // 返回存活的时间

    private:
        MYSQL* _conn;
        MYSQL_RES* _result;
        MYSQL_ROW _row;
        clock_t _aliveTime;    //记录进入空闲状态后的起始时间点
    };
}


#endif //WEBSERVER_MYSQLCONNECTIONV1_H
