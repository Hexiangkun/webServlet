//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPREQUESTLINE_H
#define WEBSERVER_HTTPREQUESTLINE_H

#include <unordered_map>
#include <string>
#include "http/util/HttpTypeUtil.h"

namespace Tiny_muduo::Http
{
    class HttpRequestLine
    {
    public:
        typedef std::unordered_map<std::string, std::string> Query;

        enum STATE {
            PARSE_SUCCESS,
            PARSE_FATAL,
        };

        enum SCHEME {
            HTTP,
            HTTPS,
        };

        HttpRequestLine(bool decode = true);

        bool parseRequestLine(const std::string& data);

        bool empty() const { return _url.empty(); }
        size_t size() const { return _url.size(); }

        HttpMethod getMethod() const { return _method; }
        std::string getMethodStr() const { return HttpMethod2Str.at(_method); }

        HttpVersion getVersion() const { return _version; }
        const std::string& getUrl() const { return _url;}
        const std::string& getHost() const { return _host; }
        const std::string& getHostName() const { return _hostName; }
        const std::string& getPort() const { return _port; }
        const std::string& getPath() const { return _path; }
        const Query& getQuery() const { return _query; }

        std::string getScheme() const {
            switch (_scheme) {
                case HTTP:
                    return "HTTP";
                case HTTPS:
                    return "HTTPS";
                default:
                    return "";
            }
        }

    private:
        STATE parseUrl();

    private:
        bool _decode;

        STATE _state;
        SCHEME _scheme;
        std::string _url;
        std::string _host;
        std::string _hostName;
        std::string _port;
        std::string _path;
        Query _query;

        HttpMethod _method;
        HttpVersion _version;

        const size_t _maxRequestLineLen;
    };
}


#endif //WEBSERVER_HTTPREQUESTLINE_H
