//
// Created by 37496 on 2024/2/22.
//

#include "HttpCookie.h"
#include "http/util/EncodeUtil.h"

namespace Tiny_muduo::Http
{
    HttpCookie::HttpCookie(const std::string &str)
                :_secure(false),
                _httpOnly(false)
    {
        std::string decoded_str = EncodeUtil::urlDecode(str);
        std::string key,val;
        size_t i=0, j =0;
        while(i < decoded_str.size()) {
            if(i == decoded_str.size() - 1 || decoded_str[i] == ';') {
                i = decoded_str[i] == ';' ? i : i+1;
                if(key.empty()) {
                    std::string attr = toLowers(decoded_str.substr(j, i-j));
                    if(attr == "secure") {
                        _secure = true;
                    }
                    else if(attr == "httponly") {
                        _httpOnly = true;
                    }
                }
                else {
                    std::string attr = toLowers(key);
                    val = decoded_str.substr(j, i-j);
                    if(attr == "domain") {
                        _domain = val;
                    }
                    else if(attr == "path") {
                        _path = val;
                    }
                    else if(attr == "expires") {
                        _expires = val;
                    }
                    else if(attr == "max-age") {
                        _maxAge = std::stoi(val);
                    }
                    else if(attr == "sessionid") {
                        _sessionId = val;
                    }
                    else {
                        add(key, val);
                    }
                }
                ++i;
                while(i < decoded_str.size() && decoded_str[i] == ' ') {
                    ++i;
                }
                j = i;
                key.clear();
                val.clear();
            }
            else if(decoded_str[i] == '='){
                key = decoded_str.substr(j, i-j);
                j = i+1;
                ++i;
            }
            else {
                ++i;
            }
        }
    }

    std::string HttpCookie::toString() const{
        std::string str;
        size_t i = 0;
        size_t n = HttpMap<CASE_SENSITIVE::NO>::size();
        for(const auto& [key, val] : _map) {
            str += key;
            str += "=";
            str += val;
            if(i++ < n-1) {
                str += "; ";
            }
        }
        if (!_expires.empty())
            str += "; Expires=" + _expires;
        if (!_domain.empty())
            str += "; Domain=" + _domain;
        if (!_path.empty())
            str += "; Path=" + _path;
        if (_maxAge > 0)
            str += "; Max-Age=" + std::to_string(_maxAge);
        if (_secure)
            str += "; Secure";
        if (_httpOnly)
            str += "; HttpOnly";
        return str;
    }
}