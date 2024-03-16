//
// Created by 37496 on 2024/3/15.
//

#ifndef WEBSERVER_GROUPUSER_H
#define WEBSERVER_GROUPUSER_H

#include "User.h"

class GroupUser : public User
{
public:
    void setRole(const std::string& role) {
        _role = role;
    }

    const std::string getRole() {
        return _role;
    }

private:
    std::string _role;  //改用户在群组里的角色：管理员或者普通成员
};

#endif //WEBSERVER_GROUPUSER_H
