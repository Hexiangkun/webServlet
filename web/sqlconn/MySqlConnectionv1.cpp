//
// Created by 37496 on 2024/2/21.
//

#include "MySqlConnectionv1.h"


namespace Tiny_muduo
{
    MySqlConnectionv1::MySqlConnectionv1() : _result(nullptr), _row(nullptr)
    {
        _conn = mysql_init(nullptr);
    }

    MySqlConnectionv1::~MySqlConnectionv1() {
        if(!_conn) {
            mysql_close(_conn);
        }
    }

    bool MySqlConnectionv1::connect(std::string ip, std::string user, std::string password, std::string dbName,
                                    unsigned int port) {
        MYSQL* p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(),
                                      dbName.c_str(), port, nullptr, 0);
        if(p == nullptr) {
            return false;
        }
        return true;
    }

    bool MySqlConnectionv1::update(const std::string &sql) {
        if(mysql_query(_conn, sql.c_str())) {
            return false;
        }
        return true;
    }

    MYSQL_RES *MySqlConnectionv1::query(const std::string &sql) {
        if(mysql_query(_conn, sql.c_str())) {
            return nullptr;
        }
        return mysql_use_result(_conn);
    }
}