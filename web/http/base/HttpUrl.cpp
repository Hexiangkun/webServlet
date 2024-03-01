//
// Created by 37496 on 2024/2/23.
//

#include "HttpUrl.h"
#include "http/util/EncodeUtil.h"

namespace Tiny_muduo::Http
{
    HttpUrl::HttpUrl(const std::string &url, bool decode) {
        if(decode) {
            _url = EncodeUtil::urlDecode(url);
        }
        else {
            _url = url;
        }
        if(decode && !url.empty() && _url.empty()) {
            _state = PARSE_FATAL;
        }
        else {
            _state = parseUrl();
        }
    }

    HttpUrl::STATE HttpUrl::parseUrl() {
        std::string_view url_view(_url);
        size_t pos;

        if(url_view.compare(0, 4, "http") == 0) {
            pos = url_view.find("://");
            if(pos == url_view.npos) {
                return PARSE_FATAL;
            }
            std::string_view scheme_strview = url_view.substr(0, pos);
            if(scheme_strview == "http") {
                _scheme = HTTP;
            }
            else if(scheme_strview == "https") {
                _scheme = HTTPS;
            }
            else {
                return PARSE_FATAL;
            }

            url_view.remove_prefix(pos+3);
            if(url_view.size() < 2) {
                return PARSE_FATAL;
            }
            pos = url_view.find("/");
            if(pos == url_view.npos) {
                _host = std::string(url_view);
                _hostName = _host;
                _path = "/";
                return PARSE_SUCCESS;
            }
            else {
                _host = std::string(url_view.substr(0, pos));
                size_t pos1 = _host.find(":");
                if(pos1 != _host.npos) {
                    _hostName = _host.substr(0, pos1);
                    _port = _host.substr(pos1+1);
                }
                else {
                    _hostName = _host;
                }
            }
            url_view.remove_prefix(pos);

        }
        else {
            if(url_view.compare(0, 1, "/") != 0) {
                return PARSE_FATAL;
            }
        }

        pos = url_view.find("?");

        if(pos == url_view.npos) {
            _path = std::string(url_view);
            return PARSE_SUCCESS;
        }
        else {
            _path = std::string(url_view.substr(0, pos));
        }

        url_view.remove_prefix(pos+1);

        std::string_view query_view = url_view;
        std::string_view key, val;
        size_t i=0, j=0;

        for(i=0; i <query_view.size(); i++) {
            if(query_view[i] == '=') {
                key = query_view.substr(j, i-j);
                j = i+1;
            }
            else if(query_view[i] == '&') {
                val = query_view.substr(j, i-j);
                j = i+1;
                _query[std::string(key)] = std::string(val);
                key = "";
                val = "";
            }
        }
        if(!key.empty()) {
            val = query_view.substr(j, i-j);
            _query[std::string(key)] = std::string(val);
        }

        return PARSE_SUCCESS;
    }
}