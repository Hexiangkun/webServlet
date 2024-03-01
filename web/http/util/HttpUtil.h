//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPUTIL_H
#define WEBSERVER_HTTPUTIL_H

#include <string>
#include <string_view>
#include "HttpMap.h"


namespace Tiny_muduo::Http
{
    //json连接key、val
    //split分割每一对键值对
    void parseKeyValue(const std::string& src, const std::string& join, const std::string& split,
                       HttpMap<CASE_SENSITIVE::YES>& map, bool urlDecode = true);
}


#endif //WEBSERVER_HTTPUTIL_H
