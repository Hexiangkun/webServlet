//
// Created by 37496 on 2024/3/15.
//

#include "FriendModel.h"
#include "sqlconn/MySqlConnectionv1.h"
#include "nlohmann/json.h"
#include <cstring>

bool FriendModel::insert(int id, int fid) {
    char sql[1024];
    sprintf(sql, "select id from user where id=%d", fid);

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res!= nullptr && mysql_fetch_row(res) != nullptr) {
            mysql_free_result(res);
            ::memset(sql, 0, sizeof sql);
            sprintf(sql, "insert into friend(userid, friendid) values(%d, %d)", id, fid);
            return mysql.update(sql);
        }
        mysql_free_result(res);
    }
    return false;
}

void FriendModel::query(int userId, std::vector<std::string> &result) {
    char sql[1024];
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userId);
    //查询userid的好友信息

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", "")) {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr) {
            MYSQL_ROW row ;
            nlohmann::json js;
            while((row = mysql_fetch_row(res)) != nullptr) {
                js["id"] = atoi(row[0]);
                js["name"] = row[1];
                js["state"] = row[2];
                result.push_back(js.dump());
            }
            mysql_free_result(res);
        }

        // 还需要再查一次，把userID作为friendID查
        ::memset(sql, 0, sizeof sql);
        sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.userid = a.id where b.friendid = %d", userId);
        res = mysql.query(sql);

        if(res!= nullptr) {
            MYSQL_ROW row;
            nlohmann::json js;

            while((row = mysql_fetch_row(res)) != nullptr) {
                js["id"] = atoi(row[0]);
                js["name"] = row[1];
                js["state"] = row[2];
                result.push_back(js.dump());
            }

        }
        mysql_free_result(res);
    }
}