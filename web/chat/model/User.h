//
// Created by 37496 on 2024/3/15.
//

#ifndef WEBSERVER_USER_H
#define WEBSERVER_USER_H

#include <string>

//映射在数据库中的User表
class User
{
public:
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
            : _id(id), _name(name), _password(password), _state(state) {}
    // 提供四个方法
    void setId(int id) { _id = id; }
    void setName(const std::string & name) { _name = name; }
    void setPassword(const std::string & password) { _password = password; }
    void setState(const std::string &state) { _state = state; }

    const int getId() { return _id; }
    const std::string getName() { return _name; }
    const std::string getPassword() { return _password; }
    const std::string getState() { return _state; }

private:
    int _id;
    std::string _name;
    std::string _password;
    std::string _state;
};


#endif //WEBSERVER_USER_H
