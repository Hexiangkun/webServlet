//
// Created by 37496 on 2024/2/21.
//

#include "MySqlConnectionv1.h"
#include "config/Config.h"

namespace SqlConn
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

    MYSQL *MySqlConnectionv1::getConnection() {
        return _conn;
    }

    bool MySqlConnectionv1::connect(const std::string &ip, const std::string &user, const std::string &password,
                                    const std::string &dbName, unsigned int port) {

        MYSQL* p = mysql_real_connect(_conn, "127.0.0.1",
                                             "root",
                                            "root",
                                            "yourdb",
                                            port, nullptr, 0);
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