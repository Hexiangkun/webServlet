//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPHEADER_H
#define WEBSERVER_HTTPHEADER_H

#include "http/util/HttpMap.h"

namespace Tiny_muduo::Http
{
    class HttpHeader : public HttpMap<CASE_SENSITIVE::NO>
    {
    public:
        HttpHeader():_length(0) {}

        void add(const std::string& key, const std::string& val) override {
            _map[toLowers(key)] = val;
            _length += key.size() + val.size();
        }

        void del(const std::string& key) override {
            ConstIterator it;
            it = _map.find(toLowers(key));
            if(it != _map.end()) {
                _length -= key.size() + it->second.size();
                _map.erase(it);
            }
        }
        const std::unordered_map<std::string, std::string>& getHeaders() const { return _map; }

        size_t keyCount() const { return _map.size(); }
        size_t size() const override { return _length; }
        bool empty() const override { return _length > 0; }

        ReturnOption<std::string> getCookie() const { return get("Cookie");}
        ReturnOption<std::string> getContentLength() const { return get("Content-Length"); }
        ReturnOption<std::string> getTransferEncoding() const { return get("Transfer-Encoding"); }
        ReturnOption<std::string> getContentType() const { return get("Content-Type"); }
        ReturnOption<std::string> getConnection() const { return get("Connection"); }
        ReturnOption<std::string> getAcceptEncoding() const { return get("Accept-Encoding"); }

        void setCookie(const std::string &val) { add("Cookie", val); }
        void setContentLength(const std::string &val) { add("Content-Length", val); }
        void setTransferEncoding(const std::string &val) { add("Transfer-Encoding", val); }
        void setContentType(const std::string &val) { add("Content-Type", val); }
        void setConnection(const std::string &val) { add("Connection", val); }
        void setAcceptEncoding(const std::string &val) { add("Accept-Encoding", val); }
    private:
        size_t _length;     //记录头的长度
    };
}

#endif //WEBSERVER_HTTPHEADER_H
