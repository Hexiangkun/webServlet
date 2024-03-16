//
// Created by 37496 on 2024/3/15.
//

#include "UserModel.h"
#include "sqlconn/MySqlConnectionv1.h"

bool UserModel::insert(User& user) {
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());


    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}


User UserModel::query(int id)
{
// 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }

    return User();
}

bool UserModel::updateState(User& user)
{
// 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

bool UserModel::offlineAll()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        return mysql.update(sql);
    }
    return false;
}