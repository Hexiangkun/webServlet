//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPCOOKIE_H
#define WEBSERVER_HTTPCOOKIE_H

#include <string>
#include "http/util/HttpMap.h"

namespace Tiny_muduo::Http
{
    class HttpCookie : public HttpMap<CASE_SENSITIVE::NO>
    {
    public:
        HttpCookie(const std::string& str = "");

        std::string toString() const;

        void setExpires(const std::string &expires) { _expires = expires; }
        const std::string &getExpires() const noexcept { return _expires; }
        void setDomain(const std::string &domain) { _domain = domain; }
        const std::string &getDomain() const noexcept { return _domain; }
        void setPath(const std::string &path) { _path = path; }
        const std::string &getPath() const noexcept { return _path; }
        void setMaxAge(int second) { _maxAge = second; }
        int getMaxAge() const noexcept { return _maxAge; }
        void setSessionId(const std::string &ssid) { _sessionId = ssid; }
        const std::string &getSessionId() const { return _sessionId; }

        void setSecure(bool secure) { _secure = secure; }
        bool isSecure() const noexcept { return _secure; }
        void setHttpOnly(bool httponly) { _httpOnly = httponly; }
        bool isHttpOnly() const noexcept { return _httpOnly; }

    private:
        std::string _expires;       //存储cookie的过期时间
        std::string _domain;        //Domain指定了哪些主机可以接受Cookie
        std::string _path;          //Path属性指定了一个URL路径，该URL路径必须存在于请求的URL中
        std::string _sessionId;     //存储cookie的会话ID
        int _maxAge;                //存储cookie的最大存活时间
        bool _secure;               //存储是否为安全cookie的标识
        bool _httpOnly;             //存储是否为HttpOnly的标识
    };
}


#endif //WEBSERVER_HTTPCOOKIE_H
