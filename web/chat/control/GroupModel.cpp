//
// Created by 37496 on 2024/3/15.
//

#include "GroupModel.h"
#include "sqlconn/MySqlConnectionv1.h"
#include <cstring>

//创建群组
bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
            group.getGName().c_str(), group.getGName().c_str());

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        if (mysql.update(sql))
        {
            group.setID(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

bool GroupModel::addUser(int userId, int groupId, const std::string &role) {
    // 1.组装sql语句
    char sql[1024] = {0};
    // 检查groupId的合法性
    sprintf(sql, "select id from allgroup where id = %d", groupId);

    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res && mysql_fetch_row(res) != nullptr) {
            ::memset(sql, 0, sizeof sql);
            sprintf(sql, "insert into groupuser values(%d, %d, '%s')",
                    groupId, userId, role.c_str());
            mysql_free_result(res);
            return mysql.update(sql);
        }
    }
    return false;
}

Group GroupModel::query(int groupId) {
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select id, groupname, groupdesc from allgroup where id = %d", groupId);
    Group group;
    SqlConn::MySqlConnectionv1 mysql;
    if (mysql.connect("", "", "", ""))
    {
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        if(res && (row = mysql_fetch_row(res)) != nullptr) {
            group.setID(atoi(row[0]));
            group.setGName(row[1]);
            group.setGDesc(row[2]);
            mysql_free_result(res);
            return group;
        }
    }
    return group;
}

//查询用户所在群组信息
void GroupModel::queryUserAllGroup(int userid ,std::vector<Group>& groups) {
    /*
    1. 先根据userid在groupuser表中查询出该用户所属的群组信息
    2. 在根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    */
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid=%d",
            userid);

    SqlConn::MySqlConnectionv1 mySqlConnectionv1;
    if(mySqlConnectionv1.connect("", "", "", "")) {
        MYSQL_RES* res = mySqlConnectionv1.query(sql);
        if(res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res) ) != nullptr) {
                Group group;
                group.setID(atoi(row[0]));
                group.setGName(row[1]);
                group.setGDesc(row[2]);
                groups.push_back(std::move(group));
            }
        }
    }

    for(Group& g: groups) {
        memset(sql, 0, sizeof sql);
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a inner join groupuser b on b.userid = a.id where b.groupid = %d", g.getID());
        MYSQL_RES* res = mySqlConnectionv1.query(sql);
        if(res!= nullptr) {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]);
                groupUser.setRole(row[3]);
                g.getUsers().push_back(groupUser);
            }
        }
        mysql_free_result(res);
    }

}


//获取一个组内的所有成员的id
bool GroupModel::queryOneGroup(int userid, int groupid, std::vector<int>& userIds) {
    char sql[1024];
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);
    SqlConn::MySqlConnectionv1 mySqlConnectionv1;
    if(mySqlConnectionv1.connect("", "", "", "")) {
        MYSQL_RES* res = mySqlConnectionv1.query(sql);
        if(res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                userIds.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
            return true;
        }
        mysql_free_result(res);
    }
    return false;
}

//获取一个组内所有成员的信息
bool GroupModel::queryOneGroup(int usrid, int gID, std::vector<GroupUser> &usrVec) {
    // 组装sql语句
    char sql[1024];
    sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid = %d",
            gID);
    SqlConn::MySqlConnectionv1 mySqlConnectionv1;
    if(mySqlConnectionv1.connect("", "", "", "")) {
        MYSQL_RES *res = mySqlConnectionv1.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser gu;
                gu.setId(atoi(row[0]));
                gu.setName(row[1]);
                gu.setState(row[2]);
                gu.setRole(row[3]);
                usrVec.push_back(gu);
            }
            mysql_free_result(res);
            return true;
        }
        mysql_free_result(res);
    }
    return false;
}


// 检查gid是否为服务器拥有的群
bool GroupModel::checkGroup(int gID)
{
    // 组装sql语句
    char sql[1024];
    sprintf(sql, "select id from allgroup where id = %d", gID);

    SqlConn::MySqlConnectionv1 mySqlConnectionv1;
    if(mySqlConnectionv1.connect("", "", "", "")) {
        MYSQL_RES *res = mySqlConnectionv1.query(sql);
        if (res != nullptr)
        {
            // 找到这样的群组和用户映射关系
            mysql_free_result(res);
            return true;
        }
        mysql_free_result(res);
    }
    return false;
}