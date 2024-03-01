//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPURL_H
#define WEBSERVER_HTTPURL_H

#include <unordered_map>
#include <string>

namespace Tiny_muduo::Http
{
    class HttpUrl
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

        HttpUrl(const std::string& url = "", bool decode = true);

        bool empty() const { return _url.empty(); }
        size_t size() const { return _url.size(); }

        STATE getState() const { return _state; }
        const std::string getUrl() const { return _url;}

        std::string_view getHost() const { return _host; }
        std::string_view getHostName() const { return _hostName; }
        std::string_view getPort() const { return _port; }
        std::string_view getPath() const { return _path; }
        const Query getQuery() const { return _query; }

        std::string_view getScheme() const {
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
        STATE _state;
        SCHEME _scheme;
        std::string _url;
        std::string _host;
        std::string _hostName;
        std::string _port;
        std::string _path;
        Query _query;
    };
}


#endif //WEBSERVER_HTTPURL_H
