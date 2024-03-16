//
// Created by 37496 on 2024/3/15.
//

#ifndef WEBSERVER_FRIENDMODEL_H
#define WEBSERVER_FRIENDMODEL_H

#include <vector>
#include <string>

class FriendModel {
public:
    bool insert(int id, int fid);

    void query(int userId, std::vector<std::string>& result);
};


#endif //WEBSERVER_FRIENDMODEL_H
